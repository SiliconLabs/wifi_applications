/***************************************************************************/ /**
 * @file sl_wifi_asset_tracking_wifi_handler.c
 * @brief wifi connection and data capture related functions
 *******************************************************************************
 * # License
 * <b>Copyright 2023 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 ******************************************************************************/

#include <stdio.h>
#include <ctype.h>
#include <FreeRTOS.h>
#include <task.h>
#include <sl_wifi.h>
#include <sl_net.h>
#include <sl_net_wifi_types.h>
#include <sl_net_dns.h>
#include <sl_sntp.h>
#include <sl_constants.h>
#include <sl_utility.h>
#include <sl_si91x_driver.h>
#include <sl_wifi_asset_tracking_app.h>
#include <sl_wifi_asset_tracking_demo_config.h>
#include <sl_wifi_asset_tracking_wifi_handler.h>

/// Firmware application wi-fi device initialization configuration
static const sl_wifi_device_configuration_t client_init_configuration = {
  .boot_option = LOAD_NWP_FW,
  .mac_address = NULL,
  .band = SL_SI91X_WIFI_BAND_2_4GHZ,
  .region_code = US,
  .boot_config = { .oper_mode = SL_SI91X_CLIENT_MODE,
                   .coex_mode = SL_SI91X_WLAN_ONLY_MODE,
                   .feature_bit_map =
                     (SL_SI91X_FEAT_SECURITY_PSK
                      | SL_SI91X_FEAT_ULP_GPIO_BASED_HANDSHAKE
                      | SL_SI91X_FEAT_AGGREGATION),
                   .tcp_ip_feature_bit_map =
                     (SL_SI91X_TCP_IP_FEAT_DHCPV4_CLIENT
                      | SL_SI91X_TCP_IP_FEAT_DNS_CLIENT
                      | SL_SI91X_TCP_IP_FEAT_SSL
#ifdef SLI_SI91X_ENABLE_IPV6
                      | SL_SI91X_TCP_IP_FEAT_DHCPV6_CLIENT
                      | SL_SI91X_TCP_IP_FEAT_IPV6
#endif
                      | SL_SI91X_TCP_IP_FEAT_ICMP
                      | SL_SI91X_TCP_IP_FEAT_SNTP_CLIENT
                      | SL_SI91X_TCP_IP_FEAT_EXTENSION_VALID),
                   .custom_feature_bit_map =
                     SL_SI91X_CUSTOM_FEAT_EXTENTION_VALID,
                   .ext_custom_feature_bit_map =
                     (SL_SI91X_EXT_FEAT_XTAL_CLK
                      | SL_SI91X_EXT_FEAT_SSL_VERSIONS_SUPPORT
                      | SL_SI91X_EXT_FEAT_UART_SEL_FOR_DEBUG_PRINTS
                      | SL_SI91X_EXT_FEAT_FRONT_END_SWITCH_PINS_ULP_GPIO_4_5_0
                      | SL_SI91X_EXT_FEAT_LOW_POWER_MODE
                      | MEMORY_CONFIG),
                   .bt_feature_bit_map = 0,
                   .ext_tcp_ip_feature_bit_map =
                     (SL_SI91X_EXT_TCP_IP_WINDOW_SCALING
                      | SL_SI91X_EXT_TCP_IP_TOTAL_SELECTS(10)
                      | SL_SI91X_CONFIG_FEAT_EXTENTION_VALID),
                   .ble_feature_bit_map = 0,
                   .ble_ext_feature_bit_map = 0,
                   .config_feature_bit_map = 0 }
};

/// Month name list as per UTC format
static char *sl_month_name_utc_format[] = { "Jan", "Feb", "Mar", "Apr", "May",
                                            "Jun", "Jul", "Aug", "Sep", "Oct",
                                            "Nov", "Dec" };

/******************************************************************************
 *  Callback function to capture Wi-Fi data at configured interval.
 *****************************************************************************/
void sl_capture_wifi_data_task()
{
  uint8_t next_packet_send = 0;
  int64_t last_ka_time = 0, last_wifi_time = 0;
  uint32_t task_delay = 0;
  uint32_t ka_interval = KEEP_ALIVE_INTERVAL * 1000;
  uint32_t wifi_interval = DEMO_CONFIG_WIFI_SAMPLING_INTERVAL * 1000;
  sl_status_t ka_status = SL_STATUS_OK, wifi_status = SL_STATUS_OK;
  TickType_t initial_tick_count = 0, later_tick_count = 0;
  TickType_t processing_diff = 0;

  if (SL_WIFI_NOT_CONNECTED
      == sl_get_wifi_asset_tracking_status()->wifi_conn_status) {
    if (SL_STATUS_OK == sl_start_wifi_connection()) {
      /// Set Wi-Fi status flag as connected
      sl_get_wifi_asset_tracking_status()->wifi_conn_status = SL_WIFI_CONNECTED;

      sl_wifi_asset_tracking_lcd_print(INDEX_WIFI_CONNECTED);

      /// Initially set Wi-Fi and keep alive packet types
      next_packet_send |= WIFI_PACKET_TYPE;
      next_packet_send |= KEEP_ALIVE_PACKET_TYPE;
    } else {
      if (SL_STATUS_OK != sl_retry_wifi_connection(true)) {
        sl_get_wifi_asset_tracking_status()->wifi_conn_status =
          SL_WIFI_SHUTDOWN;

        if (eSuspended
            == eTaskGetState(sl_get_wifi_asset_tracking_resource()->task_list.
                             recovery_task_handler)) {
          vTaskResume(
            sl_get_wifi_asset_tracking_resource()->task_list.recovery_task_handler);
        }
      } else {
        /// Set Wi-Fi status flag as connected
        sl_get_wifi_asset_tracking_status()->wifi_conn_status =
          SL_WIFI_CONNECTED;

        sl_wifi_asset_tracking_lcd_print(INDEX_WIFI_CONNECTED);

        /// Initially set Wi-Fi and keep alive packet types
        next_packet_send |= WIFI_PACKET_TYPE;
        next_packet_send |= KEEP_ALIVE_PACKET_TYPE;
      }
    }
  }

  /// Here comes only if wi-fi is connected
  if (sl_get_wifi_asset_tracking_resource()->task_list.
      temp_rh_sensor_task_handler != NULL) {
    printf("\r\nwifi_task: resuming temperature and RH sensor task\r\n");
    vTaskResume(
      sl_get_wifi_asset_tracking_resource()->task_list.temp_rh_sensor_task_handler);
  }

  if (sl_get_wifi_asset_tracking_resource()->task_list.imu_sensor_task_handler
      != NULL) {
    printf("\r\nwifi_task: resuming imu sensor task\r\n");
    vTaskResume(
      sl_get_wifi_asset_tracking_resource()->task_list.imu_sensor_task_handler);
  }

  if (sl_get_wifi_asset_tracking_resource()->task_list.
      gnss_receiver_task_handler != NULL) {
    printf("\r\nwifi_task: resuming gnss receiver task\r\n");
    vTaskResume(
      sl_get_wifi_asset_tracking_resource()->task_list.gnss_receiver_task_handler);
  }

  while (1) {
    initial_tick_count = xTaskGetTickCount();

    /// If wi-fi is in connected state
    if (SL_WIFI_CONNECTED
        == sl_get_wifi_asset_tracking_status()->wifi_conn_status) {
      /// if keep alive packet type is set then send keep alive packet
      if (next_packet_send & KEEP_ALIVE_PACKET_TYPE) {
        ka_status = sl_json_send_keep_alive_message();
      }

      /// if wi-fi packet type is set then send wi-fi packet
      if (next_packet_send & WIFI_PACKET_TYPE) {
        wifi_status = sl_json_send_wifi_message();

        /// If failed to frame wi-fi packet then call for wi-fi recovery
        if (SL_STATUS_WIFI_CONNECTION_LOST == wifi_status) {
          bool recovery_resume_required = false;

          sl_get_wifi_asset_tracking_status()->wifi_conn_status =
            SL_WIFI_DISCONNECTED;

          /// Acquire semaphore to update the status of recovery task
          if (pdTRUE
              == (xSemaphoreTake(sl_get_wifi_asset_tracking_resource()->
                                 recovery_status_mutex_handler,
                                 portMAX_DELAY))) {
            if (SL_RECOVERY_IDLE
                == sl_get_wifi_asset_tracking_status()->recovery_progress_status)
            {
              sl_get_wifi_asset_tracking_status()->recovery_progress_status =
                SL_RECOVERY_INPROGRESS;
              recovery_resume_required = true;
            }

            xSemaphoreGive(
              sl_get_wifi_asset_tracking_resource()->recovery_status_mutex_handler);

            if (recovery_resume_required) {
              printf(
                "\r\nwifi_task : Resuming recovery task for Wi-Fi recovery\r\n");
              vTaskResume(
                sl_get_wifi_asset_tracking_resource()->task_list.recovery_task_handler);
            }
          }
        }
      }

      /// reset next packet send variable
      next_packet_send &= 0;

      if ((last_ka_time + ka_interval) == (last_wifi_time + wifi_interval)) {
        if (ka_interval <= wifi_interval) {
          task_delay = ka_interval;
        } else {
          task_delay = wifi_interval;
        }

        last_ka_time += ka_interval;
        last_wifi_time += wifi_interval;

        next_packet_send |= WIFI_PACKET_TYPE;
        next_packet_send |= KEEP_ALIVE_PACKET_TYPE;
      } else if ((last_ka_time + ka_interval)
                 < (last_wifi_time + wifi_interval)) {
        if (last_ka_time > last_wifi_time) {
          task_delay = ka_interval;
        } else {
          task_delay = last_ka_time + ka_interval - last_wifi_time;
        }

        last_ka_time += ka_interval;
        next_packet_send |= KEEP_ALIVE_PACKET_TYPE;
      } else {
        if (last_wifi_time > last_ka_time) {
          task_delay = wifi_interval;
        } else {
          task_delay = last_wifi_time + wifi_interval - last_ka_time;
        }

        last_wifi_time += wifi_interval;
        next_packet_send |= WIFI_PACKET_TYPE;
      }

      /// If JSON message send success to MQTT package queue then wake-up azure cloud task
      if ((SL_STATUS_OK == ka_status) || (SL_STATUS_OK == wifi_status)) {
        /// check whether MQTT task is suspended or not - If suspended then resume it
        if (eSuspended
            == (eTaskGetState(sl_get_wifi_asset_tracking_resource()->task_list.
                              azure_cloud_communication_task_handler))) {
          vTaskResume(
            sl_get_wifi_asset_tracking_resource()->task_list.azure_cloud_communication_task_handler);
        }
      }
    }

    later_tick_count = xTaskGetTickCount();

    /// Sampling interval and setting a offset
    task_delay = pdMS_TO_TICKS(task_delay) * TIMER_CLOCK_OFFSET;

#if ENABLE_SAMPLING_JITTER
    processing_diff = (later_tick_count - initial_tick_count)
                      * TIMER_CLOCK_OFFSET;

    if (processing_diff < task_delay) {
      /// Difference of processing
      task_delay = task_delay - processing_diff;
    } else {
      /// If processing difference is greater than sampling interval
      processing_diff = processing_diff - task_delay;

      /// Take absolute value for the difference
      if (task_delay > processing_diff) {
        task_delay = task_delay - processing_diff;
      } else {
        task_delay = processing_diff - task_delay;
      }
    }
#else

    UNUSED_VARIABLE(initial_tick_count);
    UNUSED_VARIABLE(later_tick_count);
    UNUSED_VARIABLE(processing_diff);
#endif /// < ENABLE_SAMPLING_JITTER

#if DEMO_CONFIG_DEBUG_LOGS
    printf("\r\nwifi_task : delay is : %ld\r\n", task_delay);
#endif /// < DEMO_CONFIG_DEBUG_LOGS

    vTaskDelay(task_delay);
  }
}

/******************************************************************************
 *  Function to initialize and start Wi-Fi connection with configured SSID.
 *****************************************************************************/
sl_status_t sl_start_wifi_connection()
{
  sl_status_t status;
  sl_net_wifi_client_profile_t profile = { 0 };
  sl_ip_address_t ip_address = { 0 };

  status = sl_net_init(SL_NET_WIFI_CLIENT_INTERFACE,
                       &client_init_configuration,
                       NULL,
                       NULL);
  if (status != SL_STATUS_OK) {
    printf(
      "\r\nsl_start_wifi_connection : Unexpected error while initializing Wi-Fi: 0x%lx\r\n",
      status);
    return SL_STATUS_FAIL;
  }
#if DEMO_CONFIG_DEBUG_LOGS
  printf("\r\nsl_start_wifi_connection : Wi-Fi is Initialized\r\n");
#endif /// < DEMO_CONFIG_DEBUG_LOGS

#ifdef SLI_SI91X_MCU_INTERFACE
  uint8_t xtal_enable = 1;
  status = sl_si91x_m4_ta_secure_handshake(SL_SI91X_ENABLE_XTAL,
                                           1,
                                           &xtal_enable,
                                           0,
                                           NULL);
  if (status != SL_STATUS_OK) {
    printf(
      "\r\nsl_start_wifi_connection : Failed to bring m4_ta_secure_handshake: 0x%lx\r\n",
      status);
    return SL_STATUS_FAIL;
  }
#if DEMO_CONFIG_DEBUG_LOGS
  printf(
    "\r\nsl_start_wifi_connection : M4-NWP secure handshake is successful\r\n");
#endif /// < DEMO_CONFIG_DEBUG_LOGS
#endif

  status = sl_net_up(SL_NET_WIFI_CLIENT_INTERFACE,
                     SL_NET_DEFAULT_WIFI_CLIENT_PROFILE_ID);
  if (status != SL_STATUS_OK) {
    printf(
      "sl_start_wifi_connection : Error while connecting to Access point: 0x%lx\r\n",
      status);
    return SL_STATUS_FAIL;
  }
#if DEMO_CONFIG_DEBUG_LOGS
  printf("\r\nsl_start_wifi_connection : Connected to Access point\r\n");
#endif /// < DEMO_CONFIG_DEBUG_LOGS

  status = sl_net_get_profile(SL_NET_WIFI_CLIENT_INTERFACE,
                              SL_NET_DEFAULT_WIFI_CLIENT_PROFILE_ID,
                              &profile);
  if (status != SL_STATUS_OK) {
    printf(
      "\r\nsl_start_wifi_connection : Failed to get client profile: 0x%lx\r\n",
      status);

    sl_net_down(SL_NET_WIFI_CLIENT_INTERFACE);
    return SL_STATUS_FAIL;
  }
#if DEMO_CONFIG_DEBUG_LOGS
  printf(
    "\r\nsl_start_wifi_connection : Getting client profile is successful\r\n");
#endif /// < DEMO_CONFIG_DEBUG_LOGS

  ip_address.type = SL_IPV4;
  memcpy(&ip_address.ip.v4.bytes, &profile.ip.ip.v4.ip_address.bytes,
         sizeof(sl_ipv4_address_t));
  printf("\r\nsl_start_wifi_connection : IP address is ");
  print_sl_ip_address(&ip_address);
  printf("\r\n");

  if (SL_STATUS_OK != sl_set_time_and_date_using_sntp()) {
    sl_net_down(SL_NET_WIFI_CLIENT_INTERFACE);
    return SL_STATUS_FAIL;
  }

  sl_get_wifi_asset_tracking_status()->wifi_conn_status = true;

  return SL_STATUS_OK;
}

/******************************************************************************
 *  Utility function used to print char buffer.
 *****************************************************************************/
static void sl_print_char_buffer(char *buffer, uint32_t buffer_length)
{
  uint32_t i = 0;

  for (i = 0; i < buffer_length; i++) {
    printf("%c", buffer[i]);
  }

  printf("\r\n");
  return;
}

/******************************************************************************
 *  Utility function used to print calendar date and time.
 *****************************************************************************/
static void sl_print_calendar_datetime(sl_calendar_datetime_config_t data)
{
  printf("\n***Calendar time****\n");
  printf(
    "\r\nsl_print_calendar_datetime : Time Format: hour:%d, min:%d, sec:%d, msec:%d\r\n",
    data.Hour,
    data.Minute,
    data.Second,
    data.MilliSeconds);
  printf(
    "\r\nsl_print_calendar_datetime : Date Format: DD/MM/YY: %.2d/%.2d/%.2d\r\n",
    data.Day,
    data.Month,
    data.Year);
  printf("\r\nsl_print_calendar_datetime : Century: %d\r\n", data.Century);
}

/******************************************************************************
 *  Utility function used to convert NTP time to calendar date and time.
 *****************************************************************************/
static sl_status_t sl_convert_month_to_calendar_month_format(
  sl_cal_date_time_type_t *cal_data,
  char *month_str)
{
  if (!strncmp(month_str, sl_month_name_utc_format[0], 2)) {
    cal_data->month = January;
  } else if (!strncmp(month_str, sl_month_name_utc_format[1], 2)) {
    cal_data->month = February;
  } else if (!strncmp(month_str, sl_month_name_utc_format[2], 3)) {
    cal_data->month = March;
  } else if (!strncmp(month_str, sl_month_name_utc_format[3], 2)) {
    cal_data->month = April;
  } else if (!strncmp(month_str, sl_month_name_utc_format[4], 3)) {
    cal_data->month = May;
  } else if (!strncmp(month_str, sl_month_name_utc_format[5], 3)) {
    cal_data->month = June;
  } else if (!strncmp(month_str, sl_month_name_utc_format[6], 3)) {
    cal_data->month = July;
  } else if (!strncmp(month_str, sl_month_name_utc_format[7], 2)) {
    cal_data->month = August;
  } else if (!strncmp(month_str, sl_month_name_utc_format[8], 2)) {
    cal_data->month = September;
  } else if (!strncmp(month_str, sl_month_name_utc_format[9], 2)) {
    cal_data->month = October;
  } else if (!strncmp(month_str, sl_month_name_utc_format[10], 2)) {
    cal_data->month = November;
  } else if (!strncmp(month_str, sl_month_name_utc_format[11], 2)) {
    cal_data->month = December;
  } else {
    printf(
      "\r\nsl_convert_month_to_calendar_month_format : Invalid date format received\r\n");
    return SL_STATUS_FAIL;
  }

  return SL_STATUS_OK;
}

/******************************************************************************
 *  @brief Utility function used to get day of week using date.
 *  @return value (0-6) means (Sunday - Saturday).
 *****************************************************************************/
uint8_t sl_get_day_of_week(uint8_t day, uint8_t month, uint16_t year)
{
  static int t[] = { 0, 3, 2, 5, 0, 3, 5, 1, 4, 6, 2, 4 };
  year -= month < 3;
  return (year + year / 4 - year / 100 + year / 400 + t[month - 1] + day) % 7;
}

/******************************************************************************
 *  Utility function used to convert NTP time to calendar date and time.
 *****************************************************************************/
static sl_status_t sl_convert_ntp_to_calendar_time_format(
  sl_cal_date_time_type_t *cal_data,
  char *ntp_time_buff)
{
  uint8_t index = 0;
  /// Variable to store token string
  char token_str_arr[MAX_NTP_TIME_BUFFER_TOKENS][MAX_NTP_TIME_BUFFER_TOKEN_SIZE];
  char *token_str;

  if ((NULL == ntp_time_buff) || (NULL == cal_data)) {
    goto error;
  }

  memset(token_str_arr,
         '\0',
         sizeof(token_str_arr[0][0]) * MAX_NTP_TIME_BUFFER_TOKENS
         * MAX_NTP_TIME_BUFFER_TOKEN_SIZE);

  /// Split NTP time buffer using token ", :."
  token_str = strtok(ntp_time_buff, NTP_TIME_BUFFER_TOKENS_STRING);

  while (token_str != NULL && index < MAX_NTP_TIME_BUFFER_TOKENS) {
    /// Forceful condition to avoid End of Text character coming at 0th index of NTP buffer
    if (NTP_MONTH_BUFFER_INDEX == index) {
      for (uint8_t loop_index = 0, temp_index = 0;
           loop_index < strlen(token_str); ++loop_index) {
        if (0 != isalpha((unsigned char)token_str[loop_index])) {
          token_str_arr[index][temp_index++] = token_str[loop_index];
        }
      }
    } else {
      strcpy(token_str_arr[index], token_str);
    }

    ++index;
    token_str = strtok(NULL,
                       NTP_TIME_BUFFER_TOKENS_STRING);
  }

  if (MAX_NTP_TIME_BUFFER_TOKENS == index) {
    char temp_str[3];

    /// update month in calendar format
    if (SL_STATUS_OK
        != sl_convert_month_to_calendar_month_format(cal_data,
                                                     token_str_arr[SL_NTP_MONTH]))
    {
      goto error;
    }

    /// update day in calendar format
    cal_data->day = atoi(token_str_arr[SL_NTP_DAY]);

    /// update year in calendar format
    strncpy(temp_str, &(token_str_arr[SL_NTP_YEAR][2]), 2);
    temp_str[2] = '\0';
    cal_data->year = (uint8_t)atoi(temp_str);

    /// update century in calendar format
    strncpy(temp_str, token_str_arr[SL_NTP_YEAR], 1);
    temp_str[1] = '\0';
    cal_data->century = (uint8_t)atoi(temp_str);

    /// update hour in calendar format
    cal_data->hour = (uint8_t)atoi(token_str_arr[SL_NTP_HOUR]);

    /// update minute in calendar format
    cal_data->minute = (uint8_t)atoi(token_str_arr[SL_NTP_MINUTE]);

    /// update second in calendar format
    cal_data->second = (uint8_t)atoi(token_str_arr[SL_NTP_SECONDS]);

    /// update milli-seconds in calendar format
    cal_data->milliseconds = (uint16_t)atoi(token_str_arr[SL_NTP_MS]);

    /// update day of week in calendar format
    cal_data->dayofweek = sl_get_day_of_week(cal_data->day,
                                             cal_data->month,
                                             atoi(token_str_arr[SL_NTP_YEAR]));
  } else {
    goto error;
  }

#if DEMO_CONFIG_DEBUG_LOGS
  printf(
    "sl_convert_ntp_to_calendar_time_format : Calendar converted structure: century: %u day: %u dayofweek:%u hour:%u ms:%u min:%u month:%u sec:%u year:%u\r\n",
    cal_data->century,
    cal_data->day,
    cal_data->dayofweek,
    cal_data->hour,
    cal_data->milliseconds,
    cal_data->minute,
    cal_data->month,
    cal_data->second,
    cal_data->year);
#endif /// < DEMO_CONFIG_DEBUG_LOGS

  return SL_STATUS_OK;
  error:
  return SL_STATUS_FAIL;
}

/******************************************************************************
 *  Function used to set RTC calendar clock.
 *****************************************************************************/
static sl_status_t sl_init_rtc_calendar(sl_cal_date_time_type_t *cal_data)
{
  sl_status_t status;
  sl_calendar_datetime_config_t datetime_config;
  sl_calendar_datetime_config_t get_datetime;
#if DEMO_CONFIG_DEBUG_LOGS
  printf(
    "sl_init_rtc_calendar : Calendar converted structure: century: %u day: %u dayofweek:%u hour:%u ms:%u min:%u month:%u sec:%u year:%u\r\n",
    cal_data->century,
    cal_data->day,
    cal_data->dayofweek,
    cal_data->hour,
    cal_data->milliseconds,
    cal_data->minute,
    cal_data->month,
    cal_data->second,
    cal_data->year);
#endif /// < DEMO_CONFIG_DEBUG_LOGS

  status = sl_si91x_calendar_set_configuration(CAL_RC_CLOCK);

  if (status != SL_STATUS_OK) {
    printf(
      "\r\nsl_init_rtc_calendar : sl_si91x_calendar_set_configuration: Invalid Parameters, Error Code : %lu \n",
      status);
    goto error;
  }

  printf("\r\nsl_init_rtc_calendar : Successfully configured Calendar\n");

  sl_si91x_calendar_init();

  // Setting date-time for Calendar
  status = sl_si91x_calendar_build_datetime_struct(&datetime_config,
                                                   cal_data->century,
                                                   cal_data->year,
                                                   cal_data->month,
                                                   cal_data->dayofweek,
                                                   cal_data->day,
                                                   cal_data->hour,
                                                   cal_data->minute,
                                                   cal_data->second,
                                                   cal_data->milliseconds);

  if (status != SL_STATUS_OK) {
    printf(
      "\r\nsl_init_rtc_calendar : sl_si91x_calendar_build_datetime_struct: Invalid Parameters, Error Code : %lu \n",
      status);
    goto error;
  }

  printf("\r\nsl_init_rtc_calendar : Successfully built date-time structure\n");

  status = sl_si91x_calendar_set_date_time(&datetime_config);
  if (status != SL_STATUS_OK) {
    printf(
      "\r\nsl_si91x_calendar_set_date_time: Invalid Parameters, Error Code : %lu \r\n",
      status);
    goto error;
  }
  printf("\r\nsl_init_rtc_calendar : Successfully set calendar date-time\r\n");

  // Printing datet-ime for Calendar
  status = sl_si91x_calendar_get_date_time(&get_datetime);
  if (status != SL_STATUS_OK) {
    printf(
      "\r\nsl_init_rtc_calendar : sl_si91x_calendar_get_date_time: Invalid Parameters, Error Code : %lu \n",
      status);
    goto error;
  }
  printf(
    "\r\nsl_init_rtc_calendar : Successfully fetched the calendar date-time \r\n");
  sl_print_calendar_datetime(get_datetime);
  printf("\r\n");

  return SL_STATUS_OK;
  error:
  return SL_STATUS_FAIL;
}

/******************************************************************************
 * Function will setup SNTP and fetch date and time in UTC format from
 * network and configure calendar RTC using fetched data.
 ******************************************************************************/
sl_status_t sl_set_time_and_date_using_sntp()
{
  sl_status_t status;
  sl_ip_address_t address = { 0 };
  sl_sntp_client_config_t config = { 0 };
  uint8_t data[SNTP_DATA_BUFFER_LENGTH] = { 0 };
  sl_cal_date_time_type_t cal_data;

  status = sl_net_dns_resolve_hostname(NTP_SERVER_IP,
                                       10000,
                                       SL_NET_DNS_TYPE_IPV4,
                                       &address);

  if (SL_STATUS_OK == status) {
    printf("\r\nsl_set_time_and_date_using_sntp : IP address : %u.%u.%u.%u\r\n",
           address.ip.v4.bytes[0],
           address.ip.v4.bytes[1],
           address.ip.v4.bytes[2],
           address.ip.v4.bytes[3]);

    config.server_host_name = address.ip.v4.bytes;
    config.sntp_method = SNTP_METHOD;
    config.sntp_timeout = SNTP_TIMEOUT;
    config.event_handler = NULL;
    config.flags = SNTP_FLAGS;
  } else {
    printf(
      "\r\nsl_set_time_and_date_using_sntp : Failed to resolve DNS for NTP Server: %s status:0x%lx\r\n",
      NTP_SERVER_IP,
      status);
    goto error;
  }

  status = sl_sntp_client_start(&config, SNTP_API_TIMEOUT);

  if (SL_STATUS_OK == status) {
    printf(
      "\r\nsl_set_time_and_date_using_sntp : SNTP Client started successfully\r\n");
  } else {
    printf(
      "\r\nsl_set_time_and_date_using_sntp : Failed to start SNTP client: 0x%lx\r\n",
      status);
    goto error;
  }

  status = sl_sntp_client_get_time_date(data,
                                        SNTP_DATA_BUFFER_LENGTH,
                                        SNTP_API_TIMEOUT);

  if (SL_STATUS_OK == status) {
    printf(
      "\r\nsl_set_time_and_date_using_sntp : SNTP Client got TIME and DATE successfully\r\n");
  } else {
    printf(
      "\r\nsl_set_time_and_date_using_sntp : Failed to get date and time from ntp server : 0x%lx \r\n",
      status);
    goto error;
  }

  sl_print_char_buffer((char *)data, strlen((const char *)data));

  status = sl_sntp_client_stop(SNTP_API_TIMEOUT);

  if (SL_STATUS_OK == status) {
    printf(
      "\r\nsl_set_time_and_date_using_sntp : SNTP Client stopped successfully\r\n");
  } else {
    printf(
      "\r\nsl_set_time_and_date_using_sntp : Failed to stop SNTP client: 0x%lx\r\n",
      status);
    goto error;
  }

  /// Append null terminating byte at end of time-stamp buffer
  data[strlen((const char *)data)] = '\0';

  /// Network time fetched successfully then convert it to calendar time format
  status = sl_convert_ntp_to_calendar_time_format(&cal_data, (char *)data);

  if (SL_STATUS_OK == status) {
    printf(
      "\r\nsl_set_time_and_date_using_sntp : Conversion of network time to calendar time is successful\r\n");
  } else {
    printf(
      "\r\nsl_set_time_and_date_using_sntp : Unable to convert network time to calendar time - setting default values\r\n");
    goto error;
  }

  /// If unable to set network time then set default calendar time
  if (SL_STATUS_OK != sl_init_rtc_calendar(&cal_data)) {
    goto error;
  }

  return SL_STATUS_OK;
  error:

  /// set default values
  cal_data.century = DEF_CAL_CENTURY;
  cal_data.year = DEF_CAL_YEAR;
  cal_data.day = DEF_CAL_DAY;
  cal_data.hour = DEF_CAL_HOUR;
  cal_data.minute = DEF_CAL_MINUTE;
  cal_data.second = DEF_CAL_SECONDS;
  cal_data.milliseconds = DEF_CAL_MILLISECONDS;
  cal_data.month = DEF_CAL_MONTH;
  cal_data.dayofweek = DEF_CAL_DAY_OF_WEEK;

  return sl_init_rtc_calendar(&cal_data);
}

/******************************************************************************
 * Function will retry for Wi-Fi connection with configured SSID for five times
 * at every five seconds.
 ******************************************************************************/
sl_status_t sl_deinit_wifi_connection()
{
  sl_status_t status = SL_STATUS_OK;

  status = sl_net_down(SL_NET_WIFI_CLIENT_INTERFACE);
  if (status != SL_STATUS_OK) {
    printf(
      "\r\nsl_deinit_wifi_connection : Unexpected error while net down Wi-Fi: 0x%lx\r\n",
      status);
  } else {
    printf("\r\nsl_deinit_wifi_connection : Wi-Fi net down success\r\n");
  }

  status = sl_net_deinit(SL_NET_WIFI_CLIENT_INTERFACE);
  if (status != SL_STATUS_OK) {
    printf(
      "\r\nsl_deinit_wifi_connection : Unexpected error while de-initializing Wi-Fi connection: 0x%lx\r\n",
      status);
  } else {
    printf("\r\nsl_deinit_wifi_connection : Wi-Fi de-initialization success\r\n");
  }

  return status;
}

/******************************************************************************
 * Function will retry for Wi-Fi connection with configured SSID for five times
 * at every five seconds.
 ******************************************************************************/
sl_status_t sl_retry_wifi_connection(bool init)
{
  uint8_t wifi_retry;
  sl_status_t status;
  sl_net_wifi_client_profile_t profile = { 0 };
  sl_ip_address_t ip_address = { 0 };
  int32_t rssi;

#if DEMO_CONFIG_DEBUG_LOGS
  printf("\r\nsl_retry_wifi_connection : initialization flag : %d\r\n", init);
#endif /// < DEMO_CONFIG_DEBUG_LOGS

  for (wifi_retry = 0; wifi_retry < MAX_WIFI_CONN_RETRY_COUNT; ++wifi_retry) {
    printf(
      "\r\nsl_retry_wifi_connection : retrying to establish wi-fi connection\r\n");

    sl_wifi_asset_tracking_lcd_print(INDEX_WIFI_RETRY);
    if (init) {
      if (SL_STATUS_OK
          != (status =
                sl_net_up(SL_NET_WIFI_CLIENT_INTERFACE,
                          SL_NET_DEFAULT_WIFI_CLIENT_PROFILE_ID))) {
        vTaskDelay(pdMS_TO_TICKS(
                     WIFI_CONN_DELAY_BETN_RETRY) * TIMER_CLOCK_OFFSET);
        continue;
      } else {
        printf("\r\nsl_retry_wifi_connection : Connected to Access point\r\n");

        status = sl_net_get_profile(SL_NET_WIFI_CLIENT_INTERFACE,
                                    SL_NET_DEFAULT_WIFI_CLIENT_PROFILE_ID,
                                    &profile);
        if (status != SL_STATUS_OK) {
          printf(
            "\r\nsl_retry_wifi_connection : Failed to get client profile: 0x%lx\r\n",
            status);

          sl_net_down(SL_NET_WIFI_CLIENT_INTERFACE);
          continue;
        }
        printf(
          "\r\nsl_retry_wifi_connection : Getting client profile is successful\r\n");

        ip_address.type = SL_IPV4;
        memcpy(&ip_address.ip.v4.bytes,
               &profile.ip.ip.v4.ip_address.bytes,
               sizeof(sl_ipv4_address_t));
        printf("\r\nsl_retry_wifi_connection : IP address is ");
        print_sl_ip_address(&ip_address);
        printf("\r\n");

        if (SL_STATUS_OK != sl_set_time_and_date_using_sntp()) {
          sl_net_down(SL_NET_WIFI_CLIENT_INTERFACE);
          continue;
        }

        break;
      }
    } else {
      if (SL_STATUS_OK != (status = sl_get_wifi_rssi(&rssi))) {
        vTaskDelay(pdMS_TO_TICKS(
                     WIFI_CONN_DELAY_BETN_RETRY) * TIMER_CLOCK_OFFSET);
        continue;
      }

      break;
    }
  }

  return status;
}

/******************************************************************************
 * Function will fetch current RSSI value using SL_WIFI_CLIENT_INTERFACE.
 ******************************************************************************/
sl_status_t sl_get_wifi_rssi(int32_t *rssi)
{
  sl_status_t status;

  status = sl_wifi_get_signal_strength(SL_WIFI_CLIENT_INTERFACE, rssi);

  if (SL_STATUS_OK != status) {
    printf("\r\nsl_get_wifi_rssi : Failed to get RSSI Value\r\n");
    return SL_STATUS_FAIL;
  }
#if DEMO_CONFIG_DEBUG_LOGS
  printf("\r\nsl_get_wifi_rssi : RSSI Value: %ld\r\n", *rssi);
#endif /// < DEMO_CONFIG_DEBUG_LOGS

  return SL_STATUS_OK;
}

/******************************************************************************
 * Function will fetch MAC address using SL_WIFI_CLIENT_INTERFACE and convert
 * into string format.
 ******************************************************************************/
sl_status_t sl_get_wifi_mac_address(uint8_t *mac_addr)
{
  sl_mac_address_t mac_address;
  sl_status_t status;

  status = sl_wifi_get_mac_address(SL_WIFI_CLIENT_INTERFACE, &mac_address);
  if (SL_STATUS_OK != status) {
    printf("\r\nsl_get_wifi_mac_address : Failed to get MAC address\r\n");
    return SL_STATUS_FAIL;
  }

  snprintf((char *)mac_addr,
           JSON_MAX_MAC_ADDR_BUFF_SIZE,
           "%2X:%2X:%2X:%2X:%2X:%2X",
           mac_address.octet[0],
           mac_address.octet[1],
           mac_address.octet[2],
           mac_address.octet[3],
           mac_address.octet[4],
           mac_address.octet[5]);

  mac_addr[JSON_MAX_MAC_ADDR_BUFF_SIZE - 1] = '\0';

  printf("\r\nsl_get_wifi_mac_address : MAC address: %s\r\n", mac_addr);

  return SL_STATUS_OK;
}

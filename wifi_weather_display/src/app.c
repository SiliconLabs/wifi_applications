/***************************************************************************/ /**
 * @file
 * @brief Wi-Fi - Weather Display
 *******************************************************************************
 * # License
 * <b>Copyright 2022 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * SPDX-License-Identifier: Zlib
 *
 * The licensor of this software is Silicon Laboratories Inc.
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 *
 ******************************************************************************/
#include "app.h"
#include "sl_board_configuration.h"
#include "cmsis_os2.h"
#include "sl_wifi.h"
#include "sl_net.h"
#include "sl_http_client.h"
#include <string.h>
#include "sl_net_dns.h"
#include "sl_wifi_callback_framework.h"
#include "sl_net_ping.h"
#include "sl_net_wifi_types.h"
#include "sl_net_default_values.h"
#include "sl_utility.h"
#include "sl_si91x_driver.h"
#include "sl_owmhttp.h"
#include "WiFi.h"
#include "sl_wd_ntp.h"
#include "sensor.h"
#include "sl_si91x_button.h"
#include "sl_si91x_button_pin_config.h"
#include "sl_si91x_button_instances.h"

#if (APP_ENABLE_DISPLAY_DATA == 1)
#include "display.h"
#endif

// Button 0 instance
#define BUTTON_INSTANCE0    button_btn0

// Data update frequency in minutes
// Make sure that the timeout value is > 20 min!
// This is to make sure that the Maximum API calls are not exhausted
#define DATA_UPDATE_TIMEOUT 20

// Application buffer length
#define APP_BUFFER_LENGTH   2000

// Time zone offset
#define HOUR_OFFSET         0
#define MIN_OFFSET          0

// Application buffer
char app_buffer[APP_BUFFER_LENGTH];

// Application buffer index
uint32_t app_buff_index = 0;

// Variable to store the server IP address
uint8_t server_ip[16];

// Hostname for the server
char *hostname = "api.openweathermap.org";

// Base URL to acquire the Weather data
char *base_weather_url =
  "/data/2.5/weather?lat=%s&lon=%s&appid=%s&mode=json&units=metric";

// Base URL to acquire Air Quality Index data
char *base_airquality_url = "/data/2.5/air_pollution?lat=%s&lon=%s&appid=%s";

// Buffer to hold the full URL
char full_url[300]; // Adjust the size as needed

// Define the latitude, longitude, and API key
char *latitude = "xx.xxx";
char *longitude = "xx.xxx";
char *api_key = "xxxxxxxxxxxxxxxxxxxxxxx";  // Replace with your actual API key

uint8_t http_rsp_received = HTTP_ASYNC_AWAIT;

// I2C instance for the Si70xx sensor
sl_i2c_instance_t i2c_instance = SL_I2C2;

// I2C address of the Si70xx sensor
uint8_t i2c_address = SI7021_ADDR;

aqi_data_t aqi_data;
weather_data_t weather_data;
sl_calendar_datetime_config_t datetime_config;

#if (APP_ENABLE_DISPLAY_DATA == 1)
GLIB_Context_t glibContextText, glibContextNumbers;
#endif

uint8_t disp_state = 0;
bool wifi_connected = false;

osEventFlagsId_t update_evt_id;

// Update timer handle
osTimerId_t update_timer_handle;

// Timer Attributes
const osTimerAttr_t update_timer = {
  .name = "update_timer",
  .attr_bits = 0,
};

/******************************************************
 *          Callback Function Definitions
 ******************************************************/
// Timer callback function
void update_timer_callback()
{
  osEventFlagsSet(update_evt_id, 0x00000001U);
}

// Button press ISR
void sl_si91x_button_isr(uint8_t pin, int8_t state)
{
  if ((pin == BUTTON_INSTANCE0.pin) && (state == BUTTON_PRESSED)) {
    disp_state++;
    if (disp_state > DISP_AQI_REPORT) {
      disp_state = DISP_TIME;
    }
  }
}

/******************************************************
 *               Variable Definitions
 ******************************************************/
const osThreadAttr_t deviceInit_thread_attributes = {
  .name = "deviceInit",
  .attr_bits = 0,
  .cb_mem = 0,
  .cb_size = 0,
  .stack_mem = 0,
  .stack_size = 3072,
  .priority = osPriorityLow,
  .tz_module = 0,
  .reserved = 0,
};

const osThreadAttr_t app_thread_attributes = {
  .name = "appProcess",
  .attr_bits = 0,
  .cb_mem = 0,
  .cb_size = 0,
  .stack_mem = 0,
  .stack_size = 2048,
  .priority = osPriorityLow,
  .tz_module = 0,
  .reserved = 0,
};

const osThreadAttr_t update_thread_attributes = {
  .name = "dataUpdate",
  .attr_bits = 0,
  .cb_mem = 0,
  .cb_size = 0,
  .stack_mem = 0,
  .stack_size = 2048,
  .priority = osPriorityLow,
  .tz_module = 0,
  .reserved = 0,
};

/******************************************************
 *               Function Declarations
 ******************************************************/
static void device_init(void *argument);

static void app_process(void *argument);

static void data_update(void *argument);

static void acquire_weatherData();

static void initialize_si70xx();

static void acquire_dateTime();

static void acquire_aqiData();

/******************************************************
 *               Function Definitions
 ******************************************************/
void app_init(void)
{
  osThreadNew((osThreadFunc_t)device_init, NULL, &deviceInit_thread_attributes);

  osThreadNew((osThreadFunc_t)data_update, NULL, &update_thread_attributes);

  update_evt_id = osEventFlagsNew(NULL);
  if (update_evt_id == NULL) {
    printf("failed to create event id\r\n"); // Event Flags object not created, handle failure
  }
}

static void device_init(void *argument)
{
  UNUSED_PARAMETER(argument);
  sl_status_t status;

  status = WiFi_init();
  if (status != SL_STATUS_OK) {
    printf("Failed to bring Wi-Fi client interface up: 0x%lx\r\n", status);
  } else {
    printf("WiFi Client Interface Initialized\r\n");
    wifi_connected = true;
  }

  status = sl_resolve_IP(hostname, server_ip);
  if (status != SL_STATUS_OK) {
    printf("Unexpected error while resolving dns, Error 0x%lX\r\n", status);
  }

  // Acquire the Weather Data
  acquire_weatherData();

  // Acquire Air Quality Index Data
  acquire_aqiData();

  // Acquire the Current Date and Time
  acquire_dateTime();

  // Initialize the Si70xx sensor on the WSTK
  initialize_si70xx();

  status = WiFi_deinit();
  if (status != SL_STATUS_OK) {
    printf("Failed to Deinit Wi-Fi client interface up: 0x%lx\r\n", status);
  } else {
    printf("WiFi Client Interface De-initialized\r\n");
    wifi_connected = false;
  }

#if (APP_ENABLE_DISPLAY_DATA == 1)
  status = sl_display_init(&glibContextText, &glibContextNumbers);
#endif

  if (status != SL_STATUS_OK) {
    printf("Failed to initialize the display\r\n");
  }

  // Start the App process thread
  osThreadNew((osThreadFunc_t)app_process, NULL, &app_thread_attributes);

  disp_state = DISP_TIME;

  // Initialize the update timer
  update_timer_handle = osTimerNew(&update_timer_callback,
                                   osTimerPeriodic,
                                   NULL,
                                   &update_timer);

  // Start timer to perform data update procedure every DATA_UPDATE_TIME min
  osTimerStart(update_timer_handle, DATA_UPDATE_TIMEOUT * 60000);

  osThreadId_t id; // id for the currently running thread
  // Get the current thread ID
  id = osThreadGetId();
  if (id == NULL) {
    // Failed to get the id
    printf("failed to get thread id\r\n");
  }

  // Suspend this current thread once the device initialization is done
  osThreadSuspend(id);
}

static void app_process(void *argument)
{
  UNUSED_PARAMETER(argument);

  sl_status_t status = SL_STATUS_OK;
  while (true)
  {
    if (disp_state == DISP_TIME) {
      status = sl_si91x_calendar_get_date_time(&datetime_config);
      if (status != SL_STATUS_OK) {
        printf(
          "sl_si91x_calendar_get_date_time: Invalid Parameters, Error Code : %lu \n",
          status);
      }

#if (APP_ENABLE_DISPLAY_DATA == 1)
      sl_display_time(&glibContextText, &glibContextNumbers, &datetime_config);
#else
      printf("Time: %.2d:%.2d:%.2d\r\n",
             datetime_config.Hour,
             datetime_config.Minute,
             datetime_config.Second);
#endif
    } else if (disp_state == DISP_DATE) {
      status = sl_si91x_calendar_get_date_time(&datetime_config);
      if (status != SL_STATUS_OK) {
        printf(
          "sl_si91x_calendar_get_date_time: Invalid Parameters, Error Code : %lu \n",
          status);
      }

#if (APP_ENABLE_DISPLAY_DATA == 1)
      sl_display_date(&glibContextText, &glibContextNumbers, &datetime_config);
#else
      printf("Date: %.2d:%.2d:%.2d\r\n",
             datetime_config.Day,
             datetime_config.Month,
             datetime_config.Year);
#endif
    } else if (disp_state == DISP_OUTDOOR_CLIMATE) {
#if (APP_ENABLE_DISPLAY_DATA == 1)
      sl_display_outdoorClimate(&glibContextText,
                                &glibContextNumbers,
                                &weather_data);
#else
      printf("Outdoor Climate: Temperature = %.2d, Humidity =  %.2d\r\n",
             (uint16_t)weather_data.temp, (uint16_t)weather_data.humidity);
#endif
    } else if (disp_state == DISP_INDOOR_CLIMATE) {
      status = sl_si91x_si70xx_measure_rh_and_temp(i2c_instance,
                                                   i2c_address,
                                                   &weather_data.local_humidity,
                                                   &weather_data.local_temperature);
      if (status != SL_STATUS_OK) {
        printf("Sensor temperature read failed, Error Code: 0x%ld \n", status);
      }

#if (APP_ENABLE_DISPLAY_DATA == 1)
      sl_display_indoorClimate(&glibContextText,
                               &glibContextNumbers,
                               &weather_data);
#else
      printf("Indoor Climate: Temperature = %.2d, Humidity =  %.2d\r\n",
             (uint16_t)weather_data.local_temperature,
             (uint16_t)weather_data.local_humidity);
#endif
    } else if (disp_state == DISP_WEATHER_REPORT) {
#if (APP_ENABLE_DISPLAY_DATA == 1)
      sl_display_weatherReport(&glibContextText,
                               &glibContextNumbers,
                               &weather_data);
#else
      printf("-------------------------------------\r\n");
      printf("Weather Report:\r\n");
      printf("Temperature: %.1f°C\r\n", weather_data.temp);
      printf("Feels Like: %.1f°C\r\n", weather_data.feels_like);
      printf("Humidity: %d%%\r\n", weather_data.humidity);
      printf("Rain(1h): %.1fmm\r\n", weather_data.rain);
      printf("-------------------------------------\r\n");
#endif
    } else if (disp_state == DISP_AQI_REPORT) {
#if (APP_ENABLE_DISPLAY_DATA == 1)
      sl_display_aqiReport(&glibContextText, &glibContextNumbers, &aqi_data);
#else

      printf("-------------------------------------\r\n");
      printf("AQI Report:\r\n");
      printf("AQI: %s\r\n", get_aqi_category(aqi_data.aqi));
      printf("Ozone(O3): %.1f\r\n", aqi_data.ozone);
      printf("PM2.5: %.1f\r\n", aqi_data.pm2_5);
      printf("PM10: %.1f\r\n", aqi_data.pm10);
      printf("CO: %.1f\r\n", aqi_data.co);
      printf("-------------------------------------\r\n");

#endif
    }

    osDelay(1000);
  }
}

static void data_update(void *argument)
{
  UNUSED_PARAMETER(argument);
  sl_status_t status;

  while (1)
  {
    osEventFlagsWait(update_evt_id, 0x00000001U, osFlagsWaitAny, osWaitForever);
    printf(" Performing data update\r\n");

    if (!wifi_connected) {
      status = WiFi_init();
      if (status != SL_STATUS_OK) {
        printf("WiFi init failed %lX \r\n ", status);
      } else {
        printf("Wi-Fi Client Connected\r\n");
        wifi_connected = true;

        osDelay(1000);

        acquire_weatherData();
        acquire_aqiData();
        acquire_dateTime();

        status = WiFi_deinit();
        if (status != SL_STATUS_OK) {
          printf("WiFi deinit failed\r\n");
        } else {
          printf("Wi-Fi Client Disconnected\r\n");
          wifi_connected = false;
        }
      }
    } else if (wifi_connected) {
      acquire_weatherData();
      acquire_aqiData();
      acquire_dateTime();
    }

    osEventFlagsClear(update_evt_id, 0x00000001U);
  }
}

sl_status_t http_get_response_callback_handler(const sl_http_client_t *client,
                                               sl_http_client_event_t event,
                                               void *data,
                                               void *request_context)
{
  UNUSED_PARAMETER(client);
  UNUSED_PARAMETER(event);
  UNUSED_PARAMETER(request_context);

  sl_http_client_response_t *get_response = (sl_http_client_response_t *)data;

  // Uncomment for debugging the HTTP response
  //  printf("===========HTTP GET RESPONSE START===========\r\n");
  //  printf(
  //    "\r\n> Status: 0x%ld\n> GET response: %u\n> End of data: %lu\n> Data Length: %u\n> Request Context: %s\r\n",
  //    get_response->status,
  //    get_response->http_response_code,
  //    get_response->end_of_data,
  //    get_response->data_length,
  //    (char *)request_context);

  if ((get_response->status != SL_STATUS_OK)
      || ((get_response->http_response_code >= 400)
          && (get_response->http_response_code <= 599)
          && (get_response->http_response_code != 0))) {
    http_rsp_received = HTTP_FAILURE_RESPONSE;
    return get_response->status;
  }

  if (get_response->data_length) {
    if (APP_BUFFER_LENGTH > (app_buff_index + get_response->data_length)) {
      memcpy(app_buffer + app_buff_index,
             get_response->data_buffer,
             get_response->data_length);
    }
    app_buff_index += get_response->data_length;
  }

  // Uncomment for debugging
  //  printf("GET Data response:\n%s \nOffset: %ld\r\n", app_buffer, app_buff_index);
  if (get_response->end_of_data == 1) {
    app_buff_index = 0;
    http_rsp_received = HTTP_SUCCESS_RESPONSE;
  }

  return SL_STATUS_OK;
}

static void acquire_weatherData()
{
  sl_status_t status = SL_STATUS_OK;

  printf("Acquiring Weather Data\r\n");

  // clearing app_buffer data if any
  memset(app_buffer, 0, sizeof(app_buffer)); // Clear buffer data

  sl_gen_url(base_weather_url, api_key, latitude, longitude, full_url);

  status = sl_http_client_begin(server_ip, full_url, &http_rsp_received);
  if (status != SL_STATUS_OK) {
    printf("Unexpected error while HTTP client operation: 0x%lX\r\n",
           status);
  }

  parse_jsonString(app_buffer, NULL, &weather_data, PARSE_WEATHER_DATA);
}

void acquire_aqiData()
{
  sl_status_t status = SL_STATUS_OK;

  printf("Acquiring AQI Data\r\n");

  // clearing app_buffer data if any
  memset(app_buffer, 0, sizeof(app_buffer)); // Clear buffer data

  sl_gen_url(base_airquality_url, api_key, latitude, longitude, full_url);

  status = sl_http_client_begin(server_ip, full_url, &http_rsp_received);
  if (status != SL_STATUS_OK) {
    printf("Unexpected error while HTTP client operation: 0x%lX\r\n",
           status);
  }

  parse_jsonString(app_buffer, &aqi_data, NULL, PARSE_AQI_DATA);
}

void acquire_dateTime()
{
  sl_status_t status = SL_STATUS_OK;

  printf("Network Date and Time\r\n");

  // clearing app_buffer data if any
  memset(app_buffer, 0, sizeof(app_buffer)); // Clear buffer data

  status = SNTP_client_init(); // Initailize the SNTP client
  if (status != SL_STATUS_OK) {
    printf("Failed to init SNTP client 0x%lx\r\n", status);
  }

  // Get date and time
  status = SNTP_get_Datetime((uint8_t *)app_buffer, APP_BUFFER_LENGTH);
  if (status != SL_STATUS_OK) {
    printf("Failed to get Date and Time 0x%lx\r\n", status);
  }

  status = sl_si91x_calendar_set_configuration(KHZ_RC_CLK_SEL);
  if (status != SL_STATUS_OK) {
    printf(
      "sl_si91x_calendar_set_configuration: Invalid Parameters, Error Code : %lu \n",
      status);
  }

  // Initialize the calendar
  sl_si91x_calendar_init();

  // time zone offset
  timezone_offset_t offset;
  offset.hours = HOUR_OFFSET;
  offset.min = MIN_OFFSET;

  parse_datetime(app_buffer, &datetime_config, &offset); // Parse the NTP to sl_calendar_datetime_config_t and adjust time for offset

  calendar_print_datetime(datetime_config);

  status = sl_si91x_calendar_set_date_time(&datetime_config);
  if (status != SL_STATUS_OK) {
    printf(
      "sl_si91x_calendar_set_date_time: Invalid Parameters, Error Code : %lu \n",
      status);
  }

  status = SNTP_client_deinit();
  if (status != SL_STATUS_OK) {
    printf("Failed to de-init NTP client 0x%lx\r\n", status);
  }
}

void initialize_si70xx()
{
  sl_status_t status = SL_STATUS_OK;

  status = si70xx_init(i2c_instance, i2c_address);
  if (status != SL_STATUS_OK) {
    printf("Failed to Initialze the I2C si7021 sensor 0x%lx\r\n", status);
  }

  status = sl_si91x_si70xx_measure_rh_and_temp(i2c_instance,
                                               i2c_address,
                                               &weather_data.local_humidity,
                                               &weather_data.local_temperature);
  if (status != SL_STATUS_OK) {
    printf("Sensor temperature read failed, Error Code: 0x%ld \n", status);
  }
}

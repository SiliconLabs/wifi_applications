/*
 * @file sl_wd_ntp.c
 * @brief Top level NTP functions
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

#include "sl_wd_ntp.h"

#define SNTP_METHOD         SL_SNTP_UNICAST_MODE
#define FLAGS               0
#define NTP_SERVER_IP       "0.pool.ntp.org" // Most likely "162.159.200.123"
#define SNTP_TIMEOUT        50
#define SNTP_API_TIMEOUT    0
#define ASYNC_WAIT_TIMEOUT  60000
#define DNS_TIMEOUT         20000
#define MAX_DNS_RETRY_COUNT 5
#define DNS_SERVER1_IP      "8.8.8.8"
#define DNS_SERVER2_IP      "8.8.4.4"

static uint8_t callback_event = 0;
static sl_status_t cb_status = SL_STATUS_FAIL;

// static char *event_type[]     = { [SL_SNTP_CLIENT_START]           = "SNTP Client Start",
//                                  [SL_SNTP_CLIENT_GET_TIME]        = "SNTP Client Get Time",
//                                  [SL_SNTP_CLIENT_GET_TIME_DATE]   = "SNTP Client Get Time and Date",
//                                  [SL_SNTP_CLIENT_GET_SERVER_INFO] = "SNTP Client Get Server Info",
//                                  [SL_SNTP_CLIENT_STOP]            = "SNTP Client Stop" };

static void sntp_client_event_handler(sl_sntp_client_response_t *response,
                                      uint8_t *user_data,
                                      uint16_t user_data_length)
{
  uint16_t length = 0;

//  printf("\nReceived %s SNTP event with status %s\n",
//         event_type[response->event_type],
//         ((uint8_t)SL_STATUS_OK == response->status) ? "Success" : "Failed");

  if ((uint8_t)SL_STATUS_OK == response->status) {
    (response->data_length
     > user_data_length ? (length = user_data_length) : (length =
                                                           response->data_length));

    memcpy(user_data, response->data, length);
  }

  callback_event = response->event_type;
  cb_status = response->status;

  return;
}

sl_status_t SNTP_client_init(void)
{
  uint32_t start = 0;
  sl_status_t status;
  sl_ip_address_t address = { 0 };
  sl_sntp_client_config_t config = { 0 };

  int32_t dns_retry_count = MAX_DNS_RETRY_COUNT;

  // Convert DNS server IP addresses to sl_ip_address_t structures
  sl_ip_address_t primary_dns_server;
  sl_ip_address_t secondary_dns_server;

  primary_dns_server.type = SL_IPV4;
  primary_dns_server.ip.v4.value = ip_to_reverse_hex(DNS_SERVER1_IP);

  secondary_dns_server.type = SL_IPV4;
  secondary_dns_server.ip.v4.value = ip_to_reverse_hex(DNS_SERVER2_IP);

  // Create sl_net_set_dns_address_t structure
  sl_net_dns_address_t dns_address;
  dns_address.primary_server_address = &primary_dns_server;
  dns_address.secondary_server_address = &secondary_dns_server;

  // Set DNS server addresses
  status = sl_net_set_dns_server(SL_NET_WIFI_CLIENT_INTERFACE, &dns_address);

  do {
    status = sl_net_dns_resolve_hostname(NTP_SERVER_IP,
                                         DNS_TIMEOUT,
                                         SL_NET_DNS_TYPE_IPV4,
                                         &address);
    dns_retry_count--;
  } while ((dns_retry_count != 0) && (status != SL_STATUS_OK));

  printf("SNTP sever IP : %u.%u.%u.%u\r\n",
         address.ip.v4.bytes[0],
         address.ip.v4.bytes[1],
         address.ip.v4.bytes[2],
         address.ip.v4.bytes[3]);

  config.server_host_name = address.ip.v4.bytes;
  config.sntp_method = SNTP_METHOD;
  config.sntp_timeout = SNTP_TIMEOUT;
  config.event_handler = sntp_client_event_handler;
  config.flags = FLAGS;

  cb_status = SL_STATUS_FAIL;
  status = sl_sntp_client_start(&config, SNTP_API_TIMEOUT);
  if ((SNTP_API_TIMEOUT == 0) && (SL_STATUS_IN_PROGRESS == status)) {
    start = osKernelGetTickCount();

    while ((SL_SNTP_CLIENT_START != callback_event)
           && (osKernelGetTickCount() - start) <= ASYNC_WAIT_TIMEOUT) {
      osThreadYield();
    }

    if (cb_status != SL_STATUS_OK) {
      printf("Failed to start Async SNTP client: 0x%lx\r\n", status);
      return cb_status;
    }

    status = SL_STATUS_OK;
  }

  return cb_status;
}

sl_status_t SNTP_get_time(uint8_t *buffer, uint32_t buffer_length)
{
  sl_status_t status;
  uint32_t start = 0;

  cb_status = SL_STATUS_FAIL;
  status = sl_sntp_client_get_time(buffer, buffer_length, SNTP_API_TIMEOUT);
  if ((SNTP_API_TIMEOUT == 0) && (SL_STATUS_IN_PROGRESS == status)) {
    start = osKernelGetTickCount();

    while ((SL_SNTP_CLIENT_GET_TIME != callback_event)
           && (osKernelGetTickCount() - start) <= ASYNC_WAIT_TIMEOUT) {
      osThreadYield();
    }

    if (cb_status != SL_STATUS_OK) {
      printf("Failed to get async time from ntp server : 0x%lx\r\n", status);
      return cb_status;
    }
  } else {
    if (status == SL_STATUS_OK) {
      printf("SNTP Client got TIME successfully\n");
    } else {
      printf("Failed to get time from ntp server : 0x%lx\r\n", status);
      return status;
    }
  }

  return cb_status;
}

sl_status_t SNTP_get_Datetime(uint8_t *buffer, uint32_t buffer_length)
{
  uint32_t start = 0;
  sl_status_t status;

  cb_status = SL_STATUS_FAIL;
  status =
    sl_sntp_client_get_time_date(buffer, buffer_length, SNTP_API_TIMEOUT);
  if ((SNTP_API_TIMEOUT == 0) && (SL_STATUS_IN_PROGRESS == status)) {
    start = osKernelGetTickCount();

    while ((SL_SNTP_CLIENT_GET_TIME_DATE != callback_event)
           && (osKernelGetTickCount() - start) <= ASYNC_WAIT_TIMEOUT) {
      osThreadYield();
    }

    if (cb_status != SL_STATUS_OK) {
      printf("Failed to get Async date and time from ntp server : 0x%lx\r\n",
             status);
      return cb_status;
    }
  } else {
    if (status == SL_STATUS_OK) {
      printf("SNTP Client got TIME and DATE successfully\n");
    } else {
      printf("Failed to get date and time from ntp server : 0x%lx\r\n", status);
      return status;
    }
  }

  return cb_status;
}

sl_status_t SNTP_client_deinit(void)
{
  uint32_t start = 0;
  sl_status_t status;

  cb_status = SL_STATUS_FAIL;
  status = sl_sntp_client_stop(SNTP_API_TIMEOUT);
  if ((SNTP_API_TIMEOUT == 0) && (SL_STATUS_IN_PROGRESS == status)) {
    start = osKernelGetTickCount();

    while ((SL_SNTP_CLIENT_STOP != callback_event)
           && (osKernelGetTickCount() - start) <= ASYNC_WAIT_TIMEOUT) {
      osThreadYield();
    }

    if (cb_status != SL_STATUS_OK) {
      printf("Failed to stop Async SNTP client: 0x%lx\r\n", status);
      return cb_status;
    }
  } else {
    if (status == SL_STATUS_OK) {
      printf("SNTP Client stopped successfully\n");
    } else {
      printf("Failed to stop SNTP client: 0x%lx\r\n", status);
      return status;
    }
  }
  return cb_status;
}

RTC_MONTH_T convertMonthToEnum(const char month[4])
{
  if (strncmp(month, "Jan", 3) == 0) {
    return January;
  } else if (strncmp(month, "Feb", 3) == 0) {
    return February;
  } else if (strncmp(month, "Mar", 3) == 0) {
    return March;
  } else if (strncmp(month, "Apr", 3) == 0) {
    return April;
  } else if (strncmp(month, "May", 3) == 0) {
    return May;
  } else if (strncmp(month, "Jun", 3) == 0) {
    return June;
  } else if (strncmp(month, "Jul", 3) == 0) {
    return July;
  } else if (strncmp(month, "Aud", 3) == 0) {
    return August;
  } else if (strncmp(month, "Sep", 3) == 0) {
    return September;
  } else if (strncmp(month, "Oct", 3) == 0) {
    return October;
  } else if (strncmp(month, "Nov", 3) == 0) {
    return November;
  } else if (strncmp(month, "Dec", 3) == 0) {
    return December;
  } else {
    return 0;
  }
}

RTC_DAY_OF_WEEK_T calculateDayOfWeek(uint8_t day,
                                     uint8_t month,
                                     uint8_t year,
                                     uint8_t century)
{
  if (month < 3) {
    month += 12;
    year -= 1;
  }

  century -= 1;

  // Zellers Congruence formula
  uint8_t dayOfWeek =
    (day + (13 * (month + 1)) / 5 + year + (year / 4) + (century / 4) + 5
     * century) % 7;

  // Return in RTC_DAY_OF_WEEK_T format
  return ((dayOfWeek + 5) % 7) + 1;
}

sl_status_t parse_datetime(const char *data,
                           RTC_TIME_CONFIG_T *datetime_config,
                           timezone_offset_t *offset)
{
  char month[4];
  int day, year, hour, minute, second, milliseconds;
  char timezone[4];

  sscanf(data, " %4s %d, %d %d:%d:%d.%d %3s", month,
         &day, &year, &hour, &minute, &second, &milliseconds, timezone);

  datetime_config->Day = day;
  datetime_config->Hour = hour;
  datetime_config->Minute = minute;
  datetime_config->Second = second;
  datetime_config->MilliSeconds = milliseconds;
  datetime_config->Century = (uint8_t)((year / 100) + 1) - 20;
  datetime_config->Year = (uint8_t)(year % 100);

  // Buffer adjustment
  memmove(month, month + 1, strlen(month));

  // converting UTC TO local time based on the offset
  datetime_config->Hour += offset->hours;
  if (datetime_config->Hour > 24) {
    datetime_config->Hour = 1;
  }
  datetime_config->Minute += offset->min;
  if (datetime_config->Minute > 60) {
    if (datetime_config->Minute - 60 == 0) {
      datetime_config->Minute = 1;
    } else {
      datetime_config->Minute = (datetime_config->Minute - 60);
      datetime_config->Hour += 1;
    }
  }

  datetime_config->Month = convertMonthToEnum(month);

  // Caliculate the day of the week using Zellers Congurance
  datetime_config->DayOfWeek = calculateDayOfWeek(datetime_config->Day,
                                                  datetime_config->Month,
                                                  datetime_config->Year,
                                                  datetime_config->Century);

  return SL_STATUS_OK;
}

uint64_t ip_to_reverse_hex(char *ip)
{
  uint32_t ip1, ip2, ip3, ip4;
  uint64_t ip_hex;
  uint32_t status;

  status = sscanf(ip, "%lu.%lu.%lu.%lu", &ip1, &ip2, &ip3, &ip4);
  if (status != 4) {
    return 0x00000000; // Problem if we actually pass 0.0.0.0
  }

  ip_hex = (uint64_t)ip1;
  ip_hex |= (uint64_t)(ip2 << 8);
  ip_hex |= (uint64_t)(ip3 << 16);
  ip_hex |= (uint64_t)(ip4 << 24);

  return ip_hex;
}

// Function to get the string for day of the week
const char * get_day_of_week(RTC_DAY_OF_WEEK_T day)
{
  switch (day) {
    case Sunday:    return "Sunday";
    case Monday:    return "Monday";
    case Tuesday:   return "Tuesday";
    case Wednesday: return "Wednesday";
    case Thursday:  return "Thursday";
    case Friday:    return "Friday";
    case Saturday:  return "Saturday";
    default:        return "Invalid Day";
  }
}

// Function to get the string for the month
const char * get_month_name(RTC_MONTH_T month)
{
  switch (month) {
    case January:   return "January";
    case February:  return "February";
    case March:     return "March";
    case April:     return "April";
    case May:       return "May";
    case June:      return "June";
    case July:      return "July";
    case August:    return "August";
    case September: return "September";
    case October:   return "October";
    case November:  return "November";
    case December:  return "December";
    default:        return "Invalid Month";
  }
}

// Function to print the Date and Time information
void calendar_print_datetime(sl_calendar_datetime_config_t data)
{
  printf("***Calendar time****\r\n");
  printf("Time Format: hour:%d, min:%d, sec:%d, msec:%d\r\n",
         data.Hour,
         data.Minute,
         data.Second,
         data.MilliSeconds);
  printf("Date Format: DD/MM/YY: %.2d/%.2d/%.2d \r\n",
         data.Day,
         data.Month,
         data.Year);
  printf("Century: %d\r\n", data.Century + 20);
  printf("\r\n");
}

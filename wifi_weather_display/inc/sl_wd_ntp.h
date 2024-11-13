/*
 * @file sl_wd_ntp.h
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

#ifndef SL_WD_NTP_H_
#define SL_WD_NTP_H_

#include "sl_constants.h"
#include "sl_sntp.h"
#include "sl_wifi.h"
#include "sl_net_dns.h"
#include "sl_wifi_callback_framework.h"
#include "sl_si91x_types.h"
#include "string.h"
#include "sl_net.h"
#include "sl_utility.h"
#include "cmsis_os2.h"
#include "sl_si91x_calendar.h"
#include "rsi_rtc.h"

typedef struct timezone_offset
{
  uint8_t hours;
  uint8_t min;
} timezone_offset_t;

// Initialize the device as an NTP client
sl_status_t SNTP_client_init(void);

// De-initialize the device as an NTP client
sl_status_t SNTP_client_deinit(void);

// Get the network time
sl_status_t SNTP_get_time(uint8_t *buffer, uint32_t buffer_length);

// Get the network date and time in NTP format
sl_status_t SNTP_get_Datetime(uint8_t *buffer, uint32_t buffer_length);

// Helper function to convert IP to HEX
uint64_t ip_to_reverse_hex(char *ip);

// Helper function to calculate the day of the week using Zellers congruence
RTC_DAY_OF_WEEK_T calculateDayOfWeek(uint8_t day,
                                     uint8_t month,
                                     uint8_t year,
                                     uint8_t century);

// Helper function to parse the NTP date/time information into RTC_TIME_CONFIG_T
sl_status_t parse_datetime(const char *data,
                           RTC_TIME_CONFIG_T *datetime_config,
                           timezone_offset_t *offset);

// Helper function to parse NTP date information to RTC_MONTH_T
RTC_MONTH_T convertMonthToEnum(const char month[4]);

// Helper function to print data to the serial terminal
void calendar_print_datetime(sl_calendar_datetime_config_t data);

// Function to get the string for day of the week
const char *get_day_of_week(RTC_DAY_OF_WEEK_T day);

// Function to get the string for the month
const char *get_month_name(RTC_MONTH_T month);

#endif /* SL_WD_NTP_H_ */

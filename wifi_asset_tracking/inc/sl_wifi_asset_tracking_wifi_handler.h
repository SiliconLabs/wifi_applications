/***************************************************************************/ /**
 * @file sl_wifi_asset_tracking_wifi_handler.h
 * @brief wifi connection and data capture related header file
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

#ifndef SL_WIFI_ASSET_TRACKING_WIFI_HANDLER_H_
#define SL_WIFI_ASSET_TRACKING_WIFI_HANDLER_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <sl_status.h>
#include <sl_si91x_calendar.h>

/*******************************************************************************
 ***************************  Defines / Macros  ********************************
 ******************************************************************************/
#define MAX_WIFI_CONN_RETRY_COUNT            5      ///< In numbers
#define WIFI_CONN_DELAY_BETN_RETRY           15000  ///< In ms
#define KEEP_ALIVE_INTERVAL                  10     ///< In seconds
#define WIFI_PACKET_TYPE                     0x01   ///< Wi-Fi packet type
#define KEEP_ALIVE_PACKET_TYPE               0x02   ///< Keep alive packet types
#define MAX_LIMIT_OF_WIFI_SAMPLING_INTERVAL  600    ///< Maximum sampling interval of wi-fi
#define MIN_LIMIT_OF_WIFI_SAMPLING_INTERVAL  60     ///< Minimum sampling interval of wi-fi

#define NTP_SERVER_IP                        "0.pool.ntp.org"       ///< NTP server IP
#define SNTP_METHOD                          SL_SNTP_UNICAST_MODE   ///< SNTP method used
#define SNTP_FLAGS                           0                      ///< SNTP flags
#define SNTP_DATA_BUFFER_LENGTH              50                     ///< SNTP buffer length
#define SNTP_TIMEOUT                         7000   ///< In ms
#define SNTP_API_TIMEOUT                     7000   ///< In ms
#define NTP_TIME_BUFFER_TOKENS_STRING        ", :." ///< Tokens for NTP time buffer string
#define MAX_NTP_TIME_BUFFER_TOKEN_SIZE       10     ///< Maximum size for NTP time buffer token
#define MAX_NTP_TIME_BUFFER_TOKENS           7      ///< Maximum number of NTP time buffer
#define NTP_MONTH_NAME_SIZE                  3      ///< NTP month name size
#define NTP_MONTH_BUFFER_INDEX               0      ///< NTP month buffer index

#define DEF_CAL_CENTURY                      2u       ///< Default calendar century
#define DEF_CAL_YEAR                         0u       ///< Default calendar year
#define DEF_CAL_MONTH                        January  ///< Default calendar month
#define DEF_CAL_DAY_OF_WEEK                  Saturday ///< Default calendar day of the week
#define DEF_CAL_DAY                          1u       ///< Default calendar day value
#define DEF_CAL_HOUR                         0u       ///< Default calendar hour value
#define DEF_CAL_MINUTE                       0u       ///< Default calendar minute value
#define DEF_CAL_SECONDS                      0u       ///< Default calendar seconds value
#define DEF_CAL_MILLISECONDS                 00u      ///< Default calendar milli-seonds value
#define CAL_RC_CLOCK                         2u       ///< Default calendar RC clock value

/*******************************************************************************
 **************************   ENUMS and Structures   ***************************
 ******************************************************************************/

/**************************************************************************/ /**
 * @brief Structure to store calendar data.
 ******************************************************************************/
typedef struct
{
  uint8_t  century; ///< Used to configure century in calendar
  uint8_t  year; ///< Used to configure year in calendar
  uint8_t  day; ///< Used to configure day in calendar
  uint8_t  hour; ///< Used to configure hour in calendar
  uint8_t  minute; ///< Used to configure minute in calendar
  uint8_t  second; ///< Used to configure second in calendar
  uint16_t milliseconds; ///< Used to configure milliseconds in calendar
  sl_calendar_month_t month; ///< Used to configure month in calendar
  sl_calendar_days_of_week_t dayofweek; ///< Used to configure day in calendar
} sl_cal_date_time_type_t;

/**************************************************************************/ /**
 * @brief Enum to parse UTC NTP time.
 ******************************************************************************/
typedef enum
{
  SL_NTP_MONTH = 0, ///< Used to parse month from UTC NTP time format
  SL_NTP_DAY, ///< Used to parse date from UTC NTP time format
  SL_NTP_YEAR, ///< Used to parse year from UTC NTP time format
  SL_NTP_HOUR, ///< Used to parse hour from UTC NTP time format
  SL_NTP_MINUTE, ///< Used to parse minute from UTC NTP time format
  SL_NTP_SECONDS, ///< Used to parse seconds from UTC NTP time format
  SL_NTP_MS, ///< Used to parse milli-seconds from UTC NTP time format
} sl_ntp_utc_format_e;

// -----------------------------------------------------------------------------
// Prototypes

/**************************************************************************/ /**
 * @brief Callback function to capture Wi-Fi data at configured interval.
 ******************************************************************************/
void sl_capture_wifi_data_task();

/**************************************************************************/ /**
 * @brief Function to initialize and start Wi-Fi connection with configured SSID.
 * @return The following values are returned:
 * -  \ref SL_STATUS_OK on success
 * -  \ref SL_STATUS_FAIL - on sensor disable failed
 ******************************************************************************/
sl_status_t sl_start_wifi_connection();

/**************************************************************************/ /**
 * @brief Function will retry for Wi-Fi connection with configured SSID for five
 * times at every five seconds.
 * @param [init] : True If recovery during wi-fi connection initialization and
 * False If recovery during runtime
 * @return The following values are returned:
 * -  \ref SL_STATUS_OK on success
 * -  \ref SL_STATUS_FAIL - on sensor disable failed
 ******************************************************************************/
sl_status_t sl_retry_wifi_connection(bool init);

/**************************************************************************/ /**
 * @brief Function will setup SNTP and fetch date and time in UTC format from
 * network and configure calendar RTC using fetched data.
 * times at every five seconds.
 * @return The following values are returned:
 * -  \ref SL_STATUS_OK on success
 * -  \ref SL_STATUS_FAIL - on sensor disable failed
 ******************************************************************************/
sl_status_t sl_set_time_and_date_using_sntp();

/**************************************************************************/ /**
 * @brief Function will fetch current RSSI value using SL_WIFI_CLIENT_INTERFACE.
 * @param[out] rssi : output RSSI data
 * @return The following values are returned:
 * -  \ref SL_STATUS_OK on success
 * -  \ref SL_STATUS_FAIL - on sensor disable failed
 ******************************************************************************/
sl_status_t sl_get_wifi_rssi(int32_t *rssi);

/**************************************************************************/ /**
 * @brief Function will fetch MAC address using SL_WIFI_CLIENT_INTERFACE and
 * convert into string format.
 * @param[out] mac_addr : output MAC address value in string format
 * @return The following values are returned:
 * -  \ref SL_STATUS_OK on success
 * -  \ref SL_STATUS_FAIL - on sensor disable failed
 ******************************************************************************/
sl_status_t sl_get_wifi_mac_address(uint8_t *mac_addr);

/***************************************************************************/ /**
 * De-initlialize the wi-fi connection
 * @return The following values are returned:
 * -  \ref SL_STATUS_OK - on success
 * -  \ref SL_STATUS_FAIL - on de-initialized wi-fi failure
 ******************************************************************************/
sl_status_t sl_deinit_wifi_connection();

/***************************************************************************/ /**
 * Utility function used to get day of week using date.
 * @return value (0-6) means (Sunday - Saturday)
 *****************************************************************************/
uint8_t sl_get_day_of_week(uint8_t day, uint8_t month, uint16_t year);

#ifdef __cplusplus
}
#endif

#endif /* SL_WIFI_ASSET_TRACKING_WIFI_HANDLER_H_ */

/******************************************************************************/
/* EOF                                                                        */
/******************************************************************************/

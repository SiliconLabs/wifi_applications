/***************************************************************************/ /**
 * @file sl_wifi_asset_tracking_json_data_handler.h
 * @brief json data converter related header file
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

#ifndef SL_WIFI_ASSET_TRACKING_JSON_DATA_HANDLER_H_
#define SL_WIFI_ASSET_TRACKING_JSON_DATA_HANDLER_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <sl_wifi_asset_tracking_azure_handler.h>

/*******************************************************************************
 ***************************  Defines / Macros  ********************************
 ******************************************************************************/
#define QUEUE_EMPTY                                                          0                      ///< JSON queue empty state
#define JSON_PROPERTY_MSGTYPE \
  "msgtype"                                                                                         ///< String for message type
#define JSON_PROPERTY_IMU \
  "imu"                                                                                             ///< String for imu sensor message type
#define JSON_PROPERTY_TIMESTAMP \
  "timestamp"                                                                                       ///< String for timestamp message type
#define JSON_VALUE_TIMESTAMP \
  "2024-01-01 00:00:00"                                                                             ///< String for date and time
#define JSON_PROPERTY_ACCELERO \
  "accelero"                                                                                        ///< String for accelerometer values
#define JSON_PROPERTY_GYRO \
  "gyro"                                                                                            ///< String for gyroscope values
#define JSON_NULL_READING \
  "null"                                                                                            ///< String when no value is available
#define JSON_PROPERTY_GPS \
  "gps"                                                                                             ///< String for GNSS receiver message type
#define JSON_PROPERTY_LATITUDE \
  "latitude"                                                                                        ///< String for latitude value
#define JSON_PROPERTY_LONGITUDE \
  "longitude"                                                                                       ///< String for longitude value
#define JSON_PROPERTY_ALTITUDE \
  "altitude"                                                                                        ///< String for altitude value
#define JSON_PROPERTY_SATELLITE \
  "satellites"                                                                                      ///< String for number of satellites
#define JSON_PROPERTY_SATELLITE_VALUE                                        3                      ///< Satellite value
#define JSON_PROPERTY_TEMPERATURE \
  "temperature"                                                                                     ///< String for temperature message type
#define JSON_PROPERTY_HEAT \
  "heat"                                                                                            ///< String for heat message type
#define JSON_PROPERTY_UNIT \
  "unit"                                                                                            ///< String for temperature unit
#define JSON_PROPERTY_VALUE \
  "value"                                                                                           ///< String for temperature value
#define JSON_PROPERTY_HUMIDITY \
  "humidity"                                                                                        ///< String for humidity value
#define JSON_PROPERTY_KEEP_ALIVE \
  "keep-alive"                                                                                      ///< String for keep alive message type
#define JSON_PROPERTY_WIFI \
  "wifi"                                                                                            ///< String for Wi-Fi message type
#define JSON_PROPERTY_MACID \
  "macid"                                                                                           ///< String for mac ID value
#define JSON_PROPERTY_SSID \
  "ssid"                                                                                            ///< String for SSID value
#define JSON_PROPERTY_RSSI \
  "rssi"                                                                                            ///< String for RSSI value
#define JSON_PROPERTY_SESSION \
  "session"                                                                                         ///< String for session value
#define JSON_PROPERTY_NEW_SESSION_TYPE \
  "new"                                                                                             ///< String for new session type
#define JSON_PROPERTY_KEEP_ALIVE_VALUE \
  "yes"                                                                                             ///< String for keep alive message value
#define JSON_PROPERTY_INTERVAL \
  "interval"                                                                                        ///< String for interval
#define JSON_DOUBLE_FRACTION_MAX_SIZE                                        7                      ///< Maximum size for double fraction
#define JSON_MAX_TIMESTAMP_BUFF_SIZE                                         35                     ///< Maximum timestamp buffer size
#define JSON_MAX_TIMESTAMP_STRING_SIZE                                       25                     ///< Maximum size for timestamp string
#define JSON_MAX_MAC_ADDR_BUFF_SIZE                                          18                     ///< Maximum MAC address buffer size

/*******************************************************************************
 **************************   ENUMS and Structures   ***************************
 ******************************************************************************/

// -----------------------------------------------------------------------------
// Prototypes

/**************************************************************************/ /**
 * @brief Callback function to determine which sensor data format should be converted to JSON.
 * @param[in] sensor_data_queue_reading : pointer to an sensor data queue data-type.
 * @return The following values are returned:
 * -  \ref SL_STATUS_OK on success
 * -  \ref SL_STATUS_FAIL - on failure
 ******************************************************************************/
sl_status_t sl_convert_to_json_format(
  sl_wifi_asset_tracking_sensor_queue_data_t *sensor_data_queue_reading);

/**************************************************************************/ /**
 * @brief Callback function to convert bmi270 sensor data format to json data format.
 * @param[in] sensor_data_queue_reading : pointer to an sensor data queue data-type.
 * @return The following values are returned:
 * -  \ref SL_STATUS_OK on success
 * -  \ref SL_STATUS_FAIL - on failure
 ******************************************************************************/
sl_status_t sl_convert_bmi270_reading_to_json_format(
  sl_wifi_asset_tracking_sensor_queue_data_t *sensor_data_queue_reading);

/**************************************************************************/ /**
 * @brief Callback function to convert gnss sensor data format to json data format.
 * @param[in] sensor_data_queue_reading : pointer to an sensor data queue data-type.
 * @return The following values are returned:
 * -  \ref SL_STATUS_OK on success
 * -  \ref SL_STATUS_FAIL - on failure
 ******************************************************************************/
sl_status_t sl_convert_gnss_reading_to_json_format(
  sl_wifi_asset_tracking_sensor_queue_data_t *sensor_data_queue_reading);

/**************************************************************************/ /**
 * @brief Callback function to convert si7021 sensor data format to json data format.
 * @param[in] sensor_data_queue_reading : pointer to an sensor data queue data-type.
 * @return The following values are returned:
 * -  \ref SL_STATUS_OK on success
 * -  \ref SL_STATUS_FAIL - on failure
 ******************************************************************************/
sl_status_t sl_convert_si7021_reading_to_json_format(
  sl_wifi_asset_tracking_sensor_queue_data_t *sensor_data_queue_reading);

/**************************************************************************/ /**
 * @brief Callback function to convert sensor data format to json data format.
 ******************************************************************************/
void sl_json_data_converter_task();

/**************************************************************************/ /**
* @brief Function to send new session JSON message to MQTT package queue.
* @return The following values are returned:
* -  \ref SL_STATUS_OK on success
* -  \ref SL_STATUS_FAIL - on keep alive message sending failure.
*******************************************************************************/
sl_status_t sl_json_send_new_session_message();

/**************************************************************************/ /**
 * @brief Function to send keep alive JSON message to MQTT package queue.
 * @return  The following values are returned:
 * -  \ref SL_STATUS_OK on success
 * -  \ref SL_STATUS_FAIL - on keep alive message sending failure.
 ******************************************************************************/
sl_status_t sl_json_send_keep_alive_message();

/**************************************************************************/ /**
 * @brief Function to send wi-fi JSON message to MQTT package queue.
 * @return  The following values are returned:
 * -  \ref SL_STATUS_OK on success
 * -  \ref SL_STATUS_FAIL - on wi-fi message sending failure.
 ******************************************************************************/
sl_status_t sl_json_send_wifi_message();

/**************************************************************************/ /**
 * @brief Function to get time-stamp for JSON message.
 * @param[out] timestamp_buff : time-stamp will be filled in this buffer.
 * @return  The following values are returned:
 * -  \ref SL_STATUS_OK on success
 * -  \ref SL_STATUS_FAIL - on get time-stamp failure.
 ******************************************************************************/
sl_status_t sl_json_get_timestamp(uint8_t *timestamp_buff);

#ifdef __cplusplus
}
#endif

#endif /* SL_WIFI_ASSET_TRACKING_JSON_DATA_HANDLER_H_ */

/******************************************************************************/
/* EOF                                                                        */
/******************************************************************************/

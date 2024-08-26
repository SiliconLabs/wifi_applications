/***************************************************************************/ /**
 * @file sl_wifi_asset_tracking_sensor.h
 * @brief wi-fi asset tracking sensors related functions
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

#ifndef SL_WIFI_ASSET_TRACKING_SENSOR_H_
#define SL_WIFI_ASSET_TRACKING_SENSOR_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <sl_status.h>
#include <sl_si91x_i2c.h>
#include <sl_si91x_si70xx.h>
#include <sl_si91x_driver_gpio.h>

/*******************************************************************************
 ***************************  Defines / Macros  ********************************
 ******************************************************************************/
#define MAX_TEMPERATURE_UNIT_STRING_SIZE                  10      ///< Maximum string size for temperature unit
#define MAX_ACCLEROMETER_VALUES_SIZE                      3       ///< Maximum size for accelerometer values
#define MAX_GYROSCOPE_VALUES_SIZE                         3       ///< Maximum size for gyroscope values
#define MAX_INTERVAL_VALUES_SIZE                          4       ///< Maximum size for interval values
#define TX_THRESHOLD                                      0       ///< TX threshold value
#define RX_THRESHOLD                                      0       ///< RX threshold value
#define I2C                                               SL_I2C2 ///< I2C 2 instance
#define DELAY_PERIODIC_MS1                                2000    ///< sleeptimer1 periodic timeout in ms
#define UULP_GPIO_1_PIN                                   1       ///< UULP GPIO pin number 1(sensor enable)
#define SENSOR_MAX_RETRY_COUNT                            3       ///< Maximum retry count for sensor
#define SENSOR_PER_RETRY_DELAY                            5000    ///< In ms
#define GNSS_PER_RETRY_DELAY                              500     ///< In ms
#define GNSS_DATA_TIMEOUT_RETRY_DELAY                     200     ///< In ms
#define GNSS_RETRY_COUNT                                  10      ///< Retry count for GNSS receiver
#define GNSS_DATA_RETRY_COUNT                             5       ///< Retry count to receive data from
#define TEMPERATURE_UNIT_STRING                           "celsius" ///< Temperature unit used
#define MAX_TIMESTAMP_BUFF_SIZE                           28        ///< Maximum buffer size for timestamp
#define LAT_LONG_DIVISOR_UBX                              10000000  ///< Divisor value for latitude and longitude using UBX protocol
#define LAT_LONG_DIVISOR_NMEA                             1000000   ///< Divisor value for latitude and longitude using NMEA protocol
#define DELAY_TO_STABILIZE_GNSS                           60000   ///< 60 seconds delay to stabilize gnss sensor after probing
#define MAX_LIMIT_OF_TEMP_RH_SENSOR_SAMPLING_INTERVAL     120  ///< Maximum sampling interval of si7021 sensor
#define MIN_LIMIT_OF_TEMP_RH_SENSOR_SAMPLING_INTERVAL     5    ///< Minimum sampling interval of si7021 sensor
#define MAX_LIMIT_OF_IMU_SENSOR_SAMPLING_INTERVAL         60   ///< Maximum sampling interval of bmi270 sensor
#define MIN_LIMIT_OF_IMU_SENSOR_SAMPLING_INTERVAL         1    ///< Minimum sampling interval of bmi270 sensor
#define MAX_LIMIT_OF_GNSS_RECEIVER_SAMPLING_INTERVAL      600  ///< Maximum sampling interval of max-m10s gnss receiver
#define MIN_LIMIT_OF_GNSS_RECEIVER_SAMPLING_INTERVAL      60   ///< Minimum sampling interval of max-m10s gnss receiver

/// Note: FreeRTOS is not giving delay of 1000ms in 1000 ticks
#define TIMER_CLOCK_OFFSET                                1.24 ///< Change it to 1 if freeRTOS is giving 1 tick is equal to 1 ms

/*******************************************************************************
 ************************   ENUMS and Structures  ******************************
 ******************************************************************************/

/// @brief Enum for sensor queue data type
typedef enum {
  SL_INVALID_TYPE = 0, ///< Invalid sensor type
  SL_TEMP_RH_SENSOR, ///< Si7021 temperature and RH sensor type
  SL_IMU_SENSOR, ///< bmi270 IMU sensor type
  SL_GNSS_RECEIVER, ///< MAX-M10s GNSS receiver type
  SL_MAX_TYPE///< SL_MAX_TYPE
} sl_wifi_asset_tracking_sensor_queue_data_type_e;

/// @brief Structure for Si7021 temperature and RH sensor data
typedef struct {
  double temperature; ///< temperature data
  double relative_humidity; ///< relative humidity
  uint8_t  temperature_unit[MAX_TEMPERATURE_UNIT_STRING_SIZE]; ///< temperature data unit in string
} sl_temp_rh_data_t;

/// @brief Structure for bmi270 IMU sensor data
typedef struct {
  double accelerometer[MAX_ACCLEROMETER_VALUES_SIZE]; ///< accelerometer values: (x, y, z) axis
  double gyroscope[MAX_GYROSCOPE_VALUES_SIZE]; ///< gyroscope values: (x, y, z) axis
} sl_imu_data_t;

/// @brief Structure for MAX-M10s GNSS receiver data
typedef struct {
  int32_t   no_of_satellites; ///< no of satellites
  double    latitude; ///< latitude value
  double    longitude; ///< longitude value
  double    altitude; ///< altitude value
} sl_gnss_data_t;

/// @brief Structure for sensor data queue object
typedef struct {
  bool is_sensor_data_available; ///< sensor data status
  sl_wifi_asset_tracking_sensor_queue_data_type_e sensor_type; ///< sensor queue data type
  uint8_t  time_stamp[MAX_TIMESTAMP_BUFF_SIZE]; ///< sensor data time-stamp
  union {
    sl_temp_rh_data_t temp_rh_data; ///< Si7021 temperature and RH sensor data
    sl_imu_data_t imu_data; ///< bmi270 IMU sensor data
    sl_gnss_data_t gnss_data; ///< MAX-M10s GNSS receiver data
  }; ///< union of sensor queue data
} sl_wifi_asset_tracking_sensor_queue_data_t;

// -----------------------------------------------------------------------------
// Prototypes

/**************************************************************************/ /**
 * @brief Initialize I2C bus and configure FIFO threshold.
 * @return The following values are returned:
 * -  \ref SL_STATUS_OK on success
 * -  \ref SL_STATUS_FAIL - on I2C initialization and configure FIFO threshold failed
 ******************************************************************************/
sl_status_t sl_wifi_asset_tracking_master_i2c_init();

/************************************************************************* *
 * @brief Probe and initialize Si7021 (temperature & humidity sensor), bmi270(accelerometer and gyroscope sensor) and MAX-M10s (GNSS receiver).
 * @return The following values are returned:
 * -  \ref SL_STATUS_OK on success
 * -  \ref SL_STATUS_FAIL - on sensors initialization and probing failed
 *****************************************************************************
 *    sl_status_t sl_wifi_asset_tracking_sensors_init();*/

/**************************************************************************/ /**
 * @brief De-initialize required sensor modules.
 * @return The following values are returned:
 * -  \ref SL_STATUS_OK on success
 * -  \ref SL_STATUS_FAIL - on de-initialization failed
 ******************************************************************************/
sl_status_t sl_wifi_asset_tracking_sensors_deinit();

/**************************************************************************/ /**
 * @brief Probe and initialize Si7021 temperature and RH sensor.
 * @return The following values are returned:
 * -  \ref SL_STATUS_OK on success
 * -  \ref SL_STATUS_FAIL - on initialization and probing failed
 ******************************************************************************/
sl_status_t sl_init_si7021_temperature_and_rh_sensor();

/**************************************************************************/ /**
 * @brief Probe and initialize bmi270 IMU sensor.
 * @return The following values are returned:
 * -  \ref SL_STATUS_OK on success
 * -  \ref SL_STATUS_FAIL - on initialization and probing failed
 ******************************************************************************/
sl_status_t sl_init_bmi270_imu_sensor();

/**************************************************************************/ /**
 * @brief Probe and initialize MAX-M10s GNSS receiver.
 * @return The following values are returned:
 * -  \ref SL_STATUS_OK on success
 * -  \ref SL_STATUS_FAIL - on initialization and probing failed
 ******************************************************************************/
sl_status_t sl_init_max_m10s_gnss_receiver();

/**************************************************************************/ /**
 * @brief Disable on-board Si7021 temperature and RH sensor.
 * @return The following values are returned:
 * -  \ref SL_STATUS_OK on success
 * -  \ref SL_STATUS_FAIL - on sensor disable failed
 ******************************************************************************/
sl_status_t sl_disable_onboard_si7021_sensor();

/**************************************************************************/ /**
 * @brief Callback function to capture temperature and RH sensor data at configured interval.
 ******************************************************************************/
void sl_capture_temperature_rh_sensor_data_task();

/**************************************************************************/ /**
 * @brief Callback function to capture IMU sensor data at configured interval.
 ******************************************************************************/
void sl_capture_imu_sensor_data_task();

/**************************************************************************/ /**
 * @brief Callback function to capture GNSS receiver data at configured interval.
 ******************************************************************************/
void sl_capture_gnss_receiver_data_task();

/***************************************************************************/ /**
 * Callback function of temperature_rh_sensor timer, it expires when scheduler
 * is blocked in I2C read blocking API
 ******************************************************************************/
void on_temperature_rh_sensor_timer_callback();

/***************************************************************************/ /**
 * Callback function of imu sensor timer, it expires when scheduler
 * is blocked in I2C read blocking API
 ******************************************************************************/
void on_imu_sensor_timer_callback();

/***************************************************************************/ /**
 * Callback function of gnss sensor timer, it expires when scheduler
 * is blocked in I2C read blocking API
 ******************************************************************************/
void on_gnss_sensor_timer_callback();

#ifdef __cplusplus
}
#endif

#endif /* SL_WIFI_ASSET_TRACKING_SENSOR_H_ */

/******************************************************************************/
/* EOF                                                                        */
/******************************************************************************/

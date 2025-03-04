/***************************************************************************/ /**
 * @file sl_wifi_asset_tracking_demo_config.h
 * @brief wi-fi asset tracking demonstration configuration related parameters
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

#ifndef SL_WIFI_ASSET_TRACKING_DEMO_CONFIG_H_
#define SL_WIFI_ASSET_TRACKING_DEMO_CONFIG_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <sl_wifi_asset_tracking_sensor.h>
#include <sl_wifi_asset_tracking_wifi_handler.h>

/*******************************************************************************
 ***************************  Defines / Macros  ********************************
 ******************************************************************************/

/**
 * @brief Wi-Fi data sampling interval in seconds.
 * Minimum : 60 seconds
 * Maximum : 600 seconds
 * Default : 60 seconds
 */
#define DEMO_CONFIG_WIFI_SAMPLING_INTERVAL                            60
#if (DEMO_CONFIG_WIFI_SAMPLING_INTERVAL < MIN_LIMIT_OF_WIFI_SAMPLING_INTERVAL   \
                                          || DEMO_CONFIG_WIFI_SAMPLING_INTERVAL \
                                          > MAX_LIMIT_OF_WIFI_SAMPLING_INTERVAL)
#error Invalid sampling interval of wi-fi. It should be within specified range.
#endif

/**
 * @brief Si7021 Temperature and RH sensor data sampling interval in seconds.
 * Minimum : 5 seconds
 * Maximum : 120 seconds
 * Default : 5 seconds
 */
#define DEMO_CONFIG_TEMP_RH_SENSOR_SAMPLING_INTERVAL                  5
#if (DEMO_CONFIG_TEMP_RH_SENSOR_SAMPLING_INTERVAL      \
     < MIN_LIMIT_OF_TEMP_RH_SENSOR_SAMPLING_INTERVAL   \
       || DEMO_CONFIG_TEMP_RH_SENSOR_SAMPLING_INTERVAL \
     > MAX_LIMIT_OF_TEMP_RH_SENSOR_SAMPLING_INTERVAL)
#error \
  Invalid sampling interval of temperature and RH sensor. It should be within specified range.
#endif

/**
 * @brief BMI270 IMU sensor data sampling interval in seconds.
 * Minimum : 1 second
 * Maximum : 60 seconds
 * Default : 1 second
 */
#define DEMO_CONFIG_IMU_SENSOR_SAMPLING_INTERVAL                      1
#if (DEMO_CONFIG_IMU_SENSOR_SAMPLING_INTERVAL      \
     < MIN_LIMIT_OF_IMU_SENSOR_SAMPLING_INTERVAL   \
       || DEMO_CONFIG_IMU_SENSOR_SAMPLING_INTERVAL \
     > MAX_LIMIT_OF_IMU_SENSOR_SAMPLING_INTERVAL)
#error \
  Invalid sampling interval of imu sensor. It should be within specified range.
#endif

/**
 * @brief MAX-M10s GNSS receiver data sampling interval in seconds.
 * Minimum : 60 second
 * Maximum : 600 seconds
 * Default : 60 seconds
 */
#define DEMO_CONFIG_GNSS_RECEIVER_SAMPLING_INTERVAL                   60
#if (DEMO_CONFIG_GNSS_RECEIVER_SAMPLING_INTERVAL      \
     < MIN_LIMIT_OF_GNSS_RECEIVER_SAMPLING_INTERVAL   \
       || DEMO_CONFIG_GNSS_RECEIVER_SAMPLING_INTERVAL \
     > MAX_LIMIT_OF_GNSS_RECEIVER_SAMPLING_INTERVAL)
#error \
  Invalid sampling interval of gnss receiver. It should be within specified range.
#endif

/**
 * @brief Azure IoTHub Host Name.
 *
 */
#define DEMO_CONFIG_IOT_HUB_HOST_NAME \
  "<Azure IoT Hub Host Name>"

/**
 * @brief Azure IoTHub Device Id.
 *
 */
#define DEMO_CONFIG_DEVICE_ID \
  "<Azure IoT Hub Device ID>"

/**
 * @brief Azure Symmetric Key Authentication Macro
 * SYMMETRIC_KEY_AUTHENTICATION : 1 (Use symmetric authentication)
 * SYMMETRIC_KEY_AUTHENTICATION : 0 (Use X.509 authentication)
 */
#define DEMO_CONFIG_SYMMETRIC_KEY_AUTHENTICATION                      1

#if DEMO_CONFIG_SYMMETRIC_KEY_AUTHENTICATION

/**
 * @brief Azure IoT Hub Device Symmetric Key
 *
 */
#define DEMO_CONFIG_DEVICE_SYMMETRIC_KEY \
  "<Azure IoT Hub Device Symmetric Key>"

#endif

/**
 * @brief Size of the network buffer for MQTT packets.
 */
#define DEMO_CONFIG_NETWORK_BUFFER_SIZE                               (5 \
                                                                       * 1024U)

/**
 * @brief Azure IoTHub module Id.
 *
 * @note Optional argument for Azure IoTHub
 */
#define DEMO_CONFIG_MODULE_ID                                         ""

/**
 * @brief Enable to get debug logs over console.
 * Default : 0
 *
 * @note Optional argument for wi-fi asset tracking application
 */
#define DEMO_CONFIG_DEBUG_LOGS                                        1

/**
 * @brief Configure guaranteed number of samples for sensors and wi-fi as per configuration.
 * 0 : Disable guaranteed number of samples for sensors and wi-fi as per configuration.
 * 1 : Enable guaranteed number of samples for sensors and wi-fi as per configuration.
 * Default : 1
 *
 * @note Optional argument for wi-fi asset tracking application
 *
 */
#define ENABLE_SAMPLING_JITTER                                        1

#ifdef __cplusplus
}
#endif

#endif /* SL_WIFI_ASSET_TRACKING_DEMO_CONFIG_H_ */

/******************************************************************************/
/* EOF                                                                        */
/******************************************************************************/

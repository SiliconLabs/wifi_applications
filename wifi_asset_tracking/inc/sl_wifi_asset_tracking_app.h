/***************************************************************************/ /**
 * @file sl_wifi_asset_tracking_app.h
 * @brief Top level wi-fi asset tracking application functions
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

#ifndef SL_WIFI_ASSET_TRACKING_APP_H_
#define SL_WIFI_ASSET_TRACKING_APP_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <base_types.h>
#include <FreeRTOS.h>
#include <task.h>
#include <queue.h>
#include <semphr.h>
#include <timers.h>
#include <sl_wifi_asset_tracking_sensor.h>
#include <sl_wifi_asset_tracking_wifi_handler.h>
#include <sl_wifi_asset_tracking_azure_handler.h>
#include <sl_wifi_asset_tracking_json_data_handler.h>
#include <sl_wifi_asset_tracking_lcd.h>

/*******************************************************************************
 ***************************  Defines / Macros  ********************************
 ******************************************************************************/
#define PRIORITY_TEMPERATURE_RH_SENSOR_TASK                             1                           ///< Priority for temperature and humidity reading task
#define STACK_SIZE_TEMPERATURE_RH_SENSOR_TASK                           1000                        ///< Stack size for temperature and humidity reading task
#define NAME_TEMPERATURE_RH_SENSOR_TASK \
  "temp_rh_sensor_task"                                                                             ///< String for temperature and humidity reading task
#define PRIORITY_IMU_SENSOR_TASK                                        1                           ///< Priority for imu sensor reading task
#define STACK_SIZE_IMU_SENSOR_TASK                                      1000                        ///< Stack size for imu sensor reading task
#define NAME_IMU_SENSOR_TASK \
  "imu_sensor_task"                                                                                 ///< String for imu sensor reading task
#define PRIORITY_GNSS_RECEIVER_TASK                                     1                           ///< Priority for GNSS receiver reading task
#define STACK_SIZE_GNSS_RECEIVER_TASK                                   1000                        ///< Stack size for GNSS receiver reading task
#define NAME_GNSS_RECEIVER_TASK \
  "gnss_receiver_task"                                                                              ///< String for GNSS receiver reading task
#define PRIORITY_WIFI_DATA_CAPTURE_TASK                                 1                           ///< Priority for Wi-Fi data capture task
#define STACK_SIZE_WIFI_DATA_CAPTURE_TASK                               1000                        ///< Stack size for Wi-Fi data capture task
#define NAME_WIFI_DATA_CAPTURE_TASK \
  "wifi_data_capture_task"                                                                          ///< String for Wi-Fi data capture task
#define PRIORITY_JSON_DATA_CONVERTER_TASK                               2                           ///< Priority for JSON data converter task
#define STACK_SIZE_JSON_DATA_CONVERTER_TASK                             1000                        ///< Stack size for JSON data converter task
#define NAME_JSON_DATA_CONVERTER_TASK \
  "json_data_converter_task"                                                                        ///< String for JSON data converter task
#define PRIORITY_CLOUD_COMMUNICATION_TASK                               3                           ///< Priority for cloud communication task
#define STACK_SIZE_CLOUD_COMMUNICATION_TASK                             1000                        ///< Stack size for cloud communication task
#define NAME_CLOUD_COMMUNICATION_TASK \
  "cloud_communication_task"                                                                        ///< String for cloud communication task
#define PRIORITY_RECOVERY_TASK                                          4                           ///< Priority for recovery task
#define STACK_SIZE_RECOVERY_TASK                                        1000                        ///< Stack size for recovery task
#define NAME_RECOVERY_TASK \
  "recovery_task"                                                                                   ///< String for recovery task
#define PRIORITY_LCD_TASK                                               4                           ///< Priority for LCD task
#define STACK_SIZE_LCD_TASK                                             1000                        ///< Stack size for LCD task
#define NAME_LCD_TASK \
  "lcd_task"                                                                                        ///< String for LCD task
#define MAX_SIZE_OF_SENSOR_DATA_QUEUE                                   10                          ///< Maximum size of sensor data queue
#define MAX_SIZE_OF_MQTT_PACKAGE_QUEUE \
  MAX_SIZE_OF_SENSOR_DATA_QUEUE        \
  * 2                                                                                                      ///< Maximum size for MQTT package queue
#define MAX_SIZE_OF_LCD_DATA_QUEUE                                      5                           ///< Maximum size for LCD data queue
#define MAX_LCD_STRING_SIZE                                             80                          ///< Maximum string size for LCD
#define MAX_TELEMETRY_PROPERTY_BUFFER_SIZE                              80                          ///< Maximum size for telemetry buffer
#define NAME_TEMPERATURE_RH_SENSOR_TIMER \
  "temp_rh_sensor_timer"                                                                            ///< String for temperature and humidity sensor timer
#define NAME_IMU_SENSOR_TIMER \
  "imu_sensor_timer"                                                                                ///< String for imu sensor timer
#define NAME_GNSS_RECEIVER_TIMER \
  "gnss_receiver_timer"                                                                             ///< String for GNSS receiver timer
#define PERIOD_OF_TEMPERATURE_RH_SENSOR_TIMER                           2000                        ///< Period for temperature and humidity sensor timer
#define PERIOD_OF_IMU_SENSOR_TIMER                                      2000                        ///< Period for imu sensor timer
#define PERIOD_OF_GNSS_RECEIVER_TIMER                                   2000                        ///< Period for GNSS receiver timer

/*******************************************************************************
 **************************   ENUMS and Structures   ***************************
 ******************************************************************************/

/// @brief Enum for sensor status
typedef enum {
  SL_SENSOR_NOT_PROBED          = 0x00, ///< default status of sensor connectivity
  SL_SENSOR_PROBED              = 0x01, ///< If sensor probing is succeed
  SL_SENSOR_PROBE_FAILED        = 0x02, ///< If sensor probing is failed
  SL_SENSOR_CONNECTED           = 0x03, ///< If sensor is working after successful probing
  SL_SENSOR_DISCONNECTED        = 0x04, ///< If sensor connectivity is failed after successful probing
  SL_SENSOR_RECONNECTED         = 0x05, ///< If sensor reconnect after successful probing
  SL_SENSOR_SHUTDOWN            = 0x06  ///< If sensor fails after retry
} sl_siwx917_wifi_asset_tracking_sensor_status_e;

/// @brief Enum for Wi-Fi status
typedef enum {
  SL_WIFI_NOT_CONNECTED     = 0x00,  ///< default status of Wi-Fi connectivity
  SL_WIFI_CONNECTED         = 0x01,  ///< If Wi-Fi is connected successfully
  SL_WIFI_DISCONNECTED      = 0x02,  ///< If Wi-Fi connection is disconnected
  SL_WIFI_SHUTDOWN          = 0x03   ///< If Wi-Fi connectivity fails after retry
} sl_siwx917_wifi_asset_tracking_wifi_connectivity_status_e;

/// @brief Enum for cloud status
typedef enum {
  SL_CLOUD_NOT_CONNECTED     = 0x00,  ///< default status of cloud connectivity
  SL_CLOUD_CONNECTED         = 0x01,  ///< If cloud is connected successfully
  SL_CLOUD_DISCONNECTED      = 0x02,  ///< If cloud connection is disconnected
  SL_CLOUD_SHUTDOWN          = 0x03   ///< If cloud connectivity fails after retry
} sl_siwx917_wifi_asset_tracking_cloud_connectivity_status_e;

/// @brief Enum for recovery task status
typedef enum {
  SL_RECOVERY_INPROGRESS     = 0x00,  ///< If recovery task is busy
  SL_RECOVERY_IDLE           = 0x01   ///< If recovery task is not in use
} sl_siwx917_wifi_asset_tracking_recovery_task_status_e;

/// @brief Enum for indexing LCD messages/logs
typedef enum {
  INDEX_APP_START = 0,
  INDEX_BLANK_LINE,
  INDEX_SENSOR_SHUTDOWN,
  INDEX_WIFI_SHUTDOWN,
  INDEX_CLOUD_SHUTDOWN,
  INDEX_SI7021_NOT_CONNECTED,
  INDEX_BMI270_NOT_CONNECTED,
  INDEX_MAX_M10S_NOT_CONNECTED,
  INDEX_SI7021_RECONNECTING,
  INDEX_BMI270_RECONNECTING,
  INDEX_MAX_M10S_RECONNECTING,
  INDEX_SI7021_CONNECTED,
  INDEX_BMI270_CONNECTED,
  INDEX_MAX_M10S_CONNECTED,
  INDEX_WIFI_CONNECTED,
  INDEX_AZURE_CLOUD_CONNECTED,
  INDEX_WIFI_RETRY,
  INDEX_AZURE_CLOUD_RETRY
} sl_siwx917_wifi_asset_tracking_lcd_logs_indexing_e;

/// @brief Structure for tasks required in wi-fi asset tracking example
typedef struct {
  TaskHandle_t temp_rh_sensor_task_handler;            ///< temperature and RH sensor data capture task handler
  TaskHandle_t imu_sensor_task_handler;                ///< IMU sensor data capture task handler
  TaskHandle_t gnss_receiver_task_handler;             ///< GNSS receiver data capture task handler
  TaskHandle_t wifi_data_capture_task_handler;         ///< Wi-Fi data capture task handler
  TaskHandle_t json_data_converter_task_handler;       ///< JSON data converter task handler
  TaskHandle_t azure_cloud_communication_task_handler; ///< Azure cloud communication task handler
  TaskHandle_t recovery_task_handler;                  ///< Wi-Fi asset tracking application recovery task handler
  TaskHandle_t lcd_task_handler;                       ///< Wi-Fi asset tracking application LCD task handler
} sl_wifi_asset_tracking_task_list_t;

/// @brief Structure for resources required in wi-fi asset tracking example
typedef struct {
  int client_socket_id;                           ///< client socket id
  QueueHandle_t sensor_data_queue_handler;        ///< Sensor data queue handler
  QueueHandle_t sensor_data_queue_mutex_handler;  ///< Sensor data queue mutex handler
  QueueHandle_t mqtt_package_queue_handler;       ///< MQTT package queue handler
  QueueHandle_t mqtt_package_queue_mutex_handler; ///< MQTT package queue mutex handler
  QueueHandle_t lcd_queue_handler;                ///< LCD data queue handler
  QueueHandle_t recovery_status_mutex_handler;    ///< Recovery in progress status mutex handler
  SemaphoreHandle_t i2c_mutex_handler;            ///< I2C transaction mutex handler
  TimerHandle_t temperature_rh_sensor_timer;      ///< si7021 sensor timer handler
  TimerHandle_t imu_sensor_timer;                 ///< bmi270 sensor timer handler
  TimerHandle_t gnss_sensor_timer;                ///< gnss sensor timer handler
  sl_wifi_asset_tracking_task_list_t task_list;   ///< Task required in wi-fi asset tracking example
  AzureIoTHubClient_t azure_iot_hub_client;       ///< Azure IoT Hub client resource
  AzureIoTMessageProperties_t azure_msg_property_bag; ///< Azure tele-metry messages properties bag
  uint8_t azure_msg_property_buff[MAX_TELEMETRY_PROPERTY_BUFFER_SIZE]; ///< Azure message property buffer
} sl_wifi_asset_tracking_resource_t;

/// @brief Structure for connected sensors status
typedef struct {
  uint8_t temp_rh_sensor_probe_status;    ///< Holds initialization and probing status of temp and RH sensor
  uint8_t imu_sensor_probe_status;        ///< Holds initialization and probing status of IMU sensor
  uint8_t gnss_receiver_probe_status;     ///< Holds initialization and probing status of GNSS receiver
  uint8_t temp_rh_sensor_retry_cnt;       ///< Holds retry count for sensor read failure of temp and RH sensor
  uint8_t imu_sensor_retry_cnt;           ///< Holds retry count for sensor read failure of IMU sensor
  uint8_t gnss_receiver_retry_cnt;        ///< Holds retry count for sensor read failure of GNSS receiver
} sl_wifi_asset_tracking_sensor_status_t;

/// @brief Structure for wi-fi asset tracking application status
typedef struct {
  bool app_shutdown;                                    ///< Holds status about application shutdown w.r.t LCD
  uint8_t wifi_conn_status;                             ///< Holds status about wi-fi connection
  uint8_t azure_cloud_conn_status;                      ///< Holds status about cloud connection
  bool lcd_init_status;                                 ///< Holds status about LCD initialization
  uint8_t recovery_progress_status;                     ///< Holds status about whether recovery is in progress or not
  sl_wifi_asset_tracking_sensor_status_t sensor_status; ///< Holds status about all the probed and initialized sensors
} sl_wifi_asset_tracking_status_t;

/// @brief Structure for LCD data queue object
typedef struct {
  uint8_t lcd_buffer[MAX_LCD_STRING_SIZE]; ///< LCD string buffer
} sl_wifi_asset_tracking_lcd_queue_data_t;

// -----------------------------------------------------------------------------
// Prototypes

/***************************************************************************/ /**
 * Function is entry point of wi-fi asset tracking example.
 * @return The following values are returned:
 * -  \ref SL_STATUS_OK on success
 * -  \ref SL_STATUS_FAIL - on sensor disable failed
 ******************************************************************************/
sl_status_t sl_start_wifi_asset_tracking_application();

/***************************************************************************/ /**
 * Create required tasks for wi-fi asset tracking example
 * @return The following values are returned:
 * -  \ref SL_STATUS_OK on success
 * -  \ref SL_STATUS_FAIL - on sensor disable failed
 ******************************************************************************/
sl_status_t sl_create_wifi_asset_tracking_tasks();

/***************************************************************************/ /**
 * Create required resources for wi-fi asset tracking example
 * @return The following values are returned:
 * -  \ref SL_STATUS_OK on success
 * -  \ref SL_STATUS_FAIL - on sensor disable failed
 ******************************************************************************/
sl_status_t sl_init_wifi_asset_tracking_resource();

/***************************************************************************/ /**
 * Create recovery task for wi-fi asset tracking example
 ******************************************************************************/
void sl_wifi_asset_tracking_recovery_task();

/***************************************************************************/ /**
 * De-initialize all the general resource of the application
 * @return The following values are returned:
 * -  \ref SL_STATUS_OK on success
 * -  \ref SL_STATUS_FAIL - on sensor disable failed
 ******************************************************************************/
sl_status_t sl_deinit_wifi_asset_tracking_resource();

/***************************************************************************/ /**
 * De-initialize the entire application
 ******************************************************************************/
void sl_stop_wifi_asset_tracking_application();

/***************************************************************************/ /**
 * Getter function for wi-fi asset tracking resource object
 * @return valid wi-fi asset tracking resource instance
 ******************************************************************************/
sl_wifi_asset_tracking_resource_t * sl_get_wifi_asset_tracking_resource();

/***************************************************************************/ /**
 * Getter function for wi-fi asset tracking status object
 * @return valid wi-fi asset tracking status instance
 ******************************************************************************/
sl_wifi_asset_tracking_status_t * sl_get_wifi_asset_tracking_status();

/***************************************************************************/ /**
 * Getter function for wi-fi asset tracking LCD string object
 * @return valid wi-fi asset tracking LCD string instance
 ******************************************************************************/
char ** sl_get_wifi_asset_tracking_lcd_string();

#ifdef __cplusplus
}
#endif

#endif /* SL_WIFI_ASSET_TRACKING_APP_H_ */

/******************************************************************************/
/* EOF                                                                        */
/******************************************************************************/

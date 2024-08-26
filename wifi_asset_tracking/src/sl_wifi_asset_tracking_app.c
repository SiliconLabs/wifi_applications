/***************************************************************************/ /**
 * @file sl_wifi_asset_tracking_app.c
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

#include <sl_wifi_asset_tracking_app.h>

/**
 * @brief strings printed on LCD for different status.
 */
char *sl_wifi_asset_tracking_lcd_strings[] = {
  "Wi-Fi asset tracking application started",
  "   ",
  "Application shutdown: None of sensors are connected",
  "Application shutdown: Wi-Fi is not connected",
  "Application shutdown: Azure IoT Hub is not connected",
  "SI7021: Not connected",
  "BMI270: Not connected",
  "MAX-M10S: Not connected",
  "SI7021: Reconnecting",
  "BMI270: Reconnecting",
  "MAX-M10S: Reconnecting",
  "SI7021: Connected",
  "BMI270: Connected",
  "MAX-M10S: Connected",
  "Wi-Fi: Connected",
  "Azure IoT Hub: Connected",
  "Wi-Fi: Connection retrying",
  "Azure IoT Hub: Connection retrying"
};

/**
 * @brief To store wi-fi asset tracking application status.
 */
sl_wifi_asset_tracking_status_t sl_wifi_asset_tracking_status;

/**
 * @brief To store resources required in wi-fi asset tracking example.
 */
sl_wifi_asset_tracking_resource_t sl_wifi_asset_tracking_resource;

/******************************************************************************
 *  Function is entry point of wi-fi asset tracking example.
 *****************************************************************************/
sl_status_t sl_start_wifi_asset_tracking_application()
{
  printf("\r\n*********************************************************\r\n");
  printf("\r\n********Starting wi-fi asset tracking application********\r\n");
  printf("\r\n*********************************************************\r\n");

  /// create require tasks for scheduler
  if (SL_STATUS_OK != sl_create_wifi_asset_tracking_tasks()) {
    printf(
      "\r\nsl_start_wifi_asset_tracking_application : failed to create wi-fi asset tracking tasks !!!\r\n");
    goto error;
  }

  /// initialize queues and respective mutex
  if (SL_STATUS_OK != sl_init_wifi_asset_tracking_resource()) {
    printf(
      "\r\nsl_start_wifi_asset_tracking_application : failed to create wi-fi asset tracking resources !!!\r\n");
    goto error;
  }

  /// By returning Success, main function will start scheduler internally
  return SL_STATUS_OK;

  error:

  sl_deinit_wifi_asset_tracking_resource();

  /// Delete the recovery task
  if (sl_wifi_asset_tracking_resource.task_list.recovery_task_handler != NULL) {
    vTaskDelete(sl_wifi_asset_tracking_resource.task_list.recovery_task_handler);
    sl_wifi_asset_tracking_resource.task_list.recovery_task_handler = NULL;
  }

  return SL_STATUS_FAIL;
}

/******************************************************************************
 *  Create require tasks for wi-fi asset tracking example
 *****************************************************************************/
sl_status_t sl_create_wifi_asset_tracking_tasks()
{
  /// Create GNSS receiver data capture task
  if (pdPASS != xTaskCreate(sl_capture_gnss_receiver_data_task,
                            NAME_GNSS_RECEIVER_TASK,
                            STACK_SIZE_GNSS_RECEIVER_TASK,
                            NULL,
                            PRIORITY_GNSS_RECEIVER_TASK,
                            &(sl_wifi_asset_tracking_resource.task_list.
                              gnss_receiver_task_handler))) {
    goto error;
  }

  /// Create IMU sensor data capture task
  if (pdPASS != xTaskCreate(sl_capture_imu_sensor_data_task,
                            NAME_IMU_SENSOR_TASK,
                            STACK_SIZE_IMU_SENSOR_TASK,
                            NULL,
                            PRIORITY_IMU_SENSOR_TASK,
                            &(sl_wifi_asset_tracking_resource.task_list.
                              imu_sensor_task_handler))) {
    goto error;
  }

  /// Create temperate and RH sensor data capture task
  if (pdPASS != xTaskCreate(sl_capture_temperature_rh_sensor_data_task,
                            NAME_TEMPERATURE_RH_SENSOR_TASK,
                            STACK_SIZE_TEMPERATURE_RH_SENSOR_TASK,
                            NULL,
                            PRIORITY_TEMPERATURE_RH_SENSOR_TASK,
                            &(sl_wifi_asset_tracking_resource.task_list.
                              temp_rh_sensor_task_handler))) {
    goto error;
  }

  /// Create Wi-Fi data capture task
  if (pdPASS != xTaskCreate(sl_capture_wifi_data_task,
                            NAME_WIFI_DATA_CAPTURE_TASK,
                            STACK_SIZE_WIFI_DATA_CAPTURE_TASK,
                            NULL,
                            PRIORITY_WIFI_DATA_CAPTURE_TASK,
                            &(sl_wifi_asset_tracking_resource.task_list.
                              wifi_data_capture_task_handler))) {
    goto error;
  }

  /// Create JSON data converter task
  if (pdPASS != xTaskCreate(sl_json_data_converter_task,
                            NAME_JSON_DATA_CONVERTER_TASK,
                            STACK_SIZE_JSON_DATA_CONVERTER_TASK,
                            NULL,
                            PRIORITY_JSON_DATA_CONVERTER_TASK,
                            &(sl_wifi_asset_tracking_resource.task_list.
                              json_data_converter_task_handler))) {
    goto error;
  }

  /// Create Azure cloud communication task
  if (pdPASS != xTaskCreate(sl_azure_cloud_communication_task,
                            NAME_CLOUD_COMMUNICATION_TASK,
                            STACK_SIZE_CLOUD_COMMUNICATION_TASK,
                            NULL,
                            PRIORITY_CLOUD_COMMUNICATION_TASK,
                            &(sl_wifi_asset_tracking_resource.task_list.
                              azure_cloud_communication_task_handler))) {
    goto error;
  }

  /// Create recovery task
  if (pdPASS != xTaskCreate(sl_wifi_asset_tracking_recovery_task,
                            NAME_RECOVERY_TASK,
                            STACK_SIZE_RECOVERY_TASK,
                            NULL,
                            PRIORITY_RECOVERY_TASK,
                            &(sl_wifi_asset_tracking_resource.task_list.
                              recovery_task_handler))) {
    goto error;
  }

  /// Create LCD task
  if (pdPASS != xTaskCreate(sl_wifi_asset_tracking_lcd_task,
                            NAME_LCD_TASK,
                            STACK_SIZE_LCD_TASK,
                            NULL,
                            PRIORITY_LCD_TASK,
                            &(sl_wifi_asset_tracking_resource.task_list.
                              lcd_task_handler))) {
    goto error;
  }

  return SL_STATUS_OK;
  error:
  return SL_STATUS_FAIL;
}

/******************************************************************************
 *  Create required resources for wi-fi asset tracking example
 *****************************************************************************/
sl_status_t sl_init_wifi_asset_tracking_resource()
{
  /// set default status of all sensors, wi-fi and cloud
  sl_wifi_asset_tracking_status.sensor_status.temp_rh_sensor_probe_status =
    SL_SENSOR_NOT_PROBED;
  sl_wifi_asset_tracking_status.sensor_status.imu_sensor_probe_status =
    SL_SENSOR_NOT_PROBED;
  sl_wifi_asset_tracking_status.sensor_status.gnss_receiver_probe_status =
    SL_SENSOR_NOT_PROBED;
  sl_wifi_asset_tracking_status.wifi_conn_status = SL_WIFI_NOT_CONNECTED;
  sl_wifi_asset_tracking_status.azure_cloud_conn_status =
    SL_CLOUD_NOT_CONNECTED;
  sl_wifi_asset_tracking_status.recovery_progress_status = SL_RECOVERY_IDLE;
  sl_wifi_asset_tracking_status.app_shutdown = false;
  sl_wifi_asset_tracking_status.lcd_init_status = true;

  /// reset retry count
  sl_wifi_asset_tracking_status.sensor_status.temp_rh_sensor_retry_cnt = 0;
  sl_wifi_asset_tracking_status.sensor_status.imu_sensor_retry_cnt = 0;
  sl_wifi_asset_tracking_status.sensor_status.gnss_receiver_retry_cnt = 0;

  /// Create sensor data queue resource
  sl_wifi_asset_tracking_resource.sensor_data_queue_handler =
    xQueueCreate(MAX_SIZE_OF_SENSOR_DATA_QUEUE,
                 sizeof(sl_wifi_asset_tracking_sensor_queue_data_t));

  if (NULL == sl_wifi_asset_tracking_resource.sensor_data_queue_handler) {
    goto error;
  }

  /// Create sensor data queue mutex
  sl_wifi_asset_tracking_resource.sensor_data_queue_mutex_handler =
    (QueueHandle_t)xSemaphoreCreateMutex();

  if (NULL == sl_wifi_asset_tracking_resource.sensor_data_queue_mutex_handler) {
    goto error;
  }

  /// Create MQTT package data queue resource
  sl_wifi_asset_tracking_resource.mqtt_package_queue_handler =
    xQueueCreate(MAX_SIZE_OF_MQTT_PACKAGE_QUEUE,
                 sizeof(sl_wifi_asset_tracking_mqtt_package_queue_data_t));

  if (NULL == sl_wifi_asset_tracking_resource.mqtt_package_queue_handler) {
    goto error;
  }

  /// Create MQTT package data queue mutex
  sl_wifi_asset_tracking_resource.mqtt_package_queue_mutex_handler =
    (QueueHandle_t)xSemaphoreCreateMutex();

  if (NULL
      == sl_wifi_asset_tracking_resource.mqtt_package_queue_mutex_handler) {
    goto error;
  }

  /// Create LCD data queue resource
  sl_wifi_asset_tracking_resource.lcd_queue_handler =
    xQueueCreate(MAX_SIZE_OF_LCD_DATA_QUEUE,
                 sizeof(sl_wifi_asset_tracking_lcd_queue_data_t));

  if (NULL == sl_wifi_asset_tracking_resource.lcd_queue_handler) {
    goto error;
  }

  /// Create I2C call mutex
  sl_wifi_asset_tracking_resource.i2c_mutex_handler =
    (SemaphoreHandle_t)xSemaphoreCreateBinary();

  if (NULL == sl_wifi_asset_tracking_resource.i2c_mutex_handler) {
    goto error;
  }

  /// Need to give it as it is by default taken
  xSemaphoreGive(sl_wifi_asset_tracking_resource.i2c_mutex_handler);

  /// Create recovery status mutex
  sl_wifi_asset_tracking_resource.recovery_status_mutex_handler =
    (QueueHandle_t)xSemaphoreCreateMutex();

  if (NULL == sl_wifi_asset_tracking_resource.recovery_status_mutex_handler) {
    goto error;
  }

  /// Create timer for temperature_rh_sensor task
  sl_wifi_asset_tracking_resource.temperature_rh_sensor_timer = xTimerCreate(
    NAME_TEMPERATURE_RH_SENSOR_TIMER,
    (
      pdMS_TO_TICKS(
        PERIOD_OF_TEMPERATURE_RH_SENSOR_TIMER) * TIMER_CLOCK_OFFSET),
    pdFALSE,
    NULL,
    on_temperature_rh_sensor_timer_callback);

  if (NULL == sl_wifi_asset_tracking_resource.temperature_rh_sensor_timer) {
    goto error;
  }

  /// Create timer for IMU sensor task
  sl_wifi_asset_tracking_resource.imu_sensor_timer = xTimerCreate(
    NAME_IMU_SENSOR_TIMER,
    (pdMS_TO_TICKS(
       PERIOD_OF_IMU_SENSOR_TIMER) * TIMER_CLOCK_OFFSET),
    pdFALSE,
    NULL,
    on_imu_sensor_timer_callback);

  if (NULL == sl_wifi_asset_tracking_resource.imu_sensor_timer) {
    goto error;
  }

  /// Create timer for GNSS sensor task
  sl_wifi_asset_tracking_resource.gnss_sensor_timer = xTimerCreate(
    NAME_GNSS_RECEIVER_TIMER,
    (
      pdMS_TO_TICKS(
        PERIOD_OF_GNSS_RECEIVER_TIMER) * TIMER_CLOCK_OFFSET),
    pdFALSE,
    NULL,
    on_gnss_sensor_timer_callback);

  if (NULL == sl_wifi_asset_tracking_resource.gnss_sensor_timer) {
    goto error;
  }

  return SL_STATUS_OK;
  error:
  return SL_STATUS_FAIL;
}

/******************************************************************************
 *  Create recovery task for wi-fi asset tracking example
 *****************************************************************************/
void sl_wifi_asset_tracking_recovery_task()
{
  while (1) {
    /// Sensor related recovery - start

    /// When all sensor are not probed simply suspend the recovery task as this happens when application is just started.
    /// This condition is true only once in whole application life cycle.
    if ((SL_SENSOR_NOT_PROBED
         == sl_wifi_asset_tracking_status.sensor_status.
         temp_rh_sensor_probe_status)
        && (SL_SENSOR_NOT_PROBED
            == sl_wifi_asset_tracking_status.sensor_status.
            imu_sensor_probe_status)
        && (SL_SENSOR_NOT_PROBED
            == sl_wifi_asset_tracking_status.sensor_status.
            gnss_receiver_probe_status)) {
      printf(
        "\r\nrecovery_task : suspend recovery task as all sensors are not in probed state\r\n");
      vTaskSuspend(
        sl_wifi_asset_tracking_resource.task_list.recovery_task_handler);
      continue;
    }

    /// When probing of all the sensor is failed
    if ((SL_SENSOR_SHUTDOWN
         == sl_wifi_asset_tracking_status.sensor_status.
         temp_rh_sensor_probe_status)
        && (SL_SENSOR_SHUTDOWN
            == sl_wifi_asset_tracking_status.sensor_status.
            imu_sensor_probe_status)
        && (SL_SENSOR_SHUTDOWN
            == sl_wifi_asset_tracking_status.sensor_status.
            gnss_receiver_probe_status)) {
      printf(
        "\r\nrecovery_task : None of sensors got probed/connected, so closing application.\r\n");

      sl_wifi_asset_tracking_lcd_print(INDEX_SENSOR_SHUTDOWN);

      /// gracefully shutdown the application
      sl_stop_wifi_asset_tracking_application();
      break;
    }

    /// When probing of si7021 sensor is failed
    if (SL_SENSOR_PROBE_FAILED
        == sl_wifi_asset_tracking_status.sensor_status.
        temp_rh_sensor_probe_status) {
      printf(
        "\r\nrecovery_task : temperature and RH sensor is not probed/disconnected, so deleting temperature and RH sensor task.\r\n");

      sl_wifi_asset_tracking_lcd_print(INDEX_SI7021_NOT_CONNECTED);

      vTaskDelete(
        sl_wifi_asset_tracking_resource.task_list.temp_rh_sensor_task_handler);
      sl_wifi_asset_tracking_resource.task_list.temp_rh_sensor_task_handler =
        NULL;
      sl_wifi_asset_tracking_status.sensor_status.temp_rh_sensor_probe_status =
        SL_SENSOR_SHUTDOWN;
      continue;
    }

    /// When si7021 sensor is disconnected in-between running application
    if (SL_SENSOR_DISCONNECTED
        == sl_wifi_asset_tracking_status.sensor_status.
        temp_rh_sensor_probe_status) {
      printf(
        "\r\nrecovery_task : Recreating temperature and RH sensor task as temperature and RH sensor got disconnected\r\n");

      vTaskDelete(
        sl_get_wifi_asset_tracking_resource()->task_list.temp_rh_sensor_task_handler);
      sl_wifi_asset_tracking_resource.task_list.temp_rh_sensor_task_handler =
        NULL;

      /// Recreate si7021 sensor data capture task
      if (pdPASS != xTaskCreate(sl_capture_temperature_rh_sensor_data_task,
                                NAME_TEMPERATURE_RH_SENSOR_TASK,
                                STACK_SIZE_TEMPERATURE_RH_SENSOR_TASK,
                                NULL,
                                PRIORITY_TEMPERATURE_RH_SENSOR_TASK,
                                &(sl_wifi_asset_tracking_resource.task_list.
                                  temp_rh_sensor_task_handler))) {
        sl_wifi_asset_tracking_status.sensor_status.temp_rh_sensor_retry_cnt =
          0;
        sl_wifi_asset_tracking_status.sensor_status.temp_rh_sensor_probe_status
          = SL_SENSOR_SHUTDOWN;

        vTaskSuspend(
          sl_wifi_asset_tracking_resource.task_list.recovery_task_handler);
        continue;
      }

      ++sl_wifi_asset_tracking_status.sensor_status.temp_rh_sensor_retry_cnt;
      sl_wifi_asset_tracking_lcd_print(INDEX_SI7021_RECONNECTING);

      if (SENSOR_MAX_RETRY_COUNT
          == sl_wifi_asset_tracking_status.sensor_status.
          temp_rh_sensor_retry_cnt) {
        sl_wifi_asset_tracking_status.sensor_status.temp_rh_sensor_retry_cnt =
          0;
        sl_wifi_asset_tracking_status.sensor_status.temp_rh_sensor_probe_status
          = SL_SENSOR_PROBE_FAILED;
        continue;
      } else {
        sl_wifi_asset_tracking_status.sensor_status.temp_rh_sensor_probe_status
          = SL_SENSOR_RECONNECTED;
      }

      vTaskSuspend(
        sl_wifi_asset_tracking_resource.task_list.recovery_task_handler);
      continue;
    }

    /// When probing of bmi270 sensor is failed
    if (SL_SENSOR_PROBE_FAILED
        == sl_wifi_asset_tracking_status.sensor_status.imu_sensor_probe_status)
    {
      printf(
        "\r\nrecovery_task : IMU sensor is not probed/disconnected, so deleting IMU sensor task.\r\n");

      sl_wifi_asset_tracking_lcd_print(INDEX_BMI270_NOT_CONNECTED);

      vTaskDelete(
        sl_wifi_asset_tracking_resource.task_list.imu_sensor_task_handler);
      sl_wifi_asset_tracking_resource.task_list.imu_sensor_task_handler = NULL;
      sl_wifi_asset_tracking_status.sensor_status.imu_sensor_probe_status =
        SL_SENSOR_SHUTDOWN;

      continue;
    }

    /// When bmi270 sensor is disconnected in-between running application
    if (SL_SENSOR_DISCONNECTED
        == sl_wifi_asset_tracking_status.sensor_status.imu_sensor_probe_status)
    {
      printf(
        "\r\nrecovery_task : Recreating IMU sensor task as IMU sensor got disconnected\r\n");

      vTaskDelete(
        sl_get_wifi_asset_tracking_resource()->task_list.imu_sensor_task_handler);
      sl_wifi_asset_tracking_resource.task_list.imu_sensor_task_handler = NULL;

      /// Recreate bmi270 sensor data capture task
      if (pdPASS != xTaskCreate(sl_capture_imu_sensor_data_task,
                                NAME_IMU_SENSOR_TASK,
                                STACK_SIZE_IMU_SENSOR_TASK,
                                NULL,
                                PRIORITY_IMU_SENSOR_TASK,
                                &(sl_wifi_asset_tracking_resource.task_list.
                                  imu_sensor_task_handler))) {
        sl_wifi_asset_tracking_status.sensor_status.imu_sensor_retry_cnt = 0;
        sl_wifi_asset_tracking_status.sensor_status.imu_sensor_probe_status =
          SL_SENSOR_SHUTDOWN;

        vTaskSuspend(
          sl_wifi_asset_tracking_resource.task_list.recovery_task_handler);
        continue;
      }

      ++sl_wifi_asset_tracking_status.sensor_status.imu_sensor_retry_cnt;
      sl_wifi_asset_tracking_lcd_print(INDEX_BMI270_RECONNECTING);

      if (SENSOR_MAX_RETRY_COUNT
          == sl_wifi_asset_tracking_status.sensor_status.imu_sensor_retry_cnt) {
        sl_wifi_asset_tracking_status.sensor_status.imu_sensor_retry_cnt = 0;
        sl_wifi_asset_tracking_status.sensor_status.imu_sensor_probe_status =
          SL_SENSOR_PROBE_FAILED;
        continue;
      } else {
        sl_wifi_asset_tracking_status.sensor_status.imu_sensor_probe_status =
          SL_SENSOR_RECONNECTED;
      }

      vTaskSuspend(
        sl_wifi_asset_tracking_resource.task_list.recovery_task_handler);
      continue;
    }

    /// When probing of gnss receiver is failed
    if (SL_SENSOR_PROBE_FAILED
        == sl_wifi_asset_tracking_status.sensor_status.
        gnss_receiver_probe_status) {
      printf(
        "\r\nrecovery_task : GNSS receiver is not probed/disconnected, so deleting GNSS receiver task.\r\n");

      sl_wifi_asset_tracking_lcd_print(INDEX_MAX_M10S_NOT_CONNECTED);

      vTaskDelete(
        sl_wifi_asset_tracking_resource.task_list.gnss_receiver_task_handler);
      sl_wifi_asset_tracking_resource.task_list.gnss_receiver_task_handler =
        NULL;
      sl_wifi_asset_tracking_status.sensor_status.gnss_receiver_probe_status =
        SL_SENSOR_SHUTDOWN;

      continue;
    }

    /// When gnss receiver sensor is disconnected in-between running application
    if (SL_SENSOR_DISCONNECTED
        == sl_wifi_asset_tracking_status.sensor_status.
        gnss_receiver_probe_status) {
      printf(
        "\r\nrecovery_task : Recreating GNSS receiver task as GNSS receiver got disconnected\r\n");

      vTaskDelete(
        sl_get_wifi_asset_tracking_resource()->task_list.gnss_receiver_task_handler);
      sl_wifi_asset_tracking_resource.task_list.gnss_receiver_task_handler =
        NULL;

      /// Recreate GNSS receiver data capture task
      if (pdPASS != xTaskCreate(sl_capture_gnss_receiver_data_task,
                                NAME_GNSS_RECEIVER_TASK,
                                STACK_SIZE_GNSS_RECEIVER_TASK,
                                NULL,
                                PRIORITY_GNSS_RECEIVER_TASK,
                                &(sl_wifi_asset_tracking_resource.task_list.
                                  gnss_receiver_task_handler))) {
        sl_wifi_asset_tracking_status.sensor_status.gnss_receiver_retry_cnt = 0;
        sl_wifi_asset_tracking_status.sensor_status.gnss_receiver_probe_status =
          SL_SENSOR_SHUTDOWN;

        vTaskSuspend(
          sl_wifi_asset_tracking_resource.task_list.recovery_task_handler);
        continue;
      }

      ++sl_wifi_asset_tracking_status.sensor_status.gnss_receiver_retry_cnt;
      sl_wifi_asset_tracking_lcd_print(INDEX_MAX_M10S_RECONNECTING);

      if (SENSOR_MAX_RETRY_COUNT
          == sl_wifi_asset_tracking_status.sensor_status.gnss_receiver_retry_cnt)
      {
        sl_wifi_asset_tracking_status.sensor_status.gnss_receiver_retry_cnt = 0;
        sl_wifi_asset_tracking_status.sensor_status.gnss_receiver_probe_status =
          SL_SENSOR_PROBE_FAILED;
        continue;
      } else {
        sl_wifi_asset_tracking_status.sensor_status.gnss_receiver_probe_status =
          SL_SENSOR_RECONNECTED;
      }

      vTaskSuspend(
        sl_wifi_asset_tracking_resource.task_list.recovery_task_handler);
      continue;
    }

    /// Sensor related recovery - end

    /// Wi-Fi related recovery - start

    /// Initialization of Wi-Fi connection is failed
    if (SL_WIFI_SHUTDOWN == sl_wifi_asset_tracking_status.wifi_conn_status) {
      printf(
        "\r\nrecovery_task : deleting wi-fi capture task and closing application due to wi-fi connection failure.\r\n");

      sl_wifi_asset_tracking_lcd_print(INDEX_WIFI_SHUTDOWN);

      vTaskDelete(
        sl_wifi_asset_tracking_resource.task_list.wifi_data_capture_task_handler);
      sl_wifi_asset_tracking_resource.task_list.wifi_data_capture_task_handler =
        NULL;

      /// gracefully shutdown the application
      sl_stop_wifi_asset_tracking_application();
      break;
    }

    /// When Wi-Fi connection is disconnected in-between running application
    if (SL_WIFI_DISCONNECTED
        == sl_wifi_asset_tracking_status.wifi_conn_status) {
      printf("\r\nrecovery_task : retrying wi-fi connection.\r\n");

      sl_wifi_asset_tracking_status.azure_cloud_conn_status =
        SL_CLOUD_DISCONNECTED;

      /// Retry to connect WiFi connection before azure cloud communication
      if (SL_STATUS_OK == sl_retry_wifi_connection(false)) {
        sl_wifi_asset_tracking_status.wifi_conn_status = SL_WIFI_CONNECTED;

        sl_wifi_asset_tracking_lcd_print(INDEX_WIFI_CONNECTED);

        /// Retry to connect azure cloud communication
        if (SL_STATUS_OK == sl_retry_azure_cloud_connection()) {
          sl_wifi_asset_tracking_status.azure_cloud_conn_status =
            SL_CLOUD_CONNECTED;

          sl_wifi_asset_tracking_lcd_print(INDEX_AZURE_CLOUD_CONNECTED);

          /// Acquire semaphore to update the status of recovery task
          if (pdTRUE
              == (xSemaphoreTake(sl_wifi_asset_tracking_resource.
                                 recovery_status_mutex_handler,
                                 portMAX_DELAY))) {
            sl_wifi_asset_tracking_status.recovery_progress_status =
              SL_RECOVERY_IDLE;
            xSemaphoreGive(
              sl_wifi_asset_tracking_resource.recovery_status_mutex_handler);
          }

          vTaskSuspend(
            sl_wifi_asset_tracking_resource.task_list.recovery_task_handler);
          continue;
        } else {
          printf(
            "\r\nrecovery_task : closing application due to Azure IoT Hub connection failure.\r\n");

          sl_wifi_asset_tracking_lcd_print(INDEX_CLOUD_SHUTDOWN);

          sl_stop_wifi_asset_tracking_application();
          break;
        }
      } else {
        printf(
          "\r\nrecovery_task : closing application due to wi-fi connection failure\r\n");

        sl_wifi_asset_tracking_lcd_print(INDEX_WIFI_SHUTDOWN);

        sl_stop_wifi_asset_tracking_application();
        break;
      }
    }

    /// Wi-Fi related recovery - end

    /// Azure cloud related recovery - start

    /// Initialization of azure cloud connection is failed
    if (SL_CLOUD_SHUTDOWN
        == sl_wifi_asset_tracking_status.azure_cloud_conn_status) {
      printf(
        "\r\nrecovery_task : deleting Azure communication task and closing application due to Azure IoT Hub connection failure.\r\n");

      sl_wifi_asset_tracking_lcd_print(INDEX_CLOUD_SHUTDOWN);

      vTaskDelete(
        sl_wifi_asset_tracking_resource.task_list.azure_cloud_communication_task_handler);
      sl_wifi_asset_tracking_resource.task_list.
      azure_cloud_communication_task_handler = NULL;

      /// gracefully shutdown the application
      sl_stop_wifi_asset_tracking_application();
      break;
    }

    /// When cloud connection is disconnected in-between running application
    if (SL_CLOUD_DISCONNECTED
        == sl_wifi_asset_tracking_status.azure_cloud_conn_status) {
      int32_t rssi = 0;
      sl_status_t status;

      printf("\r\nrecovery_task : retrying Azure IoT Hub connection.\r\n");

      /// Check whether rssi is getting
      status = sl_get_wifi_rssi(&rssi);

      if (SL_STATUS_OK != status) {
        printf(
          "\r\nrecovery_task : wi-fi also got disconnected, trigger wi-fi recovery along with Azure IoT Hub recovery.\r\n");
        sl_wifi_asset_tracking_status.wifi_conn_status = SL_WIFI_DISCONNECTED;
        continue;
      }

      /// Retry to connect azure cloud communication
      if (SL_STATUS_OK == sl_retry_azure_cloud_connection()) {
        sl_wifi_asset_tracking_status.azure_cloud_conn_status =
          SL_CLOUD_CONNECTED;

        sl_wifi_asset_tracking_lcd_print(INDEX_AZURE_CLOUD_CONNECTED);

        /// Acquire semaphore to update the status of recovery task
        if (pdTRUE
            == (xSemaphoreTake(sl_wifi_asset_tracking_resource.
                               recovery_status_mutex_handler,
                               portMAX_DELAY))) {
          sl_wifi_asset_tracking_status.recovery_progress_status =
            SL_RECOVERY_IDLE;
          xSemaphoreGive(
            sl_wifi_asset_tracking_resource.recovery_status_mutex_handler);
        }

        vTaskSuspend(
          sl_wifi_asset_tracking_resource.task_list.recovery_task_handler);
        continue;
      } else {
        if (SL_WIFI_DISCONNECTED
            == sl_wifi_asset_tracking_status.wifi_conn_status) {
          printf(
            "\r\nrecovery_task : closing application due to wi-fi connection failure.\r\n");

          sl_wifi_asset_tracking_lcd_print(INDEX_WIFI_SHUTDOWN);
        } else {
          printf(
            "\r\nrecovery_task : closing application due to Azure IoT Hub connection failure.\r\n");

          sl_wifi_asset_tracking_lcd_print(INDEX_CLOUD_SHUTDOWN);
        }
        sl_stop_wifi_asset_tracking_application();
        break;
      }
    }

    /// Azure cloud related recovery - end

    /// Suspend recovery task - if nothing to do
    vTaskSuspend(sl_wifi_asset_tracking_resource.task_list.recovery_task_handler);
  }

  printf(
    "\r\nrecovery_task : wi-fi asset tracking application is gracefully shutdown\r\n");

  /// Delete the recovery task
  if (sl_wifi_asset_tracking_resource.task_list.recovery_task_handler != NULL) {
    vTaskDelete(sl_wifi_asset_tracking_resource.task_list.recovery_task_handler);
    sl_wifi_asset_tracking_resource.task_list.recovery_task_handler = NULL;
  }
}

/******************************************************************************
 *  Function for de-initialize the general resource
 *****************************************************************************/
sl_status_t sl_deinit_wifi_asset_tracking_resource()
{
  sl_status_t status = SL_STATUS_OK;

  /// Delete the temperature_rh_sensor timer
  if (sl_wifi_asset_tracking_resource.temperature_rh_sensor_timer != NULL) {
    xTimerDelete(sl_wifi_asset_tracking_resource.temperature_rh_sensor_timer,
                 0);
    sl_wifi_asset_tracking_resource.temperature_rh_sensor_timer = NULL;
  }

  /// Delete the imu_sensor timer
  if (sl_wifi_asset_tracking_resource.imu_sensor_timer != NULL) {
    xTimerDelete(sl_wifi_asset_tracking_resource.imu_sensor_timer, 0);
    sl_wifi_asset_tracking_resource.imu_sensor_timer = NULL;
  }

  /// Delete the gnss_sensor timer
  if (sl_wifi_asset_tracking_resource.gnss_sensor_timer != NULL) {
    xTimerDelete(sl_wifi_asset_tracking_resource.gnss_sensor_timer, 0);
    sl_wifi_asset_tracking_resource.gnss_sensor_timer = NULL;
  }

  /// Delete the sensor data queue
  if (sl_wifi_asset_tracking_resource.sensor_data_queue_handler != NULL) {
    vQueueDelete(sl_wifi_asset_tracking_resource.sensor_data_queue_handler);
    sl_wifi_asset_tracking_resource.sensor_data_queue_handler = NULL;
  }

  /// Delete the sensor data queue mutex
  if (sl_wifi_asset_tracking_resource.sensor_data_queue_mutex_handler != NULL) {
    vSemaphoreDelete(
      sl_wifi_asset_tracking_resource.sensor_data_queue_mutex_handler);
    sl_wifi_asset_tracking_resource.sensor_data_queue_mutex_handler = NULL;
  }

  /// Delete the MQTT package data queue
  if (sl_wifi_asset_tracking_resource.mqtt_package_queue_handler != NULL) {
    vQueueDelete(sl_wifi_asset_tracking_resource.mqtt_package_queue_handler);
    sl_wifi_asset_tracking_resource.mqtt_package_queue_handler = NULL;
  }

  /// Delete the LCD data queue
  if (sl_wifi_asset_tracking_resource.lcd_queue_handler != NULL) {
    vQueueDelete(sl_wifi_asset_tracking_resource.lcd_queue_handler);
    sl_wifi_asset_tracking_resource.lcd_queue_handler = NULL;
  }

  /// Delete the MQTT package data queue mutex
  if (sl_wifi_asset_tracking_resource.mqtt_package_queue_mutex_handler
      != NULL) {
    vSemaphoreDelete(
      sl_wifi_asset_tracking_resource.mqtt_package_queue_mutex_handler);
    sl_wifi_asset_tracking_resource.mqtt_package_queue_mutex_handler = NULL;
  }

  /// Delete I2C handler mutex
  if (sl_wifi_asset_tracking_resource.i2c_mutex_handler != NULL) {
    vSemaphoreDelete(sl_wifi_asset_tracking_resource.i2c_mutex_handler);
    sl_wifi_asset_tracking_resource.i2c_mutex_handler = NULL;
  }

  /// Delete recovery status mutex
  if (sl_wifi_asset_tracking_resource.recovery_status_mutex_handler != NULL) {
    vSemaphoreDelete(
      sl_wifi_asset_tracking_resource.recovery_status_mutex_handler);
    sl_wifi_asset_tracking_resource.recovery_status_mutex_handler = NULL;
  }

  /// Delete the temperature and RH sensor data capture task
  if (sl_wifi_asset_tracking_resource.task_list.temp_rh_sensor_task_handler
      != NULL) {
    vTaskDelete(
      sl_wifi_asset_tracking_resource.task_list.temp_rh_sensor_task_handler);
    sl_wifi_asset_tracking_resource.task_list.temp_rh_sensor_task_handler =
      NULL;
  }

  /// Delete the IMU sensor data capture task
  if (sl_wifi_asset_tracking_resource.task_list.imu_sensor_task_handler
      != NULL) {
    vTaskDelete(
      sl_wifi_asset_tracking_resource.task_list.imu_sensor_task_handler);
    sl_wifi_asset_tracking_resource.task_list.imu_sensor_task_handler = NULL;
  }

  /// Delete the GNSS receiver data capture task
  if (sl_wifi_asset_tracking_resource.task_list.gnss_receiver_task_handler
      != NULL) {
    vTaskDelete(
      sl_wifi_asset_tracking_resource.task_list.gnss_receiver_task_handler);
    sl_wifi_asset_tracking_resource.task_list.gnss_receiver_task_handler = NULL;
  }

  /// Delete the Wi-Fi data capture task
  if (sl_wifi_asset_tracking_resource.task_list.wifi_data_capture_task_handler
      != NULL) {
    vTaskDelete(
      sl_wifi_asset_tracking_resource.task_list.wifi_data_capture_task_handler);
    sl_wifi_asset_tracking_resource.task_list.wifi_data_capture_task_handler =
      NULL;
  }

  /// Delete the JSON data converter task
  if (sl_wifi_asset_tracking_resource.task_list.json_data_converter_task_handler
      != NULL) {
    vTaskDelete(
      sl_wifi_asset_tracking_resource.task_list.json_data_converter_task_handler);
    sl_wifi_asset_tracking_resource.task_list.json_data_converter_task_handler =
      NULL;
  }

  /// Delete the Azure cloud communication task
  if (sl_wifi_asset_tracking_resource.task_list.
      azure_cloud_communication_task_handler != NULL) {
    vTaskDelete(
      sl_wifi_asset_tracking_resource.task_list.azure_cloud_communication_task_handler);
    sl_wifi_asset_tracking_resource.task_list.
    azure_cloud_communication_task_handler = NULL;
  }

  /// Delete the LCD task
  if ((sl_get_wifi_asset_tracking_status()->lcd_init_status)
      && (sl_wifi_asset_tracking_resource.task_list.lcd_task_handler != NULL)) {
    vTaskDelete(sl_wifi_asset_tracking_resource.task_list.lcd_task_handler);
    sl_wifi_asset_tracking_resource.task_list.lcd_task_handler = NULL;
  }

  /// De-initialize I2C bus
  status = sl_i2c_driver_deinit(I2C);

  return status;
}

/******************************************************************************
 *  Function for de-initialize the entire application
 *****************************************************************************/
void sl_stop_wifi_asset_tracking_application()
{
  sl_disconnect_azure_iot_hub();

  sl_deinit_wifi_connection();

  sl_wifi_asset_tracking_sensors_deinit();

  sl_deinit_wifi_asset_tracking_resource();
}

/******************************************************************************
 *  Getter function for wi-fi asset tracking resource object
 *****************************************************************************/
sl_wifi_asset_tracking_resource_t * sl_get_wifi_asset_tracking_resource()
{
  return &sl_wifi_asset_tracking_resource;
}

/******************************************************************************
 *  Getter function for wi-fi asset tracking status object
 *****************************************************************************/
sl_wifi_asset_tracking_status_t * sl_get_wifi_asset_tracking_status()
{
  return &sl_wifi_asset_tracking_status;
}

/******************************************************************************
 *  Getter function for wi-fi asset tracking LCD string object
 *****************************************************************************/
char ** sl_get_wifi_asset_tracking_lcd_string()
{
  return sl_wifi_asset_tracking_lcd_strings;
}

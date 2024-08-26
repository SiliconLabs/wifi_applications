/***************************************************************************/ /**
 * @file sl_wifi_asset_tracking_sensor.c
 * @brief sensor related functions
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

#include <sl_wifi_asset_tracking_sensor.h>
#include <sl_wifi_asset_tracking_app.h>
#include <sl_wifi_asset_tracking_demo_config.h>
#include "sparkfun_bmi270.h"
#include "gnss_max_m10s_driver.h"

#define I2C_INSTANCE_USED            SL_I2C2
static sl_i2c_instance_t i2c_instance = I2C_INSTANCE_USED;
static mikroe_i2c_handle_t app_i2c_instance = &i2c_instance;

static sl_max_m10s_cfg_data_t gnss_cfg_data; ///< To store GNSS receiver configuration.
static bmi270_cfg_data_t bmi_cfg_data; ///< To store BMI270 configuration.

/******************************************************************************
 * De-initialize required sensor modules.
 *****************************************************************************/
sl_status_t sl_wifi_asset_tracking_sensors_deinit()
{
  sl_status_t status = SL_STATUS_OK;

  /// De-initialize GNSS receiver module
  status = gnss_max_m10s_deinit(&gnss_cfg_data);

  return status;
}

/******************************************************************************
 *  Probe and initialize Si7021 temperature and RH sensor.
 *****************************************************************************/
sl_status_t sl_init_si7021_temperature_and_rh_sensor()
{
  sl_status_t status;

  /// Initializes sensor and reads electronic ID 1st byte
  xTimerStart(
    sl_get_wifi_asset_tracking_resource()->temperature_rh_sensor_timer,
    0);
  status = sl_si91x_si70xx_init(I2C, SI7021_ADDR, SL_EID_FIRST_BYTE);
  xTimerStop(sl_get_wifi_asset_tracking_resource()->temperature_rh_sensor_timer,
             0);

  if (status != SL_STATUS_OK) {
    return SL_STATUS_FAIL;
  }

  /// Initializes sensor and reads electronic ID 2nd byte
  xTimerStart(
    sl_get_wifi_asset_tracking_resource()->temperature_rh_sensor_timer,
    0);
  status = sl_si91x_si70xx_init(I2C, SI7021_ADDR, SL_EID_SECOND_BYTE);
  xTimerStop(sl_get_wifi_asset_tracking_resource()->temperature_rh_sensor_timer,
             0);

  if (status != SL_STATUS_OK) {
    return SL_STATUS_FAIL;
  }

  printf(
    "\r\nsl_init_si7021_temperature_and_rh_sensor : Successfully initialized Si7021 temperature and RH sensor\n");
  return SL_STATUS_OK;
}

/******************************************************************************
 *  Probe and initialize bmi270 IMU sensor.
 *****************************************************************************/
sl_status_t sl_init_bmi270_imu_sensor()
{
  sl_status_t status = SL_STATUS_OK;

  /// Initializing I2C instance and slave address
  bmi_cfg_data.i2c_instance = app_i2c_instance;
  bmi_cfg_data.bmi270_slave_address = BMI270_ADDR;

  /// Initializing IMU sensor configuration

  /// append ORing of all the needed features of bmi270
  bmi_cfg_data.enable_features.features = BMI270_FEATURE_ACC
                                          | BMI270_FEATURE_GYRO;

  /// append the various configuration of accelerometer
  bmi_cfg_data.acc_config.bw = BMI270_ACCEL_NORM_AVG4;
  bmi_cfg_data.acc_config.odr = BMI270_ACCEL_ODR_100HZ;
  bmi_cfg_data.acc_config.filter = BMI270_ACCEL_PERFO_OPT;
  bmi_cfg_data.acc_config.range = BMI270_ACCEL_RANGE_2G;

  /// append the various configuration of gyroscope
  bmi_cfg_data.gyro_config.bw = BMI270_GYRO_BWP_NORMAL;
  bmi_cfg_data.gyro_config.odr = BMI270_GYRO_ODR_200HZ;
  bmi_cfg_data.gyro_config.filter = BMI270_GYRO_F_PERFO_OPT;
  bmi_cfg_data.gyro_config.range = BMI270_GYRO_RANGE_2000DPS;
  bmi_cfg_data.gyro_config.noise = BMI270_GYRO_N_POWER_OPT;

  /// Initialize BMI270 sensor
  xTimerStart(sl_get_wifi_asset_tracking_resource()->imu_sensor_timer, 0);
  status = sparkfun_bmi270_init(&bmi_cfg_data);
  xTimerStop(sl_get_wifi_asset_tracking_resource()->imu_sensor_timer, 0);

  if (SL_STATUS_OK != status) {
    return SL_STATUS_FAIL;
  }

  /// Enable and configure features of BMI270 sensor
  xTimerStart(sl_get_wifi_asset_tracking_resource()->imu_sensor_timer, 0);
  status |= sparkfun_bmi270_enable_and_config_features(&bmi_cfg_data);
  xTimerStop(sl_get_wifi_asset_tracking_resource()->imu_sensor_timer, 0);

  if (SL_STATUS_OK != status) {
    return SL_STATUS_FAIL;
  }

  printf(
    "\r\nsl_init_bmi270_imu_sensor : Successfully initialized BMI270 sensor\n");

  return SL_STATUS_OK;
}

/******************************************************************************
 *  Probe and initialize MAX-M10s GNSS receiver.
 *****************************************************************************/
sl_status_t sl_init_max_m10s_gnss_receiver()
{
  sl_status_t status = SL_STATUS_OK;
  gnss_cfg_data.i2c_instance = app_i2c_instance;
  gnss_cfg_data.device_address = GNSS_ADDRESS;
  gnss_cfg_data.protocol_type = SL_MAX_M10S_PROTOCOL_UBX;

  xTimerStart(sl_get_wifi_asset_tracking_resource()->gnss_sensor_timer, 0);
  status = gnss_max_m10s_begin(GNSS_POLL_MAX_TIMEOUT, &gnss_cfg_data);
  xTimerStop(sl_get_wifi_asset_tracking_resource()->gnss_sensor_timer, 0);

  if (status == SL_STATUS_OK) {
    printf(
      "\r\nsl_init_max_m10s_gnss_receiver : Successfully initialized GNSS receiver\n");

    xTimerStart(sl_get_wifi_asset_tracking_resource()->gnss_sensor_timer, 0);

    status = gnss_max_m10s_set_i2c_output(&gnss_cfg_data,
                                          COM_TYPE_UBX,
                                          VAL_LAYER_RAM_BBR,
                                          GNSS_POLL_MAX_TIMEOUT);

    xTimerStop(sl_get_wifi_asset_tracking_resource()->gnss_sensor_timer, 0);

    if (status == SL_STATUS_OK) {
#if DEMO_CONFIG_DEBUG_LOGS
      printf(
        "\r\nsl_init_max_m10s_gnss_receiver : output port is set to UBX\r\n");
#endif /// < DEMO_CONFIG_DEBUG_LOGS
    } else {
#if DEMO_CONFIG_DEBUG_LOGS
      printf(
        "\r\nsl_init_max_m10s_gnss_receiver : SetI2C output port failed !!\r\n");
#endif /// < DEMO_CONFIG_DEBUG_LOGS
    }
  } else {
    printf("gnss_max_m10s_begin fail 0x%lx\r\n", status);
    return SL_STATUS_FAIL;
  }

  return status;
}

/******************************************************************************
 *  Disable on-board Si7021 temperature and RH sensor.
 *****************************************************************************/
sl_status_t sl_disable_onboard_si7021_sensor()
{
  sl_status_t status;
  sl_si91x_gpio_driver_get_uulp_npss_pin(UULP_GPIO_1_PIN);

  /// Enable GPIO ULP_CLK
  status = sl_si91x_gpio_driver_enable_clock(
    (sl_si91x_gpio_select_clock_t)ULPCLK_GPIO);

  if (status != SL_STATUS_OK) {
    printf(
      "\r\nsl_disable_onboard_si7021_sensor : sl_si91x_gpio_driver_enable_clock, Error code: %lu",
      status);
    return SL_STATUS_FAIL;
  }

  /// Set NPSS GPIO pin MUX
  status = sl_si91x_gpio_driver_set_uulp_npss_pin_mux(UULP_GPIO_1_PIN,
                                                      NPSS_GPIO_PIN_MUX_MODE0);

  if (status != SL_STATUS_OK) {
    printf(
      "\r\nsl_disable_onboard_si7021_sensor : sl_si91x_gpio_driver_set_uulp_npss_pin_mux, Error code: %lu",
      status);
    return SL_STATUS_FAIL;
  }

  /// Set NPSS GPIO pin direction
  status =
    sl_si91x_gpio_driver_set_uulp_npss_direction(UULP_GPIO_1_PIN,
                                                 (sl_si91x_gpio_direction_t)GPIO_OUTPUT);

  if (status != SL_STATUS_OK) {
    printf(
      "\r\nsl_disable_onboard_si7021_sensor : sl_si91x_gpio_driver_set_uulp_npss_direction, Error code: %lu",
      status);
    return SL_STATUS_FAIL;
  }

  /// Set UULP GPIO pin
  status = sl_si91x_gpio_driver_set_uulp_npss_pin_value(UULP_GPIO_1_PIN, CLR);

  if (status != SL_STATUS_OK) {
    printf(
      "\r\nsl_disable_onboard_si7021_sensor : sl_si91x_gpio_driver_set_uulp_npss_pin_value, Error code: %lu",
      status);
    return SL_STATUS_FAIL;
  }

  printf(
    "\r\nsl_disable_onboard_si7021_sensor : Successfully disabled on-board Si7021 temperature and RH sensor\r\n");

  return SL_STATUS_OK;
}

/******************************************************************************
 *  Callback function to capture temperature and RH sensor data at configured interval.
 *****************************************************************************/
void sl_capture_temperature_rh_sensor_data_task()
{
  sl_status_t status = SL_STATUS_OK;
  sl_wifi_asset_tracking_sensor_queue_data_t si7021_reading;
  TickType_t task_delay;
  TickType_t initial_tick_count = 0, later_tick_count = 0;
  TickType_t processing_diff = 0;
  uint32_t temp_sampling_interval =
    DEMO_CONFIG_TEMP_RH_SENSOR_SAMPLING_INTERVAL * 1000;
  uint32_t humidity = 0;
  int32_t temperature = 0;

  /// Appending sensor unit and its type
  strcpy((char *)(si7021_reading.temp_rh_data.temperature_unit),
         TEMPERATURE_UNIT_STRING);
  si7021_reading.sensor_type = SL_TEMP_RH_SENSOR;

  if ((SL_SENSOR_NOT_PROBED
       == sl_get_wifi_asset_tracking_status()->sensor_status.
       temp_rh_sensor_probe_status)
      || (SL_SENSOR_RECONNECTED
          == sl_get_wifi_asset_tracking_status()->sensor_status.
          temp_rh_sensor_probe_status)) {
    if (SL_SENSOR_RECONNECTED
        == sl_get_wifi_asset_tracking_status()->sensor_status.
        temp_rh_sensor_probe_status) {
      vTaskDelay(pdMS_TO_TICKS(SENSOR_PER_RETRY_DELAY) * TIMER_CLOCK_OFFSET);
    }

    if (SL_STATUS_OK == sl_disable_onboard_si7021_sensor()) {
      if (pdTRUE
          == xSemaphoreTake(sl_get_wifi_asset_tracking_resource()->
                            i2c_mutex_handler,
                            portMAX_DELAY)) {
        /// Initialize the si7021 sensor
        status = sl_init_si7021_temperature_and_rh_sensor();

        xSemaphoreGive(sl_get_wifi_asset_tracking_resource()->i2c_mutex_handler);

        if (SL_STATUS_OK == status) {
          /// make sensor status flag as working
          sl_get_wifi_asset_tracking_status()->sensor_status.
          temp_rh_sensor_probe_status = SL_SENSOR_CONNECTED;

          sl_wifi_asset_tracking_lcd_print(INDEX_SI7021_CONNECTED);

          sl_get_wifi_asset_tracking_status()->sensor_status.
          temp_rh_sensor_retry_cnt = 0;
        } else {
          sl_get_wifi_asset_tracking_status()->sensor_status.
          temp_rh_sensor_probe_status = SL_SENSOR_PROBE_FAILED;

          sl_wifi_asset_tracking_lcd_print(INDEX_SI7021_NOT_CONNECTED);

          if (eSuspended
              == eTaskGetState(sl_get_wifi_asset_tracking_resource()->task_list.
                               recovery_task_handler)) {
            vTaskResume(
              sl_get_wifi_asset_tracking_resource()->task_list.recovery_task_handler);
          }
        }
      }
    }
  }

  /// If wi-fi not in initialized state then only suspend temperature and RH sensor task
  if (SL_WIFI_NOT_CONNECTED
      == sl_get_wifi_asset_tracking_status()->wifi_conn_status) {
    printf(
      "\r\ntemperature_rh_sensor_task : suspending task as sensor got probed but wi-fi is not connected\r\n");
    vTaskSuspend(
      sl_get_wifi_asset_tracking_resource()->task_list.temp_rh_sensor_task_handler);
  }

  while (1) {
    initial_tick_count = xTaskGetTickCount();

    if (SL_SENSOR_CONNECTED
        == sl_get_wifi_asset_tracking_status()->sensor_status.
        temp_rh_sensor_probe_status) {
      /// Reads si7021 sensor
      if (pdTRUE
          == xSemaphoreTake(sl_get_wifi_asset_tracking_resource()->
                            i2c_mutex_handler,
                            portMAX_DELAY)) {
        xTimerStart(
          sl_get_wifi_asset_tracking_resource()->temperature_rh_sensor_timer,
          0);
        status = sl_si91x_si70xx_read_temp_from_rh(I2C,
                                                   SI7021_ADDR,
                                                   &humidity,
                                                   &temperature);
        xTimerStop(
          sl_get_wifi_asset_tracking_resource()->temperature_rh_sensor_timer,
          0);
        xSemaphoreGive(sl_get_wifi_asset_tracking_resource()->i2c_mutex_handler);
      }

      if (status != SL_STATUS_OK) {
        si7021_reading.is_sensor_data_available = false;
        printf("\r\ntemperature_rh_sensor_task : si7021 Sensor read is failed\n");
      } else {
        si7021_reading.is_sensor_data_available = true;

#if DEMO_CONFIG_DEBUG_LOGS
        printf(
          "\r\ntemperature_rh_sensor_task : si7021 Sensor read is successful\n");
#endif /// < DEMO_CONFIG_DEBUG_LOGS
      }
    } else {
      si7021_reading.is_sensor_data_available = false;
      goto taskdelay;
    }

    /// If failed to fetch time-stamp then discard the packet
    if (SL_STATUS_OK != sl_json_get_timestamp(si7021_reading.time_stamp)) {
      printf(
        "\r\ntemperature_rh_sensor_task :  failed to fetch time-stamp, discarding the packet\r\n");
      goto taskdelay;
    }

    si7021_reading.temp_rh_data.temperature = (double)temperature;
    si7021_reading.temp_rh_data.relative_humidity = (double)humidity;

    /// Acquire sensor data queue mutex and send data to sensor data queue
    if (pdTRUE
        == xSemaphoreTake(sl_get_wifi_asset_tracking_resource()->
                          sensor_data_queue_mutex_handler,
                          portMAX_DELAY)) {
      if (pdTRUE
          == xQueueSend(sl_get_wifi_asset_tracking_resource()->
                        sensor_data_queue_handler,
                        &si7021_reading, 0)) {
      } else {
        /// Receive sensor data as sensor data queue is full
        if (pdTRUE
            == xQueueReceive(sl_get_wifi_asset_tracking_resource()->
                             sensor_data_queue_handler,
                             &si7021_reading, 0)) {
          printf(
            "\r\ntemperature_rh_sensor_task : received sensor data as sensor data queue is full\r\n");

          /// again send latest si7021 latest data to sensor data queue
          xQueueSend(
            sl_get_wifi_asset_tracking_resource()->sensor_data_queue_handler,
            &si7021_reading,
            0);
        }
      }
      printf(
        "\r\ntemperature_rh_sensor_task : si7021 sensor data is sent to sensor data queue\r\n");

      xSemaphoreGive(
        sl_get_wifi_asset_tracking_resource()->sensor_data_queue_mutex_handler);
    }

    /// check whether JSON data converter task is suspended or not
    if (eSuspended
        == eTaskGetState(sl_get_wifi_asset_tracking_resource()->task_list.
                         json_data_converter_task_handler)) {
      vTaskResume(
        sl_get_wifi_asset_tracking_resource()->task_list.json_data_converter_task_handler);
    }

    taskdelay:

    later_tick_count = xTaskGetTickCount();

    /// Sampling interval and setting a offset
    task_delay = pdMS_TO_TICKS(temp_sampling_interval) * TIMER_CLOCK_OFFSET;

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
    printf("\r\ntemperature_rh_sensor_task : delay is : %ld\r\n", task_delay);
#endif /// < DEMO_CONFIG_DEBUG_LOGS

    vTaskDelay(task_delay);
  }
}

/******************************************************************************
 *  Callback function to capture IMU sensor data at configured interval.
 *****************************************************************************/
void sl_capture_imu_sensor_data_task()
{
  sl_status_t acc_status = SL_STATUS_OK, gyro_status = SL_STATUS_OK;
  sl_status_t status = SL_STATUS_OK;
  sl_wifi_asset_tracking_sensor_queue_data_t bmi270_reading;
  TickType_t task_delay;
  TickType_t initial_tick_count = 0, later_tick_count = 0;
  TickType_t processing_diff = 0;
  uint32_t imu_sampling_interval = DEMO_CONFIG_IMU_SENSOR_SAMPLING_INTERVAL
                                   * 1000;

  /// Appending sensor type
  bmi270_reading.sensor_type = SL_IMU_SENSOR;

  if ((SL_SENSOR_NOT_PROBED
       == sl_get_wifi_asset_tracking_status()->sensor_status.
       imu_sensor_probe_status)
      || (SL_SENSOR_RECONNECTED
          == sl_get_wifi_asset_tracking_status()->sensor_status.
          imu_sensor_probe_status)) {
    if (SL_SENSOR_RECONNECTED
        == sl_get_wifi_asset_tracking_status()->sensor_status.
        imu_sensor_probe_status) {
      vTaskDelay(pdMS_TO_TICKS(SENSOR_PER_RETRY_DELAY) * TIMER_CLOCK_OFFSET);
    }

    if (pdTRUE
        == xSemaphoreTake(sl_get_wifi_asset_tracking_resource()->
                          i2c_mutex_handler,
                          portMAX_DELAY)) {
      status = sl_init_bmi270_imu_sensor();

      xSemaphoreGive(sl_get_wifi_asset_tracking_resource()->i2c_mutex_handler);

      /// Initialize the bmi270 sensor
      if (SL_STATUS_OK == status) {
        /// make sensor status flag as working
        sl_get_wifi_asset_tracking_status()->sensor_status.
        imu_sensor_probe_status = SL_SENSOR_CONNECTED;

        sl_wifi_asset_tracking_lcd_print(INDEX_BMI270_CONNECTED);

        sl_get_wifi_asset_tracking_status()->sensor_status.imu_sensor_retry_cnt
          = 0;
      } else {
        sl_get_wifi_asset_tracking_status()->sensor_status.
        imu_sensor_probe_status = SL_SENSOR_PROBE_FAILED;

        sl_wifi_asset_tracking_lcd_print(INDEX_BMI270_NOT_CONNECTED);

        if (eSuspended
            == eTaskGetState(sl_get_wifi_asset_tracking_resource()->task_list.
                             recovery_task_handler)) {
          vTaskResume(
            sl_get_wifi_asset_tracking_resource()->task_list.recovery_task_handler);
        }
      }
    }
  }

  /// If wi-fi not in initialized state then only suspend IMU sensor task
  if (SL_WIFI_NOT_CONNECTED
      == sl_get_wifi_asset_tracking_status()->wifi_conn_status) {
    printf(
      "\r\nimu_sensor_task : suspending task as sensor got probed but wi-fi is not connected\r\n");
    vTaskSuspend(
      sl_get_wifi_asset_tracking_resource()->task_list.imu_sensor_task_handler);
  }

  while (1) {
    initial_tick_count = xTaskGetTickCount();

    if (SL_SENSOR_CONNECTED
        == sl_get_wifi_asset_tracking_status()->sensor_status.
        imu_sensor_probe_status) {
      if (pdTRUE
          == xSemaphoreTake(sl_get_wifi_asset_tracking_resource()->
                            i2c_mutex_handler,
                            portMAX_DELAY)) {
        /// Read accelerometer data
        xTimerStart(sl_get_wifi_asset_tracking_resource()->imu_sensor_timer, 0);
        acc_status = sparkfun_bmi270_read_acc_reading(&bmi_cfg_data,
                                                      bmi270_reading.imu_data.accelerometer);
        xTimerStop(sl_get_wifi_asset_tracking_resource()->imu_sensor_timer, 0);
        xSemaphoreGive(sl_get_wifi_asset_tracking_resource()->i2c_mutex_handler);
      }

      if (acc_status != SL_STATUS_OK) {
        bmi270_reading.is_sensor_data_available = false;
        printf(
          "\r\nimu_sensor_task : bmi270 Accelerometer sensor read is failed\n");
      } else {
        if (pdTRUE
            == xSemaphoreTake(sl_get_wifi_asset_tracking_resource()->
                              i2c_mutex_handler,
                              portMAX_DELAY)) {
          xTimerStart(sl_get_wifi_asset_tracking_resource()->imu_sensor_timer,
                      0);
          gyro_status = sparkfun_bmi270_read_gyro_reading(&bmi_cfg_data,
                                                          bmi270_reading.imu_data.gyroscope);
          xTimerStop(sl_get_wifi_asset_tracking_resource()->imu_sensor_timer,
                     0);
          xSemaphoreGive(
            sl_get_wifi_asset_tracking_resource()->i2c_mutex_handler);
        }
        if (gyro_status != SL_STATUS_OK) {
          bmi270_reading.is_sensor_data_available = false;
          printf(
            "\r\nimu_sensor_task : bmi270 gyroscope sensor read is failed\n");
        } else {
          bmi270_reading.is_sensor_data_available = true;
        }
      }
    } else {
      bmi270_reading.is_sensor_data_available = false;
      goto taskdelay;
    }

    /// If failed to fetch time-stamp then discard the packet
    if (SL_STATUS_OK != sl_json_get_timestamp(bmi270_reading.time_stamp)) {
      printf(
        "\r\nimu_sensor_task : failed to fetch time-stamp, discarding the packet\r\n");
      goto taskdelay;
    }

    /// Acquire sensor data queue mutex and send data to sensor data queue
    if (pdTRUE
        == xSemaphoreTake(sl_get_wifi_asset_tracking_resource()->
                          sensor_data_queue_mutex_handler,
                          portMAX_DELAY)) {
      if (pdTRUE
          == xQueueSend(sl_get_wifi_asset_tracking_resource()->
                        sensor_data_queue_handler,
                        &bmi270_reading, 0)) {
      } else {
        /// Receive sensor data as sensor data queue is full
        if (pdTRUE
            == xQueueReceive(sl_get_wifi_asset_tracking_resource()->
                             sensor_data_queue_handler,
                             &bmi270_reading, 0)) {
          printf(
            "\r\nimu_sensor_task : received sensor data as sensor data queue is full\r\n");
          /// again send latest bmi270 latest data to sensor data queue
          xQueueSend(
            sl_get_wifi_asset_tracking_resource()->sensor_data_queue_handler,
            &bmi270_reading,
            0);
        }
      }

      printf(
        "\r\nimu_sensor_task : bmi270 sensor data is sent to sensor data queue\r\n");

      xSemaphoreGive(
        sl_get_wifi_asset_tracking_resource()->sensor_data_queue_mutex_handler);
    }

    /// check whether JSON data converter task is suspended or not
    if (eSuspended
        == eTaskGetState(sl_get_wifi_asset_tracking_resource()->task_list.
                         json_data_converter_task_handler)) {
      vTaskResume(
        sl_get_wifi_asset_tracking_resource()->task_list.json_data_converter_task_handler);
    }

    taskdelay:

    later_tick_count = xTaskGetTickCount();

    /// Sampling interval and setting a offset
    task_delay = pdMS_TO_TICKS(imu_sampling_interval) * TIMER_CLOCK_OFFSET;

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
    printf("\r\nimu_sensor_task : delay is : %ld\r\n", task_delay);
#endif /// < DEMO_CONFIG_DEBUG_LOGS

    vTaskDelay(task_delay);
  }
}

/******************************************************************************
 *  Callback function to capture GNSS receiver data at configured interval.
 *****************************************************************************/
void sl_capture_gnss_receiver_data_task()
{
  sl_wifi_asset_tracking_sensor_queue_data_t gnss_reading;
  TickType_t initial_tick_count = 0, later_tick_count = 0;
  TickType_t processing_diff = 0;
  uint32_t gnss_sampling_interval =
    DEMO_CONFIG_GNSS_RECEIVER_SAMPLING_INTERVAL * 1000;
  TickType_t task_delay;
  uint8_t fix_type = 0;
  uint8_t retry_count = 0, data_retry_count = 0;
  sl_status_t status = SL_STATUS_OK;
  /// Appending sensor type
  gnss_reading.sensor_type = SL_GNSS_RECEIVER;

  if ((SL_SENSOR_NOT_PROBED
       == sl_get_wifi_asset_tracking_status()->sensor_status.
       gnss_receiver_probe_status)
      || (SL_SENSOR_RECONNECTED
          == sl_get_wifi_asset_tracking_status()->sensor_status.
          gnss_receiver_probe_status)) {
    if (SL_SENSOR_RECONNECTED
        == sl_get_wifi_asset_tracking_status()->sensor_status.
        gnss_receiver_probe_status) {
      vTaskDelay(pdMS_TO_TICKS(SENSOR_PER_RETRY_DELAY) * TIMER_CLOCK_OFFSET);
    }

    if (pdTRUE
        == xSemaphoreTake(sl_get_wifi_asset_tracking_resource()->
                          i2c_mutex_handler,
                          portMAX_DELAY)) {
      /// Initialize the gnss receiver sensor
      status = sl_init_max_m10s_gnss_receiver();

      xSemaphoreGive(sl_get_wifi_asset_tracking_resource()->i2c_mutex_handler);

      if (SL_STATUS_OK == status) {
        /// make sensor status flag as working
        sl_get_wifi_asset_tracking_status()->sensor_status.
        gnss_receiver_probe_status = SL_SENSOR_CONNECTED;

        sl_wifi_asset_tracking_lcd_print(INDEX_MAX_M10S_CONNECTED);

        sl_get_wifi_asset_tracking_status()->sensor_status.
        gnss_receiver_retry_cnt = 0;
      } else {
        sl_get_wifi_asset_tracking_status()->sensor_status.
        gnss_receiver_probe_status = SL_SENSOR_PROBE_FAILED;

        sl_wifi_asset_tracking_lcd_print(INDEX_MAX_M10S_NOT_CONNECTED);

        if (eSuspended
            == eTaskGetState(sl_get_wifi_asset_tracking_resource()->task_list.
                             recovery_task_handler)) {
          vTaskResume(
            sl_get_wifi_asset_tracking_resource()->task_list.recovery_task_handler);
        }
      }
    }
  }

  /// If wi-fi not in initialized state then only suspend GNNS receiver task
  if (SL_WIFI_NOT_CONNECTED
      == sl_get_wifi_asset_tracking_status()->wifi_conn_status) {
    printf(
      "\r\ngnss_receiver_task : suspending task as sensor got probed but wi-fi is not connected\r\n");
    vTaskSuspend(
      sl_get_wifi_asset_tracking_resource()->task_list.gnss_receiver_task_handler);
  }

  while (1) {
    initial_tick_count = xTaskGetTickCount();

    if (SL_SENSOR_CONNECTED
        == sl_get_wifi_asset_tracking_status()->sensor_status.
        gnss_receiver_probe_status) {
      gnss_reading.is_sensor_data_available = false;

      while (GNSS_RETRY_COUNT > retry_count) {
        if (pdTRUE
            == xSemaphoreTake(sl_get_wifi_asset_tracking_resource()->
                              i2c_mutex_handler,
                              portMAX_DELAY)) {
          xTimerStart(sl_get_wifi_asset_tracking_resource()->gnss_sensor_timer,
                      0);
          status = gnss_max_m10s_get_fix_type(&gnss_cfg_data,
                                              GNSS_POLL_MAX_TIMEOUT,
                                              &fix_type);
          xTimerStop(sl_get_wifi_asset_tracking_resource()->gnss_sensor_timer,
                     0);

          if (status != SL_STATUS_OK) {
            fix_type = 0;
          }
          xSemaphoreGive(
            sl_get_wifi_asset_tracking_resource()->i2c_mutex_handler);
        }
#if DEMO_CONFIG_DEBUG_LOGS
        printf("\r\ngnss_receiver_task : fix type is: %d\r\n", fix_type);
#endif /// < DEMO_CONFIG_DEBUG_LOGS

        if (((SL_MAX_M10S_PROTOCOL_UBX == gnss_cfg_data.protocol_type)
             && ((3 == fix_type) || (2 == fix_type)))
            || ((SL_MAX_M10S_PROTOCOL_NMEA == gnss_cfg_data.protocol_type)
                && (1 == fix_type))) {
          for (data_retry_count = 0; data_retry_count < GNSS_DATA_RETRY_COUNT;
               ++data_retry_count) {
            if (pdTRUE
                == xSemaphoreTake(sl_get_wifi_asset_tracking_resource()->
                                  i2c_mutex_handler,
                                  portMAX_DELAY)) {
              xTimerStart(
                sl_get_wifi_asset_tracking_resource()->gnss_sensor_timer,
                0);
              status = gnss_max_m10s_get_nav_pvt(&gnss_cfg_data,
                                                 GNSS_POLL_MAX_TIMEOUT);
              xTimerStop(
                sl_get_wifi_asset_tracking_resource()->gnss_sensor_timer,
                0);

              xSemaphoreGive(
                sl_get_wifi_asset_tracking_resource()->i2c_mutex_handler);

              if (status == SL_STATUS_OK) {
                gnss_reading.gnss_data.latitude =
                  (double)gnss_cfg_data.packetUBXNAVPVT->data.lat;

                if (SL_MAX_M10S_PROTOCOL_UBX == gnss_cfg_data.protocol_type) {
                  gnss_reading.gnss_data.latitude /= LAT_LONG_DIVISOR_UBX;
                } else if (SL_MAX_M10S_PROTOCOL_NMEA
                           == gnss_cfg_data.protocol_type) {
                  gnss_reading.gnss_data.latitude /= LAT_LONG_DIVISOR_NMEA;
                }
#if DEMO_CONFIG_DEBUG_LOGS
                printf("\r\ngnss_receiver_task : latitude is : %.7lf\r\n",
                       gnss_reading.gnss_data.latitude);
#endif /// < DEMO_CONFIG_DEBUG_LOGS

                gnss_reading.gnss_data.longitude =
                  (double)gnss_cfg_data.packetUBXNAVPVT->data.lon;

                if (SL_MAX_M10S_PROTOCOL_UBX == gnss_cfg_data.protocol_type) {
                  gnss_reading.gnss_data.longitude /= LAT_LONG_DIVISOR_UBX;
                } else if (SL_MAX_M10S_PROTOCOL_NMEA
                           == gnss_cfg_data.protocol_type) {
                  gnss_reading.gnss_data.longitude /= LAT_LONG_DIVISOR_NMEA;
                }
#if DEMO_CONFIG_DEBUG_LOGS
                printf("\r\ngnss_receiver_task : longitude is : %.7lf\r\n",
                       gnss_reading.gnss_data.longitude);
#endif /// < DEMO_CONFIG_DEBUG_LOGS

                gnss_reading.gnss_data.altitude =
                  (double)gnss_cfg_data.packetUBXNAVPVT->data.hMSL;
#if DEMO_CONFIG_DEBUG_LOGS
                printf("\r\ngnss_receiver_task : altitude is : %lf\r\n",
                       gnss_reading.gnss_data.altitude);
#endif /// < DEMO_CONFIG_DEBUG_LOGS

                gnss_reading.gnss_data.no_of_satellites =
                  (int32_t)gnss_cfg_data.packetUBXNAVPVT->data.numSV;
#if DEMO_CONFIG_DEBUG_LOGS
                printf("\r\ngnss_receiver_task : satellite is : %ld\r\n",
                       gnss_reading.gnss_data.no_of_satellites);
#endif /// < DEMO_CONFIG_DEBUG_LOGS

                gnss_reading.is_sensor_data_available = true;
                break;
              } else {
                gnss_max_m10s_delay(GNSS_DATA_TIMEOUT_RETRY_DELAY);

#if DEMO_CONFIG_DEBUG_LOGS
                printf(
                  "\r\ngnss_receiver_task : Data is not received, retry-count: %d\r\n",
                  data_retry_count);
#endif /// < DEMO_CONFIG_DEBUG_LOGS
                continue;
              }
            }
          }
        }

        if (true == gnss_reading.is_sensor_data_available) {
          printf(
            "\r\ngnss_receiver_task : Data is received, break the fix-type retry loop\r\n");
          break;
        }

        retry_count++;
        gnss_max_m10s_delay(GNSS_PER_RETRY_DELAY);
      }

      /// If retry count reached to maximum value then send data as null
      if (GNSS_RETRY_COUNT == retry_count) {
        retry_count = 0;
        gnss_reading.is_sensor_data_available = false;
        printf(
          "\r\ngnss_receiver_task : GNSS receiver read failed due to fix type not found, fix_type:%u\n",
          fix_type);
        goto taskdelay;
      }
    } else {
      gnss_reading.is_sensor_data_available = false;
      goto taskdelay;
    }

    /// If failed to fetch time-stamp then discard the packet
    if (SL_STATUS_OK != sl_json_get_timestamp(gnss_reading.time_stamp)) {
      printf(
        "\r\ngnss_receiver_task : failed to fetch time-stamp, discarding the packet\r\n");
      goto taskdelay;
    }

    /// Acquire sensor data queue mutex and send data to sensor data queue
    if (pdTRUE
        == xSemaphoreTake(sl_get_wifi_asset_tracking_resource()->
                          sensor_data_queue_mutex_handler,
                          portMAX_DELAY)) {
      if (pdTRUE
          == xQueueSend(sl_get_wifi_asset_tracking_resource()->
                        sensor_data_queue_handler,
                        &gnss_reading, 0)) {
      } else {
        /// Recieve sensor data as sensor data queue is full
        if (pdTRUE
            == xQueueReceive(sl_get_wifi_asset_tracking_resource()->
                             sensor_data_queue_handler,
                             &gnss_reading, 0)) {
          printf(
            "\r\ngnss_receiver_task : received sensor data as sensor data queue is full\r\n");
          /// again send latest si7021 latest data to sensor data queue
          xQueueSend(
            sl_get_wifi_asset_tracking_resource()->sensor_data_queue_handler,
            &gnss_reading,
            0);
        }
      }

      printf(
        "\r\ngnss_receiver_task : gnss receiver data is sent to sensor data queue\r\n");

      xSemaphoreGive(
        sl_get_wifi_asset_tracking_resource()->sensor_data_queue_mutex_handler);
    }

    /// check whether JSON data converter task is suspended or not
    if (eSuspended
        == eTaskGetState(sl_get_wifi_asset_tracking_resource()->task_list.
                         json_data_converter_task_handler)) {
      vTaskResume(
        sl_get_wifi_asset_tracking_resource()->task_list.json_data_converter_task_handler);
    }

    taskdelay:

    later_tick_count = xTaskGetTickCount();

    /// Sampling interval and setting a offset
    task_delay = pdMS_TO_TICKS(gnss_sampling_interval) * TIMER_CLOCK_OFFSET;

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
    printf("\r\ngnss_receiver_task : delay is : %ld\r\n", task_delay);
#endif /// < DEMO_CONFIG_DEBUG_LOGS

    vTaskDelay(task_delay);
  }
}

/******************************************************************************
 *  Callback function of temperature_rh_sensor timer
 *****************************************************************************/
void on_temperature_rh_sensor_timer_callback()
{
  xSemaphoreGiveFromISR(
    sl_get_wifi_asset_tracking_resource()->i2c_mutex_handler,
    (BaseType_t *)pdTRUE);

  if (SL_SENSOR_NOT_PROBED
      == sl_get_wifi_asset_tracking_status()->sensor_status.
      temp_rh_sensor_probe_status) {
    sl_get_wifi_asset_tracking_status()->sensor_status.
    temp_rh_sensor_probe_status = SL_SENSOR_PROBE_FAILED;

    if (eSuspended
        == eTaskGetState(sl_get_wifi_asset_tracking_resource()->task_list.
                         recovery_task_handler)) {
      vTaskResume(
        sl_get_wifi_asset_tracking_resource()->task_list.recovery_task_handler);
    }
  } else if ((SL_SENSOR_CONNECTED
              == sl_get_wifi_asset_tracking_status()->sensor_status.
              temp_rh_sensor_probe_status)
             || (SL_SENSOR_RECONNECTED
                 == sl_get_wifi_asset_tracking_status()->sensor_status.
                 temp_rh_sensor_probe_status)) {
    sl_get_wifi_asset_tracking_status()->sensor_status.
    temp_rh_sensor_probe_status = SL_SENSOR_DISCONNECTED;

    if (eSuspended
        == eTaskGetState(sl_get_wifi_asset_tracking_resource()->task_list.
                         recovery_task_handler)) {
      vTaskResume(
        sl_get_wifi_asset_tracking_resource()->task_list.recovery_task_handler);
    }
  }
}

/******************************************************************************
 *  Callback function of imu sensor timer
 *****************************************************************************/
void on_imu_sensor_timer_callback()
{
  xSemaphoreGiveFromISR(
    sl_get_wifi_asset_tracking_resource()->i2c_mutex_handler,
    (BaseType_t *)pdTRUE);

  if (SL_SENSOR_NOT_PROBED
      == sl_get_wifi_asset_tracking_status()->sensor_status.
      imu_sensor_probe_status) {
    sl_get_wifi_asset_tracking_status()->sensor_status.imu_sensor_probe_status =
      SL_SENSOR_PROBE_FAILED;

    if (eSuspended
        == eTaskGetState(sl_get_wifi_asset_tracking_resource()->task_list.
                         recovery_task_handler)) {
      vTaskResume(
        sl_get_wifi_asset_tracking_resource()->task_list.recovery_task_handler);
    }
  } else if ((SL_SENSOR_CONNECTED
              == sl_get_wifi_asset_tracking_status()->sensor_status.
              imu_sensor_probe_status)
             || (SL_SENSOR_RECONNECTED
                 == sl_get_wifi_asset_tracking_status()->sensor_status.
                 imu_sensor_probe_status)) {
    sl_get_wifi_asset_tracking_status()->sensor_status.imu_sensor_probe_status =
      SL_SENSOR_DISCONNECTED;

    if (eSuspended
        == eTaskGetState(sl_get_wifi_asset_tracking_resource()->task_list.
                         recovery_task_handler)) {
      vTaskResume(
        sl_get_wifi_asset_tracking_resource()->task_list.recovery_task_handler);
    }
  }
}

/******************************************************************************
 *  Callback function of gnss sensor timer
 *****************************************************************************/
void on_gnss_sensor_timer_callback()
{
  xSemaphoreGiveFromISR(
    sl_get_wifi_asset_tracking_resource()->i2c_mutex_handler,
    (BaseType_t *)pdTRUE);

  if (SL_SENSOR_NOT_PROBED
      == sl_get_wifi_asset_tracking_status()->sensor_status.
      gnss_receiver_probe_status) {
    sl_get_wifi_asset_tracking_status()->sensor_status.
    gnss_receiver_probe_status = SL_SENSOR_PROBE_FAILED;

    if (eSuspended
        == eTaskGetState(sl_get_wifi_asset_tracking_resource()->task_list.
                         recovery_task_handler)) {
      vTaskResume(
        sl_get_wifi_asset_tracking_resource()->task_list.recovery_task_handler);
    }
  } else if ((SL_SENSOR_CONNECTED
              == sl_get_wifi_asset_tracking_status()->sensor_status.
              gnss_receiver_probe_status)
             || (SL_SENSOR_RECONNECTED
                 == sl_get_wifi_asset_tracking_status()->sensor_status.
                 gnss_receiver_probe_status)) {
    sl_get_wifi_asset_tracking_status()->sensor_status.
    gnss_receiver_probe_status = SL_SENSOR_DISCONNECTED;

    if (eSuspended
        == eTaskGetState(sl_get_wifi_asset_tracking_resource()->task_list.
                         recovery_task_handler)) {
      vTaskResume(
        sl_get_wifi_asset_tracking_resource()->task_list.recovery_task_handler);
    }
  }
}

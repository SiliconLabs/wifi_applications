/***************************************************************************/ /**
 * @file sl_wifi_asset_tracking_json_data_handler.c
 * @brief json data converter related functions
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
#include <stdio.h>
#include <string.h>
#include <sl_si91x_calendar.h>
#include <azure_iot_json_writer.h>
#include <sl_net_default_values.h>
#include <sl_wifi_asset_tracking_app.h>
#include <sl_wifi_asset_tracking_json_data_handler.h>
#include <sl_wifi_asset_tracking_wifi_handler.h>
#include <sl_wifi_asset_tracking_demo_config.h>

/******************************************************************************
 *  Callback function to convert bmi270 data format to JSON data format.
 *****************************************************************************/
sl_status_t sl_convert_bmi270_reading_to_json_format(
  sl_wifi_asset_tracking_sensor_queue_data_t *sensor_data_queue_reading)
{
  AzureIoTJSONWriter_t bmi270_writer;
  sl_wifi_asset_tracking_mqtt_package_queue_data_t bmi270_json_data;
  AzureIoTResult_t writer_status;

  /// Initialize the JSON writer
  writer_status = AzureIoTJSONWriter_Init(&bmi270_writer,
                                          bmi270_json_data.mqtt_buffer,
                                          sizeof(bmi270_json_data.mqtt_buffer));
  if (writer_status != eAzureIoTSuccess) {
    printf(
      "\r\nsl_convert_bmi270_reading_to_json_format : Failed to initialize JSON writer error code: %d\r\n",
      writer_status);
    goto error;
  }

  /// Construct the JSON message
  writer_status = AzureIoTJSONWriter_AppendBeginObject(&bmi270_writer);
  if (writer_status != eAzureIoTSuccess) {
    printf(
      "\r\nsl_convert_bmi270_reading_to_json_format : Failed to append begin object error code: %d\r\n",
      writer_status);
    goto error;
  }

  /// Append msgtype
  writer_status = AzureIoTJSONWriter_AppendPropertyWithStringValue(
    &bmi270_writer,
    (const
     uint8_t *)JSON_PROPERTY_MSGTYPE,
    strlen(
      JSON_PROPERTY_MSGTYPE),
    (const
     uint8_t *)JSON_PROPERTY_IMU,
    strlen(
      JSON_PROPERTY_IMU));
  if (writer_status != eAzureIoTSuccess) {
    printf(
      "\r\nsl_convert_bmi270_reading_to_json_format : Failed to append message type with value error code: %d\r\n",
      writer_status);
    goto error;
  }

  /// Append time-stamp
  writer_status = AzureIoTJSONWriter_AppendPropertyWithStringValue(
    &bmi270_writer,
    (const
     uint8_t *)JSON_PROPERTY_TIMESTAMP,
    strlen(
      JSON_PROPERTY_TIMESTAMP),
    (const
     uint8_t *)sensor_data_queue_reading->time_stamp,
    strlen((char
            *)sensor_data_queue_reading->time_stamp));
  if (writer_status != eAzureIoTSuccess) {
    printf(
      "\r\nsl_convert_bmi270_reading_to_json_format : Failed to append time stamp with value error code: %d\r\n",
      writer_status);
    goto error;
  }

  /// If sensor data is not available then send null value
  if (!sensor_data_queue_reading->is_sensor_data_available) {
    /// Append accelero property
    writer_status = AzureIoTJSONWriter_AppendPropertyName(&bmi270_writer,
                                                          (const uint8_t *)JSON_PROPERTY_ACCELERO,
                                                          strlen(
                                                            JSON_PROPERTY_ACCELERO));
    if (writer_status != eAzureIoTSuccess) {
      printf(
        "\r\nsl_convert_bmi270_reading_to_json_format : Failed to append accelerometer property error code: %d\r\n",
        writer_status);
      goto error;
    }

    /// Append null value to accelero object
    writer_status = AzureIoTJSONWriter_AppendNull(&bmi270_writer);
    if (writer_status != eAzureIoTSuccess) {
      printf(
        "\r\nsl_convert_bmi270_reading_to_json_format : Failed to append null value to accelero error code: %d\r\n",
        writer_status);
      goto error;
    }

    /// Append gyro property
    writer_status = AzureIoTJSONWriter_AppendPropertyName(&bmi270_writer,
                                                          (const uint8_t *)JSON_PROPERTY_GYRO,
                                                          strlen(
                                                            JSON_PROPERTY_GYRO));
    if (writer_status != eAzureIoTSuccess) {
      printf(
        "\r\nsl_convert_bmi270_reading_to_json_format : Failed to append gyroscope property error code: %d\r\n",
        writer_status);
      goto error;
    }

    /// Append null value to gyro object
    writer_status = AzureIoTJSONWriter_AppendNull(&bmi270_writer);
    if (writer_status != eAzureIoTSuccess) {
      printf(
        "\r\nsl_convert_bmi270_reading_to_json_format : Failed to append null value to gyro error code: %d\r\n",
        writer_status);
      goto error;
    }
  } else {
    /// Append accelero property
    writer_status = AzureIoTJSONWriter_AppendPropertyName(&bmi270_writer,
                                                          (const uint8_t *)JSON_PROPERTY_ACCELERO,
                                                          strlen(
                                                            JSON_PROPERTY_ACCELERO));
    if (writer_status != eAzureIoTSuccess) {
      printf(
        "\r\nsl_convert_bmi270_reading_to_json_format : Failed to append accelerometer property error code: %d\r\n",
        writer_status);
      goto error;
    }

    /// Append start of array
    writer_status = AzureIoTJSONWriter_AppendBeginArray(&bmi270_writer);
    if (writer_status != eAzureIoTSuccess) {
      printf(
        "\r\nsl_convert_bmi270_reading_to_json_format : Failed to append begin of array for accelerometer error code: %d\r\n",
        writer_status);
      goto error;
    }

    /// Append values inside array
    for (uint8_t index = 0; index < MAX_ACCLEROMETER_VALUES_SIZE; ++index) {
      writer_status = AzureIoTJSONWriter_AppendDouble(&bmi270_writer,
                                                      sensor_data_queue_reading->imu_data.accelerometer[
                                                        index],
                                                      JSON_DOUBLE_FRACTION_MAX_SIZE);
      if (writer_status != eAzureIoTSuccess) {
        printf(
          "\r\nsl_convert_bmi270_reading_to_json_format : Failed to append accelerometer data in array error code: %d\r\n",
          writer_status);
        goto error;
      }
    }

    /// Append endof array
    writer_status = AzureIoTJSONWriter_AppendEndArray(&bmi270_writer);
    if (writer_status != eAzureIoTSuccess) {
      printf(
        "\r\nsl_convert_bmi270_reading_to_json_format : Failed to append end of array for accelerometer error code: %d\r\n",
        writer_status);
      goto error;
    }

    /// Append gyro property
    writer_status = AzureIoTJSONWriter_AppendPropertyName(&bmi270_writer,
                                                          (const uint8_t *)JSON_PROPERTY_GYRO,
                                                          strlen(
                                                            JSON_PROPERTY_GYRO));
    if (writer_status != eAzureIoTSuccess) {
      printf(
        "\r\nsl_convert_bmi270_reading_to_json_format : Failed to append gyroscope property error code: %d\r\n",
        writer_status);
      goto error;
    }

    /// Append start of an array
    writer_status = AzureIoTJSONWriter_AppendBeginArray(&bmi270_writer);
    if (writer_status != eAzureIoTSuccess) {
      printf(
        "\r\nsl_convert_bmi270_reading_to_json_format : Failed to append start of array for gyroscope error code: %d\r\n",
        writer_status);
      goto error;
    }

    /// Append gyroscope values inside array
    for (uint8_t index = 0; index < MAX_GYROSCOPE_VALUES_SIZE; ++index) {
      writer_status = AzureIoTJSONWriter_AppendDouble(&bmi270_writer,
                                                      sensor_data_queue_reading->imu_data.gyroscope[
                                                        index],
                                                      JSON_DOUBLE_FRACTION_MAX_SIZE);
      if (writer_status != eAzureIoTSuccess) {
        printf(
          "\r\nsl_convert_bmi270_reading_to_json_format : Failed to append gyroscope data in array error code: %d\r\n",
          writer_status);
        goto error;
      }
    }

    /// Append end of array
    writer_status = AzureIoTJSONWriter_AppendEndArray(&bmi270_writer);
    if (writer_status != eAzureIoTSuccess) {
      printf(
        "\r\nsl_convert_bmi270_reading_to_json_format : Failed to append end of array for gyroscope error code: %d\r\n",
        writer_status);
      goto error;
    }
  }

  /// Append close of main JSON object
  writer_status = AzureIoTJSONWriter_AppendEndObject(&bmi270_writer);
  if (writer_status != eAzureIoTSuccess) {
    printf(
      "\r\nsl_convert_bmi270_reading_to_json_format : Failed to append end of main object error code: %d\r\n",
      writer_status);
    goto error;
  }

  /// Update length of MQTT buffer
  bmi270_json_data.mqtt_buffer_len = AzureIoTJSONWriter_GetBytesUsed(
    &bmi270_writer);

  /// Acquire MQTT data queue mutex and send data to MQTT data queue
  if (pdTRUE
      == xSemaphoreTake(sl_get_wifi_asset_tracking_resource()->
                        mqtt_package_queue_mutex_handler,
                        portMAX_DELAY)) {
    if (pdTRUE
        == xQueueSend(sl_get_wifi_asset_tracking_resource()->
                      mqtt_package_queue_handler,
                      &bmi270_json_data, 0)) {
#if DEMO_CONFIG_DEBUG_LOGS
      printf(
        "\r\nsl_convert_bmi270_reading_to_json_format : JSON format data is sent to the MQTT data queue\r\n");
#endif /// < DEMO_CONFIG_DEBUG_LOGS
    }
    xSemaphoreGive(
      sl_get_wifi_asset_tracking_resource()->mqtt_package_queue_mutex_handler);
  }

  return SL_STATUS_OK;
  error:
  return SL_STATUS_FAIL;
}

/******************************************************************************
 *  Callback function to convert gnss data format to json data format.
 *****************************************************************************/
sl_status_t sl_convert_gnss_reading_to_json_format(
  sl_wifi_asset_tracking_sensor_queue_data_t *sensor_data_queue_reading)
{
  AzureIoTJSONWriter_t gnss_writer;
  sl_wifi_asset_tracking_mqtt_package_queue_data_t gnss_json_data;
  AzureIoTResult_t writer_status;

  /// Initialize the JSON writer
  writer_status = AzureIoTJSONWriter_Init(&gnss_writer,
                                          gnss_json_data.mqtt_buffer,
                                          sizeof(gnss_json_data.mqtt_buffer));
  if (writer_status != eAzureIoTSuccess) {
    printf(
      "\r\nsl_convert_gnss_reading_to_json_format : Failed to initialize JSON writer error code: %d\r\n",
      writer_status);
    goto error;
  }

  /// Construct the JSON message
  writer_status = AzureIoTJSONWriter_AppendBeginObject(&gnss_writer);
  if (writer_status != eAzureIoTSuccess) {
    printf(
      "\r\nsl_convert_gnss_reading_to_json_format : Append begin main object failed with error code: %d\r\n",
      writer_status);
    goto error;
  }

  /// Append gps msgtype
  writer_status = AzureIoTJSONWriter_AppendPropertyWithStringValue(&gnss_writer,
                                                                   (const
                                                                    uint8_t *)JSON_PROPERTY_MSGTYPE,
                                                                   strlen(
                                                                     JSON_PROPERTY_MSGTYPE),
                                                                   (const
                                                                    uint8_t *)JSON_PROPERTY_GPS,
                                                                   strlen(
                                                                     JSON_PROPERTY_GPS));
  if (writer_status != eAzureIoTSuccess) {
    printf(
      "\r\nsl_convert_gnss_reading_to_json_format : Append msgtype:gps object failed with error code: %d\r\n",
      writer_status);
    goto error;
  }

  /// Append time-stamp
  writer_status = AzureIoTJSONWriter_AppendPropertyWithStringValue(&gnss_writer,
                                                                   (const
                                                                    uint8_t *)JSON_PROPERTY_TIMESTAMP,
                                                                   strlen(
                                                                     JSON_PROPERTY_TIMESTAMP),
                                                                   (const
                                                                    uint8_t *)sensor_data_queue_reading->time_stamp,
                                                                   strlen((char
                                                                           *)
                                                                          sensor_data_queue_reading
                                                                          ->
                                                                          time_stamp));
  if (writer_status != eAzureIoTSuccess) {
    printf(
      "\r\nsl_convert_gnss_reading_to_json_format : Append time-stamp object failed with error code: %d\r\n",
      writer_status);
    goto error;
  }

  /// If sensor data is not available then send null
  if (!sensor_data_queue_reading->is_sensor_data_available) {
    /// Append gps property
    writer_status = AzureIoTJSONWriter_AppendPropertyName(&gnss_writer,
                                                          (const uint8_t *)JSON_PROPERTY_GPS,
                                                          strlen(
                                                            JSON_PROPERTY_GPS));
    if (writer_status != eAzureIoTSuccess) {
      printf(
        "\r\nsl_convert_gnss_reading_to_json_format : Append gps object failed with error code: %d\r\n",
        writer_status);
      goto error;
    }

    /// Append null value to gps object
    writer_status = AzureIoTJSONWriter_AppendNull(&gnss_writer);
    if (writer_status != eAzureIoTSuccess) {
      printf(
        "\r\nsl_convert_gnss_reading_to_json_format : Append null value to gps object failed with error code: %d\r\n",
        writer_status);
      goto error;
    }
  } else {
    /// Append gps property
    writer_status = AzureIoTJSONWriter_AppendPropertyName(&gnss_writer,
                                                          (const uint8_t *)JSON_PROPERTY_GPS,
                                                          strlen(
                                                            JSON_PROPERTY_GPS));
    if (writer_status != eAzureIoTSuccess) {
      printf(
        "\r\nsl_convert_gnss_reading_to_json_format : Append gps object failed with error code: %d\r\n",
        writer_status);
      goto error;
    }

    /// Append begin of gps object
    writer_status = AzureIoTJSONWriter_AppendBeginObject(&gnss_writer);
    if (writer_status != eAzureIoTSuccess) {
      printf(
        "\r\nsl_convert_gnss_reading_to_json_format : Append begin object for gps value failed with error code: %d\r\n",
        writer_status);
      goto error;
    }

    /// Append latitude property
    writer_status = AzureIoTJSONWriter_AppendPropertyWithDoubleValue(
      &gnss_writer,
      (const
       uint8_t *)JSON_PROPERTY_LATITUDE,
      strlen(
        JSON_PROPERTY_LATITUDE),
      sensor_data_queue_reading->gnss_data.latitude,
      JSON_DOUBLE_FRACTION_MAX_SIZE);
    if (writer_status != eAzureIoTSuccess) {
      printf(
        "\r\nsl_convert_gnss_reading_to_json_format : Append latitude failed with error code: %d\r\n",
        writer_status);
      goto error;
    }

    /// Append longitude property
    writer_status = AzureIoTJSONWriter_AppendPropertyWithDoubleValue(
      &gnss_writer,
      (const
       uint8_t *)JSON_PROPERTY_LONGITUDE,
      strlen(
        JSON_PROPERTY_LONGITUDE),
      sensor_data_queue_reading->gnss_data.longitude,
      JSON_DOUBLE_FRACTION_MAX_SIZE);
    if (writer_status != eAzureIoTSuccess) {
      printf(
        "\r\nsl_convert_gnss_reading_to_json_format : Append longitude failed with error code: %d\r\n",
        writer_status);
      goto error;
    }

    /// Append altitude property
    writer_status = AzureIoTJSONWriter_AppendPropertyWithDoubleValue(
      &gnss_writer,
      (const
       uint8_t *)JSON_PROPERTY_ALTITUDE,
      strlen(
        JSON_PROPERTY_ALTITUDE),
      sensor_data_queue_reading->gnss_data.altitude,
      JSON_DOUBLE_FRACTION_MAX_SIZE);
    if (writer_status != eAzureIoTSuccess) {
      printf(
        "\r\nsl_convert_gnss_reading_to_json_format : Append altitude failed with error code: %d\r\n",
        writer_status);
      goto error;
    }

    /// Append satellites property
    writer_status = AzureIoTJSONWriter_AppendPropertyWithInt32Value(
      &gnss_writer,
      (const
       uint8_t *)JSON_PROPERTY_SATELLITE,
      strlen(
        JSON_PROPERTY_SATELLITE),
      sensor_data_queue_reading->gnss_data.no_of_satellites);
    if (writer_status != eAzureIoTSuccess) {
      printf(
        "\r\nsl_convert_gnss_reading_to_json_format : Append satellites failed with error code: %d\r\n",
        writer_status);
      goto error;
    }

    /// Append close of gps object
    writer_status = AzureIoTJSONWriter_AppendEndObject(&gnss_writer);
    if (writer_status != eAzureIoTSuccess) {
      printf(
        "\r\nsl_convert_gnss_reading_to_json_format : Append end gps object failed with error code: %d\r\n",
        writer_status);
      goto error;
    }
  }

  /// Append close of main object
  writer_status = AzureIoTJSONWriter_AppendEndObject(&gnss_writer);
  if (writer_status != eAzureIoTSuccess) {
    printf(
      "\r\nsl_convert_gnss_reading_to_json_format : Append end main object failed with error code: %d\r\n",
      writer_status);
    goto error;
  }

  /// Update length of MQTT buffer
  gnss_json_data.mqtt_buffer_len =
    AzureIoTJSONWriter_GetBytesUsed(&gnss_writer);

  /// Acquire MQTT data queue mutex and send data to MQTT data queue
  if (pdTRUE
      == xSemaphoreTake(sl_get_wifi_asset_tracking_resource()->
                        mqtt_package_queue_mutex_handler,
                        portMAX_DELAY)) {
    if (pdTRUE
        == xQueueSend(sl_get_wifi_asset_tracking_resource()->
                      mqtt_package_queue_handler,
                      &gnss_json_data, 0)) {
#if DEMO_CONFIG_DEBUG_LOGS
      printf(
        "\r\nsl_convert_gnss_reading_to_json_format : JSON format data is sent to the MQTT data queue\r\n");
#endif /// < DEMO_CONFIG_DEBUG_LOGS
    }
    xSemaphoreGive(
      sl_get_wifi_asset_tracking_resource()->mqtt_package_queue_mutex_handler);
  }

  return SL_STATUS_OK;
  error:
  return SL_STATUS_FAIL;
}

/******************************************************************************
 *  Callback function to convert si7021 data format to json data format.
 *****************************************************************************/
sl_status_t sl_convert_si7021_reading_to_json_format(
  sl_wifi_asset_tracking_sensor_queue_data_t *sensor_data_queue_reading)
{
  AzureIoTJSONWriter_t si7021_writer;
  sl_wifi_asset_tracking_mqtt_package_queue_data_t si7021_json_data;
  AzureIoTResult_t writer_status;

  /// Initialize the JSON writer
  writer_status = AzureIoTJSONWriter_Init(&si7021_writer,
                                          si7021_json_data.mqtt_buffer,
                                          sizeof(si7021_json_data.mqtt_buffer));
  if (writer_status != eAzureIoTSuccess) {
    printf(
      "\r\nsl_convert_si7021_reading_to_json_format : JSON Writer Initialization Failed with error code: %d\r\n",
      writer_status);
    goto error;
  }

  /// Append start of main object
  writer_status = AzureIoTJSONWriter_AppendBeginObject(&si7021_writer);
  if (writer_status != eAzureIoTSuccess) {
    printf(
      "\r\nsl_convert_si7021_reading_to_json_format : Append begin main object failed with error code: %d\r\n",
      writer_status);
    goto error;
  }

  /// Append msgtype heat property
  writer_status = AzureIoTJSONWriter_AppendPropertyWithStringValue(
    &si7021_writer,
    (const
     uint8_t *)JSON_PROPERTY_MSGTYPE,
    strlen(
      JSON_PROPERTY_MSGTYPE),
    (const
     uint8_t *)JSON_PROPERTY_HEAT,
    strlen(
      JSON_PROPERTY_HEAT));
  if (writer_status != eAzureIoTSuccess) {
    printf(
      "\r\nsl_convert_si7021_reading_to_json_format : Append msgtype Failed with error code: %d\r\n",
      writer_status);
    goto error;
  }

  /// Append time-stamp property
  writer_status = AzureIoTJSONWriter_AppendPropertyWithStringValue(
    &si7021_writer,
    (const
     uint8_t *)JSON_PROPERTY_TIMESTAMP,
    strlen(
      JSON_PROPERTY_TIMESTAMP),
    (const
     uint8_t *)sensor_data_queue_reading->time_stamp,
    strlen((char
            *)sensor_data_queue_reading->time_stamp));
  if (writer_status != eAzureIoTSuccess) {
    printf(
      "\r\nsl_convert_si7021_reading_to_json_format : Append time-stamp Failed with error code: %d\r\n",
      writer_status);
    goto error;
  }

  /// If sensor data is not available then send null
  if (!sensor_data_queue_reading->is_sensor_data_available) {
    /// Append heat property
    writer_status = AzureIoTJSONWriter_AppendPropertyName(&si7021_writer,
                                                          (const uint8_t *)JSON_PROPERTY_HEAT,
                                                          strlen(
                                                            JSON_PROPERTY_HEAT));
    if (writer_status != eAzureIoTSuccess) {
      printf(
        "\r\nsl_convert_si7021_reading_to_json_format : Append heat property Failed with error code: %d\r\n",
        writer_status);
      goto error;
    }

    /// Append null value to heat property
    writer_status = AzureIoTJSONWriter_AppendNull(&si7021_writer);
    if (writer_status != eAzureIoTSuccess) {
      printf(
        "\r\nsl_convert_si7021_reading_to_json_format : Append null value Failed with error code: %d\r\n",
        writer_status);
      goto error;
    }
  } else {
    /// Append heat property
    writer_status = AzureIoTJSONWriter_AppendPropertyName(&si7021_writer,
                                                          (const uint8_t *)JSON_PROPERTY_HEAT,
                                                          strlen(
                                                            JSON_PROPERTY_HEAT));
    if (writer_status != eAzureIoTSuccess) {
      printf(
        "\r\nsl_convert_si7021_reading_to_json_format : Append heat property Failed with error code: %d\r\n",
        writer_status);
      goto error;
    }

    /// Append begin of heat property
    writer_status = AzureIoTJSONWriter_AppendBeginObject(&si7021_writer);
    if (writer_status != eAzureIoTSuccess) {
      printf(
        "\r\nsl_convert_si7021_reading_to_json_format : Append begin heat object failed with error code: %d\r\n",
        writer_status);
      goto error;
    }

    /// Append temperature object within heat property
    writer_status = AzureIoTJSONWriter_AppendPropertyName(&si7021_writer,
                                                          (const uint8_t *)JSON_PROPERTY_TEMPERATURE,
                                                          strlen(
                                                            JSON_PROPERTY_TEMPERATURE));
    if (writer_status != eAzureIoTSuccess) {
      printf(
        "\r\nsl_convert_si7021_reading_to_json_format : Append temperature property Failed with error code: %d\r\n",
        writer_status);
      goto error;
    }

    /// Append temperature begin object
    writer_status = AzureIoTJSONWriter_AppendBeginObject(&si7021_writer);
    if (writer_status != eAzureIoTSuccess) {
      printf(
        "\r\nsl_convert_si7021_reading_to_json_format : Append begin temperature object failed with error code: %d\r\n",
        writer_status);
      goto error;
    }

    /// Append value property within temperature
    writer_status = AzureIoTJSONWriter_AppendPropertyWithDoubleValue(
      &si7021_writer,
      (const
       uint8_t *)JSON_PROPERTY_VALUE,
      strlen(
        JSON_PROPERTY_VALUE),
      sensor_data_queue_reading->temp_rh_data.temperature,
      JSON_DOUBLE_FRACTION_MAX_SIZE);
    if (writer_status != eAzureIoTSuccess) {
      printf(
        "\r\nsl_convert_si7021_reading_to_json_format : Append temperature value Failed with error code: %d\r\n",
        writer_status);
      goto error;
    }

    /// Append unit property within temperature
    writer_status = AzureIoTJSONWriter_AppendPropertyWithStringValue(
      &si7021_writer,
      (const
       uint8_t *)JSON_PROPERTY_UNIT,
      strlen(
        JSON_PROPERTY_UNIT),
      (const
       uint8_t *)sensor_data_queue_reading->temp_rh_data.temperature_unit,
      strlen((
               char *)(sensor_data_queue_reading->temp_rh_data.temperature_unit)));
    if (writer_status != eAzureIoTSuccess) {
      printf(
        "\r\nsl_convert_si7021_reading_to_json_format : Append temperature unit Failed with error code: %d\r\n",
        writer_status);
      goto error;
    }

    /// Append end of temperature object
    writer_status = AzureIoTJSONWriter_AppendEndObject(&si7021_writer);
    if (writer_status != eAzureIoTSuccess) {
      printf(
        "\r\nsl_convert_si7021_reading_to_json_format : Append end temperature object failed with error code: %d\r\n",
        writer_status);
      goto error;
    }

    /// Append humidity property within heat object
    writer_status = AzureIoTJSONWriter_AppendPropertyWithDoubleValue(
      &si7021_writer,
      (const
       uint8_t *)JSON_PROPERTY_HUMIDITY,
      strlen(
        JSON_PROPERTY_HUMIDITY),
      sensor_data_queue_reading->temp_rh_data.relative_humidity,
      JSON_DOUBLE_FRACTION_MAX_SIZE);
    if (writer_status != eAzureIoTSuccess) {
      printf(
        "\r\nsl_convert_si7021_reading_to_json_format : Append humidity failed with error code: %d\r\n",
        writer_status);
      goto error;
    }

    /// Append end of heat object
    writer_status = AzureIoTJSONWriter_AppendEndObject(&si7021_writer);
    if (writer_status != eAzureIoTSuccess) {
      printf(
        "\r\nsl_convert_si7021_reading_to_json_format : Append end of heat object failed with error code: %d\r\n",
        writer_status);
      goto error;
    }
  }
  /// Append end of main JSON object
  writer_status = AzureIoTJSONWriter_AppendEndObject(&si7021_writer);
  if (writer_status != eAzureIoTSuccess) {
    printf(
      "\r\nsl_convert_si7021_reading_to_json_format : Append end JSON object failed with error code: %d\r\n",
      writer_status);
    goto error;
  }

  /// Update length of MQTT buffer
  si7021_json_data.mqtt_buffer_len = AzureIoTJSONWriter_GetBytesUsed(
    &si7021_writer);

  /// Acquire MQTT data queue mutex and send data to MQTT data queue
  if (pdTRUE
      == xSemaphoreTake(sl_get_wifi_asset_tracking_resource()->
                        mqtt_package_queue_mutex_handler,
                        portMAX_DELAY)) {
    if (pdTRUE
        == xQueueSend(sl_get_wifi_asset_tracking_resource()->
                      mqtt_package_queue_handler,
                      &si7021_json_data, 0)) {
#if DEMO_CONFIG_DEBUG_LOGS
      printf(
        "\r\nsl_convert_si7021_reading_to_json_format : JSON format data is sent to the MQTT data queue\r\n");
#endif /// < DEMO_CONFIG_DEBUG_LOGS
    }
    xSemaphoreGive(
      sl_get_wifi_asset_tracking_resource()->mqtt_package_queue_mutex_handler);
  }

  return SL_STATUS_OK;
  error:
  return SL_STATUS_FAIL;
}

/******************************************************************************
 *  Callback function to convert sensor data format to json data format.
 *****************************************************************************/
sl_status_t sl_convert_to_json_format(
  sl_wifi_asset_tracking_sensor_queue_data_t *sensor_data_queue_reading)
{
  sl_status_t status;
  switch (sensor_data_queue_reading->sensor_type) {
    case SL_TEMP_RH_SENSOR:
      status = sl_convert_si7021_reading_to_json_format(
        sensor_data_queue_reading);
      if (status != SL_STATUS_OK) {
        return SL_STATUS_FAIL;
      }

      break;

    case SL_IMU_SENSOR:
      status = sl_convert_bmi270_reading_to_json_format(
        sensor_data_queue_reading);
      if (status != SL_STATUS_OK) {
        return SL_STATUS_FAIL;
      }
      break;

    case SL_GNSS_RECEIVER:
      status =
        sl_convert_gnss_reading_to_json_format(sensor_data_queue_reading);
      if (status != SL_STATUS_OK) {
        return SL_STATUS_FAIL;
      }
      break;

    default:
      /// Nothing to do
      break;
  }
  return SL_STATUS_OK;
}

/******************************************************************************
 *  Callback function to convert sensor data format to json data format.
 *****************************************************************************/
void sl_json_data_converter_task()
{
  sl_wifi_asset_tracking_mqtt_package_queue_data_t mqtt_data_queue_reading;
  sl_wifi_asset_tracking_sensor_queue_data_t sensor_data_queue_reading;
  sl_status_t status;
  while (1) {
    /// Check if sensor data queue is empty
    if (QUEUE_EMPTY
        == uxQueueMessagesWaiting(sl_get_wifi_asset_tracking_resource()->
                                  sensor_data_queue_handler)) {
#if DEMO_CONFIG_DEBUG_LOGS
      printf(
        "\r\njson_task : suspend json task as sensor data queue is empty\r\n");
#endif /// < DEMO_CONFIG_DEBUG_LOGS
      vTaskSuspend(
        sl_get_wifi_asset_tracking_resource()->task_list.json_data_converter_task_handler);
    } else {
      /// Check whether MQTT data queue is full
      if (MAX_SIZE_OF_MQTT_PACKAGE_QUEUE
          == uxQueueMessagesWaiting(sl_get_wifi_asset_tracking_resource()->
                                    sensor_data_queue_handler)) {
        /// Acquire MQTT data queue mutex and recieve data from MQTT data queue
        if (pdTRUE
            == xSemaphoreTake(sl_get_wifi_asset_tracking_resource()->
                              mqtt_package_queue_mutex_handler,
                              portMAX_DELAY)) {
          xQueueReceive(
            sl_get_wifi_asset_tracking_resource()->mqtt_package_queue_handler,
            &mqtt_data_queue_reading,
            0);
          printf(
            "\r\njson_task : Data is released from the MQTT data queue as it is full\r\n");
        }

        xSemaphoreGive(
          sl_get_wifi_asset_tracking_resource()->mqtt_package_queue_mutex_handler);
      }

      /// Acquire sensor data queue mutex and receive data from sensor data queue
      if (pdTRUE
          == xSemaphoreTake(sl_get_wifi_asset_tracking_resource()->
                            sensor_data_queue_mutex_handler,
                            portMAX_DELAY)) {
        xQueueReceive(
          sl_get_wifi_asset_tracking_resource()->sensor_data_queue_handler,
          &sensor_data_queue_reading,
          0);
#if DEMO_CONFIG_DEBUG_LOGS
        printf(
          "\r\njson_task : Sensor data is received from the sensor data queue for conversion to JSON format\r\n");
#endif /// < DEMO_CONFIG_DEBUG_LOGS
      }

      xSemaphoreGive(
        sl_get_wifi_asset_tracking_resource()->sensor_data_queue_mutex_handler);

      /// Convert sensor data into json format
      status = sl_convert_to_json_format(&sensor_data_queue_reading);

      if (SL_STATUS_OK == status) {
        /// Check whether Azure cloud communication task is suspended
        if (eSuspended
            == eTaskGetState(sl_get_wifi_asset_tracking_resource()->task_list.
                             azure_cloud_communication_task_handler)) {
          vTaskResume(
            sl_get_wifi_asset_tracking_resource()->task_list.azure_cloud_communication_task_handler);
        }
      }
    }
  }
}

/******************************************************************************
 * Function to send new session JSON message to MQTT package queue.
 ******************************************************************************/
sl_status_t sl_json_send_new_session_message()
{
  sl_wifi_asset_tracking_mqtt_package_queue_data_t new_session_message;
  AzureIoTJSONWriter_t new_session_writer;
  AzureIoTResult_t writer_status;
  uint8_t timestamp_buff[JSON_MAX_TIMESTAMP_BUFF_SIZE];

  /// If failed to fetch time-stamp then discard the packet
  if (SL_STATUS_OK != sl_json_get_timestamp(timestamp_buff)) {
    printf(
      "\r\nsl_json_send_new_session_message : failed to fetch time-stamp, discarding the packet\r\n");
    goto error;
  }

  /// Initialize the JSON writer
  writer_status = AzureIoTJSONWriter_Init(&new_session_writer,
                                          new_session_message.mqtt_buffer,
                                          sizeof(new_session_message.mqtt_buffer));

  if (writer_status != eAzureIoTSuccess) {
    printf(
      "\r\nsl_json_send_new_session_message : Failed to initialize JSON writer for keep alive message error code: %d\r\n",
      writer_status);
    goto error;
  }

  /// Construct the JSON message - append begin object
  writer_status = AzureIoTJSONWriter_AppendBeginObject(&new_session_writer);
  if (writer_status != eAzureIoTSuccess) {
    printf(
      "\r\nsl_json_send_new_session_message : Failed to append begin object error code: %d\r\n",
      writer_status);
    goto error;
  }

  /// Append message type property
  writer_status = AzureIoTJSONWriter_AppendPropertyWithStringValue(
    &new_session_writer,
    (const
     uint8_t *)JSON_PROPERTY_MSGTYPE,
    strlen(
      JSON_PROPERTY_MSGTYPE),
    (const
     uint8_t *)JSON_PROPERTY_SESSION,
    strlen(
      JSON_PROPERTY_SESSION));
  if (writer_status != eAzureIoTSuccess) {
    printf(
      "\r\nsl_json_send_new_session_message : Failed to append message type - session error code: %d\r\n",
      writer_status);
    goto error;
  }

  /// Append time-stamp property
  writer_status = AzureIoTJSONWriter_AppendPropertyWithStringValue(
    &new_session_writer,
    (const
     uint8_t *)JSON_PROPERTY_TIMESTAMP,
    strlen(
      JSON_PROPERTY_TIMESTAMP),
    (const
     uint8_t *)timestamp_buff,
    strlen((char
            *)timestamp_buff));
  if (writer_status != eAzureIoTSuccess) {
    printf(
      "\r\nsl_json_send_new_session_message : Failed to append time-stamp property error code: %d\r\n",
      writer_status);
    goto error;
  }

  /// Append session type property
  writer_status = AzureIoTJSONWriter_AppendPropertyWithStringValue(
    &new_session_writer,
    (const
     uint8_t *)JSON_PROPERTY_SESSION,
    strlen(
      JSON_PROPERTY_SESSION),
    (const
     uint8_t *)JSON_PROPERTY_NEW_SESSION_TYPE,
    strlen(
      JSON_PROPERTY_NEW_SESSION_TYPE));
  if (writer_status != eAzureIoTSuccess) {
    printf(
      "\r\nsl_json_send_new_session_message : Failed to append session type property error code: %d\r\n",
      writer_status);
    goto error;
  }

  /// Append close object
  writer_status = AzureIoTJSONWriter_AppendEndObject(&new_session_writer);
  if (writer_status != eAzureIoTSuccess) {
    printf(
      "\r\nsl_json_send_new_session_message : Failed to append end of main object for keep-alive property error code: %d\r\n",
      writer_status);
    goto error;
  }

  /// Update length of MQTT buffer
  new_session_message.mqtt_buffer_len = AzureIoTJSONWriter_GetBytesUsed(
    &new_session_writer);

  /// Acquire MQTT data queue mutex
  if (pdTRUE
      == xSemaphoreTake(sl_get_wifi_asset_tracking_resource()->
                        mqtt_package_queue_mutex_handler,
                        portMAX_DELAY)) {
    /// send data to MQTT data queue - If queue is full no need to receive and send it again
    xQueueSend(
      sl_get_wifi_asset_tracking_resource()->mqtt_package_queue_handler,
      &new_session_message,
      0);
    xSemaphoreGive(
      sl_get_wifi_asset_tracking_resource()->mqtt_package_queue_mutex_handler);
  }
#if DEMO_CONFIG_DEBUG_LOGS
  printf(
    "\r\nsl_json_send_new_session_message : JSON format data is sent to the MQTT data queue\r\n");
#endif /// < DEMO_CONFIG_DEBUG_LOGS

  return SL_STATUS_OK;
  error:
  return SL_STATUS_FAIL;
}

/*****************************************************************************
 * Function to send wi-fi JSON message to MQTT package queue.
 ******************************************************************************/
sl_status_t sl_json_send_wifi_message()
{
  sl_wifi_asset_tracking_mqtt_package_queue_data_t wifi_data;
  AzureIoTJSONWriter_t wifi_data_writer;
  AzureIoTResult_t writer_status;
  uint8_t timestamp_buff[JSON_MAX_TIMESTAMP_BUFF_SIZE];
  uint8_t mac_address_buff[JSON_MAX_MAC_ADDR_BUFF_SIZE];
  int32_t rssi;

  /// If failed to fetch time-stamp then return failure
  if (SL_STATUS_OK != sl_json_get_timestamp(timestamp_buff)) {
    printf(
      "\r\nsl_json_send_wifi_message : failed to fetch time-stamp, discarding the packet\r\n");
    goto error;
  }

  /// If failed to fetch RSSI then return failure
  if (SL_STATUS_OK != sl_get_wifi_rssi(&rssi)) {
    printf(
      "\r\nsl_json_send_wifi_message : failed to fetch RSSI, discarding the packet\r\n");
    goto wifi_failure;
  }

  /// If failed to fetch MAC address then return failure
  if (SL_STATUS_OK != sl_get_wifi_mac_address(mac_address_buff)) {
    printf(
      "\r\nsl_json_send_wifi_message : failed to fetch MAC Address, discarding the packet\r\n");
    goto wifi_failure;
  }

  /// Initialize the JSON writer
  writer_status = AzureIoTJSONWriter_Init(&wifi_data_writer,
                                          wifi_data.mqtt_buffer,
                                          sizeof(wifi_data.mqtt_buffer));

  if (writer_status != eAzureIoTSuccess) {
    printf(
      "\r\nsl_json_send_wifi_message : Failed to initialize JSON writer for wi-fi data message error code: %d\r\n",
      writer_status);
    goto error;
  }

  /// Construct the JSON message - append begin object
  writer_status = AzureIoTJSONWriter_AppendBeginObject(&wifi_data_writer);
  if (writer_status != eAzureIoTSuccess) {
    printf(
      "\r\nsl_json_send_wifi_message : Append main begin object failed for wi-fi data. error code: %d\r\n",
      writer_status);
    goto error;
  }

  /// Append message type property
  writer_status = AzureIoTJSONWriter_AppendPropertyWithStringValue(
    &wifi_data_writer,
    (const
     uint8_t *)JSON_PROPERTY_MSGTYPE,
    strlen(
      JSON_PROPERTY_MSGTYPE),
    (const
     uint8_t *)JSON_PROPERTY_WIFI,
    strlen(
      JSON_PROPERTY_WIFI));
  if (writer_status != eAzureIoTSuccess) {
    printf(
      "\r\nsl_json_send_wifi_message : Failed to append message type wi-fi error code: %d\r\n",
      writer_status);
    goto error;
  }

  /// Append time-stamp property
  writer_status = AzureIoTJSONWriter_AppendPropertyWithStringValue(
    &wifi_data_writer,
    (const
     uint8_t *)JSON_PROPERTY_TIMESTAMP,
    strlen(
      JSON_PROPERTY_TIMESTAMP),
    (const
     uint8_t *)timestamp_buff,
    strlen((char
            *)timestamp_buff));
  if (writer_status != eAzureIoTSuccess) {
    printf(
      "\r\nsl_json_send_wifi_message : Failed to append time-stamp property error code: %d\r\n",
      writer_status);
    goto error;
  }

  /// Append wi-fi object
  writer_status = AzureIoTJSONWriter_AppendPropertyName(&wifi_data_writer,
                                                        (const uint8_t *)JSON_PROPERTY_WIFI,
                                                        strlen(
                                                          JSON_PROPERTY_WIFI));
  if (writer_status != eAzureIoTSuccess) {
    printf(
      "\r\nsl_json_send_wifi_message : Failed to append wi-fi property error code: %d\r\n",
      writer_status);
    goto error;
  }

  /// Append begin object
  writer_status = AzureIoTJSONWriter_AppendBeginObject(&wifi_data_writer);
  if (writer_status != eAzureIoTSuccess) {
    printf(
      "\r\nsl_json_send_wifi_message : Failed to append begin object for wi-fi property error code: %d\r\n",
      writer_status);
    goto error;
  }

  /// Append MAC address key value pair
  writer_status = AzureIoTJSONWriter_AppendPropertyWithStringValue(
    &wifi_data_writer,
    (const
     uint8_t *)JSON_PROPERTY_MACID,
    strlen(
      JSON_PROPERTY_MACID),
    (const
     uint8_t *)mac_address_buff,
    strlen((char
            *)mac_address_buff));
  if (writer_status != eAzureIoTSuccess) {
    printf(
      "\r\nsl_json_send_wifi_message : Failed to append macid key value pair error code: %d\r\n",
      writer_status);
    goto error;
  }

  /// Append SSID key value pair
  writer_status = AzureIoTJSONWriter_AppendPropertyWithStringValue(
    &wifi_data_writer,
    (const
     uint8_t *)JSON_PROPERTY_SSID,
    strlen(
      JSON_PROPERTY_SSID),
    (const
     uint8_t *)DEFAULT_WIFI_CLIENT_PROFILE_SSID,
    strlen(
      DEFAULT_WIFI_CLIENT_PROFILE_SSID));
  if (writer_status != eAzureIoTSuccess) {
    printf(
      "\r\nsl_json_send_wifi_message : Failed to append SSID key value pair error code: %d\r\n",
      writer_status);
    goto error;
  }

  /// Append RSSI key value pair
  writer_status = AzureIoTJSONWriter_AppendPropertyWithInt32Value(
    &wifi_data_writer,
    (const uint8_t
     *)JSON_PROPERTY_RSSI,
    strlen(
      JSON_PROPERTY_RSSI),
    rssi);
  if (writer_status != eAzureIoTSuccess) {
    printf(
      "\r\nsl_json_send_wifi_message : Failed to append RSSI key value pair error code: %d\r\n",
      writer_status);
    goto error;
  }

  /// append close inner object
  writer_status = AzureIoTJSONWriter_AppendEndObject(&wifi_data_writer);
  if (writer_status != eAzureIoTSuccess) {
    printf(
      "\r\nsl_json_send_wifi_message : Failed to append inner close object error code: %d\r\n",
      writer_status);
    goto error;
  }

  /// append close main object
  writer_status = AzureIoTJSONWriter_AppendEndObject(&wifi_data_writer);
  if (writer_status != eAzureIoTSuccess) {
    printf(
      "\r\nsl_json_send_wifi_message : Failed to append main close object error code: %d\r\n",
      writer_status);
    goto error;
  }

  /// Update length of MQTT buffer
  wifi_data.mqtt_buffer_len =
    AzureIoTJSONWriter_GetBytesUsed(&wifi_data_writer);

  /// Acquire MQTT data queue mutex
  if (pdTRUE
      == xSemaphoreTake(sl_get_wifi_asset_tracking_resource()->
                        mqtt_package_queue_mutex_handler,
                        portMAX_DELAY)) {
    /// send data to MQTT data queue - If queue is full the receive the data and send it again
    if (errQUEUE_FULL
        == xQueueSend(sl_get_wifi_asset_tracking_resource()->
                      mqtt_package_queue_handler,
                      &wifi_data, 0)) {
      sl_wifi_asset_tracking_mqtt_package_queue_data_t mqtt_data;
      xQueueReceive(
        sl_get_wifi_asset_tracking_resource()->mqtt_package_queue_handler,
        &mqtt_data,
        0);
      printf(
        "\r\nsl_json_send_wifi_message : Data is released from the MQTT data queue as it is full\r\n");
      xQueueSend(
        sl_get_wifi_asset_tracking_resource()->mqtt_package_queue_handler,
        &wifi_data,
        0);
    }

    xSemaphoreGive(
      sl_get_wifi_asset_tracking_resource()->mqtt_package_queue_mutex_handler);
  }
#if DEMO_CONFIG_DEBUG_LOGS
  printf(
    "\r\nsl_json_send_wifi_message : Wi-Fi JSON format data is sent to the MQTT data queue\r\n");
#endif /// < DEMO_CONFIG_DEBUG_LOGS

  return SL_STATUS_OK;
  error:
  return SL_STATUS_FAIL;
  wifi_failure:
  return SL_STATUS_WIFI_CONNECTION_LOST;
}

/*****************************************************************************
 * Function to send keep alive JSON message to MQTT package queue.
 ******************************************************************************/
sl_status_t sl_json_send_keep_alive_message()
{
  sl_wifi_asset_tracking_mqtt_package_queue_data_t keep_alive_data;
  AzureIoTJSONWriter_t keep_alive_writer;
  AzureIoTResult_t writer_status;
  uint8_t timestamp_buff[JSON_MAX_TIMESTAMP_BUFF_SIZE];

  /// If failed to fetch time-stamp then discard the packet
  if (SL_STATUS_OK != sl_json_get_timestamp(timestamp_buff)) {
    printf(
      "\r\nsl_json_send_keep_alive_message : failed to fetch time-stamp, discarding the packet\r\n");
    goto error;
  }

  /// Initialize the JSON writer
  writer_status = AzureIoTJSONWriter_Init(&keep_alive_writer,
                                          keep_alive_data.mqtt_buffer,
                                          sizeof(keep_alive_data.mqtt_buffer));

  if (writer_status != eAzureIoTSuccess) {
    printf(
      "\r\nsl_json_send_keep_alive_message : Failed to initialize JSON writer for keep alive message error code: %d\r\n",
      writer_status);
    goto error;
  }

  /// Construct the JSON message - append begin object
  writer_status = AzureIoTJSONWriter_AppendBeginObject(&keep_alive_writer);
  if (writer_status != eAzureIoTSuccess) {
    printf(
      "\r\nsl_json_send_keep_alive_message : Failed to append begin object error code: %d\r\n",
      writer_status);
    goto error;
  }

  /// Append message type property
  writer_status = AzureIoTJSONWriter_AppendPropertyWithStringValue(
    &keep_alive_writer,
    (const
     uint8_t *)JSON_PROPERTY_MSGTYPE,
    strlen(
      JSON_PROPERTY_MSGTYPE),
    (const
     uint8_t *)JSON_PROPERTY_KEEP_ALIVE,
    strlen(
      JSON_PROPERTY_KEEP_ALIVE));
  if (writer_status != eAzureIoTSuccess) {
    printf(
      "\r\nsl_json_send_keep_alive_message : Failed to append message type - keep-alive error code: %d\r\n",
      writer_status);
    goto error;
  }

  writer_status = AzureIoTJSONWriter_AppendPropertyWithStringValue(
    &keep_alive_writer,
    (const
     uint8_t *)JSON_PROPERTY_TIMESTAMP,
    strlen(
      JSON_PROPERTY_TIMESTAMP),
    (const
     uint8_t *)timestamp_buff,
    strlen((char
            *)timestamp_buff));
  if (writer_status != eAzureIoTSuccess) {
    printf(
      "\r\nsl_json_send_keep_alive_message : Failed to append time-stamp property error code: %d\r\n",
      writer_status);
    goto error;
  }

  /// Append keep-alive property
  writer_status = AzureIoTJSONWriter_AppendPropertyWithStringValue(
    &keep_alive_writer,
    (const
     uint8_t *)JSON_PROPERTY_KEEP_ALIVE,
    strlen(
      JSON_PROPERTY_KEEP_ALIVE),
    (const
     uint8_t *)JSON_PROPERTY_KEEP_ALIVE_VALUE,
    strlen(
      JSON_PROPERTY_KEEP_ALIVE_VALUE));
  if (writer_status != eAzureIoTSuccess) {
    printf(
      "\r\nsl_json_send_keep_alive_message : Failed to append keep-alive property error code: %d\r\n",
      writer_status);
    goto error;
  }

  /// Append interval property
  writer_status = AzureIoTJSONWriter_AppendPropertyName(&keep_alive_writer,
                                                        (const uint8_t *)JSON_PROPERTY_INTERVAL,
                                                        strlen(
                                                          JSON_PROPERTY_INTERVAL));
  if (writer_status != eAzureIoTSuccess) {
    printf(
      "\r\nsl_json_send_keep_alive_message : Failed to append interval property error code: %d\r\n",
      writer_status);
    goto error;
  }

  /// Append start of array
  writer_status = AzureIoTJSONWriter_AppendBeginArray(&keep_alive_writer);
  if (writer_status != eAzureIoTSuccess) {
    printf(
      "\r\nsl_json_send_keep_alive_message : Failed to append begin of array for interval error code: %d\r\n",
      writer_status);
    goto error;
  }

  /// Append values inside array
  for (uint8_t index = 0; index < MAX_INTERVAL_VALUES_SIZE; ++index) {
    switch (index) {
      case 0:
        writer_status = AzureIoTJSONWriter_AppendInt32(&keep_alive_writer,
                                                       DEMO_CONFIG_WIFI_SAMPLING_INTERVAL);
        if (writer_status != eAzureIoTSuccess) {
          printf(
            "\r\nsl_json_send_keep_alive_message : Failed to append wi-fi interval  in array error code: %d\r\n",
            writer_status);
          goto error;
        }
        break;

      case 1:
        writer_status = AzureIoTJSONWriter_AppendInt32(&keep_alive_writer,
                                                       DEMO_CONFIG_TEMP_RH_SENSOR_SAMPLING_INTERVAL);
        if (writer_status != eAzureIoTSuccess) {
          printf(
            "\r\nsl_json_send_keep_alive_message : Failed to append temperature and RH sensor interval in array error code: %d\r\n",
            writer_status);
          goto error;
        }
        break;

      case 2:
        writer_status = AzureIoTJSONWriter_AppendInt32(&keep_alive_writer,
                                                       DEMO_CONFIG_IMU_SENSOR_SAMPLING_INTERVAL);
        if (writer_status != eAzureIoTSuccess) {
          printf(
            "\r\nsl_json_send_keep_alive_message : Failed to append IMU sensor interval in array error code: %d\r\n",
            writer_status);
          goto error;
        }
        break;

      case 3:
        writer_status = AzureIoTJSONWriter_AppendInt32(&keep_alive_writer,
                                                       DEMO_CONFIG_GNSS_RECEIVER_SAMPLING_INTERVAL);
        if (writer_status != eAzureIoTSuccess) {
          printf(
            "\r\nsl_json_send_keep_alive_message : Failed to append GNSS receiver interval in array error code: %d\r\n",
            writer_status);
          goto error;
        }
        break;
    }
  }

  /// Append endof array
  writer_status = AzureIoTJSONWriter_AppendEndArray(&keep_alive_writer);
  if (writer_status != eAzureIoTSuccess) {
    printf(
      "\r\nsl_json_send_keep_alive_message : Failed to append end of array for interval error code: %d\r\n",
      writer_status);
    goto error;
  }

  /// append close object
  writer_status = AzureIoTJSONWriter_AppendEndObject(&keep_alive_writer);
  if (writer_status != eAzureIoTSuccess) {
    printf(
      "\r\nsl_json_send_keep_alive_message : Failed to append end of main object for keep-alive property error code: %d\r\n",
      writer_status);
    goto error;
  }

  /// Update length of MQTT buffer
  keep_alive_data.mqtt_buffer_len = AzureIoTJSONWriter_GetBytesUsed(
    &keep_alive_writer);

  /// Acquire MQTT data queue mutex
  if (pdTRUE
      == xSemaphoreTake(sl_get_wifi_asset_tracking_resource()->
                        mqtt_package_queue_mutex_handler,
                        portMAX_DELAY)) {
    /// send data to MQTT data queue - If queue is full the receive the data and send it again
    if (errQUEUE_FULL
        == xQueueSend(sl_get_wifi_asset_tracking_resource()->
                      mqtt_package_queue_handler,
                      &keep_alive_data, 0)) {
      sl_wifi_asset_tracking_mqtt_package_queue_data_t mqtt_data;
      xQueueReceive(
        sl_get_wifi_asset_tracking_resource()->mqtt_package_queue_handler,
        &mqtt_data,
        0);
      printf(
        "\r\nsl_json_send_keep_alive_message : Data is released from the MQTT data queue as it is full\r\n");
      xQueueSend(
        sl_get_wifi_asset_tracking_resource()->mqtt_package_queue_handler,
        &keep_alive_data,
        0);
    }

    xSemaphoreGive(
      sl_get_wifi_asset_tracking_resource()->mqtt_package_queue_mutex_handler);
  }
#if DEMO_CONFIG_DEBUG_LOGS
  printf(
    "\r\nsl_json_send_keep_alive_message : JSON format data is sent to the MQTT data queue\r\n");
#endif /// < DEMO_CONFIG_DEBUG_LOGS

  return SL_STATUS_OK;
  error:
  return SL_STATUS_FAIL;
}

/*****************************************************************************
 * Function to get time-stamp for JSON message.
 ******************************************************************************/
sl_status_t sl_json_get_timestamp(uint8_t *timestamp_buff)
{
  sl_calendar_datetime_config_t datetime;
  sl_status_t status;

  status = sl_si91x_calendar_get_date_time(&datetime);
  if (status != SL_STATUS_OK) {
    printf(
      "\r\nsl_json_get_timestamp : sl_si91x_calendar_get_date_time: Invalid Parameters, Error Code : %lu \n",
      status);
    return SL_STATUS_FAIL;
  }

  /// Max buffer size is to avoid warning based on format specifier size
  snprintf((char *)timestamp_buff,
           JSON_MAX_TIMESTAMP_BUFF_SIZE,
           "%u0%02u-%02u-%02uT%02u:%02u:%02u.%03uZ",
           datetime.Century,
           datetime.Year,
           datetime.Month,
           datetime.Day,
           datetime.Hour,
           datetime.Minute,
           datetime.Second,
           datetime.MilliSeconds);

  /// Max string size is maximum size of time-stamp
  timestamp_buff[JSON_MAX_TIMESTAMP_STRING_SIZE - 1] = '\0';
#if DEMO_CONFIG_DEBUG_LOGS
  printf("\r\nsl_json_get_timestamp : getjson_time: %s \r\n", timestamp_buff);
#endif /// < DEMO_CONFIG_DEBUG_LOGS

  return SL_STATUS_OK;
}

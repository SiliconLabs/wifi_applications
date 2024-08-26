/***************************************************************************/ /**
 * @file sl_wifi_asset_tracking_azure_handler.c
 * @brief Azure IoT Hub connection and data sending to cloud related functions
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
#include <sl_net.h>
#include <sl_net_si91x.h>
#include <sl_net_dns.h>
#include <sl_net_constants.h>
#include <sl_utility.h>
#include <socket.h>
#include <errno.h>
#include <socket.h>
#include <sl_si91x_core_utilities.h>
#include <sl_si91x_hmac.h>
#include <sl_wifi_asset_tracking_demo_config.h>
#include <sl_wifi_asset_tracking_azure_handler.h>
#include <sl_wifi_asset_tracking_app.h>

#include <silabs_dgcert_ca.pem.h>

#ifndef DEMO_CONFIG_DEVICE_SYMMETRIC_KEY
#include "sl_wifi_asset_tracking_cert.pem.h"
#include "sl_wifi_asset_tracking_key.pem.h"
#endif

/**
 * @brief To delete the specified Wi-Fi credential.
 */
extern sl_status_t sl_si91x_delete_credential(sl_net_credential_id_t id,
                                              sl_net_credential_type_t type);

/**
 * @brief Static buffer used to hold MQTT messages being sent and received.
 */
static uint8_t sl_mqtt_msg_buffer[DEMO_CONFIG_NETWORK_BUFFER_SIZE];

/******************************************************************************
 *  Callback function to start cloud communication from SiWG917 device.
 *****************************************************************************/
void sl_azure_cloud_communication_task()
{
  sl_wifi_asset_tracking_mqtt_package_queue_data_t mqtt_data_queue_reading =
  { 0, { 0 } };

  AzureIoTResult_t msg_result;
  int32_t rssi;

  /// This loop is used to establish connection with Azure cloud after wi-fi connection is successful
  while (1) {
    if ((SL_WIFI_CONNECTED
         == sl_get_wifi_asset_tracking_status()->wifi_conn_status)
        && (SL_CLOUD_NOT_CONNECTED
            == sl_get_wifi_asset_tracking_status()->azure_cloud_conn_status)) {
      /// create Azure IoT Hub connection
      if (SL_STATUS_OK == sl_start_azure_cloud_connection()) {
        /// update status flag as connected
        sl_get_wifi_asset_tracking_status()->azure_cloud_conn_status =
          SL_CLOUD_CONNECTED;

        sl_wifi_asset_tracking_lcd_print(INDEX_AZURE_CLOUD_CONNECTED);

        break;
      } else {
        if (SL_STATUS_OK != sl_retry_azure_cloud_connection()) {
          sl_get_wifi_asset_tracking_status()->azure_cloud_conn_status =
            SL_CLOUD_SHUTDOWN;
          if (eSuspended
              == eTaskGetState(sl_get_wifi_asset_tracking_resource()->task_list.
                               recovery_task_handler)) {
            vTaskResume(
              sl_get_wifi_asset_tracking_resource()->task_list.recovery_task_handler);
          }
        } else {
          /// update status flag as connected
          sl_get_wifi_asset_tracking_status()->azure_cloud_conn_status =
            SL_CLOUD_CONNECTED;

          sl_wifi_asset_tracking_lcd_print(INDEX_AZURE_CLOUD_CONNECTED);

          break;
        }
      }
    } else {
      vTaskSuspend(
        sl_get_wifi_asset_tracking_resource()->task_list.azure_cloud_communication_task_handler);
      continue;
    }
  }

  /// This loop is used to send data to Azure cloud once connection is establish
  while (1) {
    /// Check if MQTT data queue is empty
    if (QUEUE_EMPTY
        == uxQueueMessagesWaiting(sl_get_wifi_asset_tracking_resource()->
                                  mqtt_package_queue_handler)) {
#if DEMO_CONFIG_DEBUG_LOGS
      printf(
        "\r\nazure_communication_task : suspend azure communication task as mqtt_data_queue is empty\r\n");
#endif /// < DEMO_CONFIG_DEBUG_LOGS
      vTaskSuspend(
        sl_get_wifi_asset_tracking_resource()->task_list.azure_cloud_communication_task_handler);
    } else {
      if (SL_STATUS_OK != sl_get_wifi_rssi(&rssi)) {
        bool recovery_resume_required = false;

        sl_get_wifi_asset_tracking_status()->azure_cloud_conn_status =
          SL_CLOUD_DISCONNECTED;
        sl_get_wifi_asset_tracking_status()->wifi_conn_status =
          SL_WIFI_DISCONNECTED;

        /// Acquire semaphore to update the status of recovery task
        if (pdTRUE
            == (xSemaphoreTake(sl_get_wifi_asset_tracking_resource()->
                               recovery_status_mutex_handler,
                               portMAX_DELAY))) {
          if (SL_RECOVERY_IDLE
              == sl_get_wifi_asset_tracking_status()->recovery_progress_status)
          {
            sl_get_wifi_asset_tracking_status()->recovery_progress_status =
              SL_RECOVERY_INPROGRESS;
            recovery_resume_required = true;
          }

          xSemaphoreGive(
            sl_get_wifi_asset_tracking_resource()->recovery_status_mutex_handler);

          if (recovery_resume_required) {
            printf(
              "\r\nazure_communication_task : Resuming recovery task for Wi-Fi recovery\r\n");

            vTaskResume(
              sl_get_wifi_asset_tracking_resource()->task_list.recovery_task_handler);
          }
        }
      }

      if ((SL_CLOUD_CONNECTED
           == sl_get_wifi_asset_tracking_status()->azure_cloud_conn_status)
          && (SL_WIFI_CONNECTED
              == sl_get_wifi_asset_tracking_status()->wifi_conn_status)) {
        /// Acquire MQTT data queue mutex and receive data from MQTT data queue
        if (pdTRUE
            == (xSemaphoreTake(sl_get_wifi_asset_tracking_resource()->
                               mqtt_package_queue_mutex_handler,
                               portMAX_DELAY))) {
          xQueueReceive(
            sl_get_wifi_asset_tracking_resource()->mqtt_package_queue_handler,
            &mqtt_data_queue_reading,
            0);
          printf(
            "\r\nazure_communication_task : Data is received from the MQTT data queue\r\n");
          xSemaphoreGive(
            sl_get_wifi_asset_tracking_resource()->mqtt_package_queue_mutex_handler);
        }

        /// Print the JSON buffer
        printf("\r\n\r\nJSON Buffer: %.*s\r\n\r\n",
               (int)mqtt_data_queue_reading.mqtt_buffer_len,
               mqtt_data_queue_reading.mqtt_buffer);

        /// Send JSON pay load to the cloud using MQTT Publish message
        msg_result =
          AzureIoTHubClient_SendTelemetry(
            &(sl_get_wifi_asset_tracking_resource()
              ->azure_iot_hub_client),
            mqtt_data_queue_reading.mqtt_buffer,
            mqtt_data_queue_reading.mqtt_buffer_len,
            &(
              sl_get_wifi_asset_tracking_resource()->azure_msg_property_bag),
            eAzureIoTHubMessageQoS0,
            NULL);
        if (msg_result != eAzureIoTSuccess) {
          bool recovery_resume_required = false;

          printf("\r\nazure_communication_task : MQTT Publish sent failed\r\n");
          sl_get_wifi_asset_tracking_status()->azure_cloud_conn_status =
            SL_CLOUD_DISCONNECTED;

          /// Acquire semaphore to update the status of recovery task
          if (pdTRUE
              == (xSemaphoreTake(sl_get_wifi_asset_tracking_resource()->
                                 recovery_status_mutex_handler,
                                 portMAX_DELAY))) {
            if (SL_RECOVERY_IDLE
                == sl_get_wifi_asset_tracking_status()->recovery_progress_status)
            {
              sl_get_wifi_asset_tracking_status()->recovery_progress_status =
                SL_RECOVERY_INPROGRESS;
              recovery_resume_required = true;
            }

            xSemaphoreGive(
              sl_get_wifi_asset_tracking_resource()->recovery_status_mutex_handler);

            if (recovery_resume_required) {
              printf(
                "\r\nazure_communication_task : Resuming recovery task for Azure IoT Hub recovery\r\n\r\n");
              vTaskResume(
                sl_get_wifi_asset_tracking_resource()->task_list.recovery_task_handler);
            }
          }
        } else {
          printf("\r\nazure_communication_task : MQTT Publish sent success\r\n");
        }
      } else {
        /// Comes here when wi-fi or cloud or both is not connected
        printf(
          "\r\nazure_communication_task : suspend azure communication task as cloud/wifi is not connected\r\n");

        vTaskSuspend(
          sl_get_wifi_asset_tracking_resource()->task_list.azure_cloud_communication_task_handler);
      }
    }
  }
}

/******************************************************************************
 *  Function to initialize and start Azure cloud connection using configured
 *  authentication method.
 *****************************************************************************/
sl_status_t sl_start_azure_cloud_connection()
{
  /// Flash SSL certificates
  if (SL_STATUS_OK != sl_load_ssl_certificates()) {
    printf(
      "\r\nsl_start_azure_cloud_connection : Failed to load SSL certificates\r\n");
    return SL_STATUS_FAIL;
  }

  /// Create an TLS connection
  if (SL_STATUS_OK != sl_create_tls_client_connection()) {
    printf(
      "\r\nsl_start_azure_cloud_connection : Failed to create an TLS connection\r\n");
    return SL_STATUS_FAIL;
  }

  /// Create an MQTT connection with configured IoT Hub
  if (SL_STATUS_OK != sl_connect_azure_iot_hub()) {
    printf(
      "\r\nsl_start_azure_cloud_connection : Failed to create an MQTT connection\r\n");
    return SL_STATUS_FAIL;
  }

  /// Set MQTT properties for tele-metry messages
  if (SL_STATUS_OK != sl_create_telemetry_message_properties()) {
    printf(
      "\r\nsl_start_azure_cloud_connection : Failed to create an MQTT properties for telemetry messages\r\n");
    return SL_STATUS_FAIL;
  }

  return SL_STATUS_OK;
}

/******************************************************************************
 * Get Unix time in milliseconds.
 *****************************************************************************/
uint32_t sl_get_unix_time(void)
{
  return (uint32_t)osKernelGetTickCount();
}

/******************************************************************************
 *  Function to connect with Azure IoT Hub configured authentication method.
 *****************************************************************************/
sl_status_t sl_connect_azure_iot_hub()
{
  AzureIoTResult_t azure_iot_result_status;
  NetworkContext_t network_context = { 0 };
  TlsTransportParams_t tls_transport_params = { 0 };
  AzureIoTTransportInterface_t azure_transport;
  AzureIoTHubClientOptions_t azure_iot_hub_options = { 0 };
  bool is_session_present;
  uint8_t *iot_hub_hostname = (uint8_t *)DEMO_CONFIG_IOT_HUB_HOST_NAME;
  uint8_t *iot_hub_deviceid = (uint8_t *)DEMO_CONFIG_DEVICE_ID;
  uint32_t iot_hub_hostname_len = sizeof(DEMO_CONFIG_IOT_HUB_HOST_NAME) - 1;
  uint32_t iot_hub_deviceid_len = sizeof(DEMO_CONFIG_DEVICE_ID) - 1;

  /// Initialize transport parameters
  network_context.pParams = &tls_transport_params;

  /// Fill in transport interface, send and receive function pointers
  azure_transport.pxNetworkContext = &network_context;
  azure_transport.xSend = sl_tls_sock_send;
  azure_transport.xRecv = sl_tls_sock_recv;

  /// Initialize Azure IoT Hub options
  azure_iot_result_status =
    AzureIoTHubClient_OptionsInit(&azure_iot_hub_options);

  printf("\r\nsl_connect_azure_iot_hub : AzureIoTHubClient_OptionsInit: %x\r\n",
         azure_iot_result_status);

  if (azure_iot_result_status != eAzureIoTSuccess) {
    return SL_STATUS_FAIL;
  }

  azure_iot_hub_options.pucModuleID = (const uint8_t *)DEMO_CONFIG_MODULE_ID;
  azure_iot_hub_options.ulModuleIDLength = sizeof(DEMO_CONFIG_MODULE_ID) - 1;

#if defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wincompatible-pointer-types"
#endif // __GNUC__

  /// Initialize Azure IoT Hub Client
  azure_iot_result_status =
    AzureIoTHubClient_Init(&(sl_get_wifi_asset_tracking_resource()->
                             azure_iot_hub_client),
                           iot_hub_hostname,
                           iot_hub_hostname_len,
                           iot_hub_deviceid,
                           iot_hub_deviceid_len,
                           &azure_iot_hub_options,
                           sl_mqtt_msg_buffer,
                           sizeof(sl_mqtt_msg_buffer),
                           sl_get_unix_time,
                           &azure_transport);

#if defined(__GNUC__)
#pragma GCC diagnostic pop
#endif // __GNUC__

  printf("\r\nsl_connect_azure_iot_hub : AzureIoTHubClient_Init: %x\r\n",
         azure_iot_result_status);

  if (azure_iot_result_status != eAzureIoTSuccess) {
    return SL_STATUS_FAIL;
  }

#ifdef DEMO_CONFIG_DEVICE_SYMMETRIC_KEY
  azure_iot_result_status =
    AzureIoTHubClient_SetSymmetricKey(
      &(sl_get_wifi_asset_tracking_resource()->
        azure_iot_hub_client),
      (const uint8_t *)DEMO_CONFIG_DEVICE_SYMMETRIC_KEY,
      sizeof(DEMO_CONFIG_DEVICE_SYMMETRIC_KEY) - 1,
      sl_create_crypto_hmac);

  printf(
    "\r\nsl_connect_azure_iot_hub : AzureIoTHubClient_SetSymmetricKey: %x\r\n",
    azure_iot_result_status);

  if (azure_iot_result_status != eAzureIoTSuccess) {
    return SL_STATUS_FAIL;
  }
#endif

  /// Sends an MQTT Connect packet over the already established TLS connection.
  /// Waits for connection acknowledgment (CONNACK) packet.
  printf(
    "\r\nsl_connect_azure_iot_hub : Creating an MQTT connection to %s.\r\n",
    iot_hub_hostname);

  azure_iot_result_status =
    AzureIoTHubClient_Connect(&(sl_get_wifi_asset_tracking_resource()->
                                azure_iot_hub_client),
                              false,
                              &is_session_present,
                              TRANSPORT_MQTT_CONNACK_RECV_TIMEOUT_MS);

  if (azure_iot_result_status != eAzureIoTSuccess) {
    return SL_STATUS_FAIL;
  }

  return SL_STATUS_OK;
}

/******************************************************************************
 *  Function used to create Crypto HMAC configuration.
 *****************************************************************************/
uint32_t sl_create_crypto_hmac(const uint8_t *pucKey,
                               uint32_t ulKeyLength,
                               const uint8_t *pucData,
                               uint32_t ulDataLength,
                               uint8_t *pucOutput,
                               uint32_t ulOutputLength,
                               uint32_t *pulBytesCopied)
{
  sl_si91x_hmac_config_t hmac_config = { 0 };
  sl_status_t status;

  (void)pucKey;
  (void)ulKeyLength;
  (void)pucData;
  (void)ulDataLength;
  (void)pucOutput;
  (void)ulOutputLength;
  (void)pulBytesCopied;

  hmac_config.hmac_mode = SL_SI91X_HMAC_SHA_256;
  hmac_config.msg_length = ulDataLength;
  hmac_config.msg = pucData;
  hmac_config.key_config.B0.key_size = ulKeyLength;
  hmac_config.key_config.B0.key_type = SL_SI91X_TRANSPARENT_KEY;
  hmac_config.key_config.B0.key = (uint8_t *)pucKey;

  status = sl_si91x_hmac(&hmac_config, pucOutput);
  if (status != SL_STATUS_OK) {
    printf("\r\nsl_create_crypto_hmac : HMAC failed, Error Code : 0x%lX\r\n",
           status);
    return 1;
  }

  *pulBytesCopied = 32;

  printf(
    "\r\nsl_create_crypto_hmac : ulOutputLength : %ld, pulBytesCopied : %ld\r\n",
    ulOutputLength,
    *pulBytesCopied);
  printf("\r\nsl_create_crypto_hmac : HMAC success\r\n");
  return 0;
}

/******************************************************************************
* Function will create properties for tele-metry messages
******************************************************************************/
sl_status_t sl_create_telemetry_message_properties()
{
  AzureIoTResult_t azure_iot_status;

  /// Create a bag of properties for the tele-metry */
  azure_iot_status =
    AzureIoTMessage_PropertiesInit(&(sl_get_wifi_asset_tracking_resource()->
                                     azure_msg_property_bag),
                                   sl_get_wifi_asset_tracking_resource()->azure_msg_property_buff,
                                   0,
                                   sizeof(sl_get_wifi_asset_tracking_resource()
                                          ->azure_msg_property_buff));

  if (azure_iot_status != eAzureIoTSuccess) {
    return SL_STATUS_FAIL;
  }

  /// Sending a default property (Content-Type)
  azure_iot_status =
    AzureIoTMessage_PropertiesAppend(
      &(sl_get_wifi_asset_tracking_resource()->
        azure_msg_property_bag),
      (uint8_t *)AZ_IOT_MESSAGE_PROPERTIES_CONTENT_TYPE,
      sizeof(
        AZ_IOT_MESSAGE_PROPERTIES_CONTENT_TYPE) - 1,
      (uint8_t *)TRANSPORT_MQTT_MESSAGE_CONTENT_TYPE,
      sizeof(
        TRANSPORT_MQTT_MESSAGE_CONTENT_TYPE) - 1);

  if (azure_iot_status != eAzureIoTSuccess) {
    return SL_STATUS_FAIL;
  }

  /// Sending a default property (Content-Encoding)
  azure_iot_status =
    AzureIoTMessage_PropertiesAppend(
      &(sl_get_wifi_asset_tracking_resource()->
        azure_msg_property_bag),
      (uint8_t *)AZ_IOT_MESSAGE_PROPERTIES_CONTENT_ENCODING,
      sizeof(
        AZ_IOT_MESSAGE_PROPERTIES_CONTENT_ENCODING) - 1,
      (uint8_t *)TRANSPORT_MQTT_MESSAGE_CONTENT_ENCODING,
      sizeof(
        TRANSPORT_MQTT_MESSAGE_CONTENT_ENCODING) - 1);

  if (azure_iot_status != eAzureIoTSuccess) {
    return SL_STATUS_FAIL;
  }

  return SL_STATUS_OK;
}

/******************************************************************************
* Function will retry for Azure cloud connection with configured
* authentication method for five times at every five seconds.
******************************************************************************/
sl_status_t sl_retry_azure_cloud_connection()
{
  uint8_t retry_cnt;
  int32_t rssi = 0;
  sl_status_t status;

  sl_disconnect_azure_iot_hub();

  for (retry_cnt = 0; retry_cnt < AZURE_CLOUD_CONN_RETRY_COUNT; ++retry_cnt) {
    printf(
      "\r\nsl_retry_azure_cloud_connection : retrying to establish Azure cloud connection\r\n");

    sl_wifi_asset_tracking_lcd_print(INDEX_AZURE_CLOUD_RETRY);

    /// Check whether rssi is getting
    status = sl_get_wifi_rssi(&rssi);

    if (SL_STATUS_OK != status) {
      printf(
        "\r\nsl_retry_azure_cloud_connection : wi-fi also got disconnected.\r\n");

      sl_get_wifi_asset_tracking_status()->wifi_conn_status =
        SL_WIFI_DISCONNECTED;

      retry_cnt = AZURE_CLOUD_CONN_RETRY_COUNT;

      break;
    }

    if (SL_STATUS_OK != sl_start_azure_cloud_connection()) {
      sl_disconnect_azure_iot_hub();

      vTaskDelay(pdMS_TO_TICKS(
                   AZURE_CLOUD_CONN_DELAY_BETN_RETRY) * TIMER_CLOCK_OFFSET);

      continue;
    }
    break;
  }

  if (AZURE_CLOUD_CONN_RETRY_COUNT == retry_cnt) {
    return SL_STATUS_FAIL;
  }

  return SL_STATUS_OK;
}

/******************************************************************************
* Function is used to load SSL certificates based on authentication method.
******************************************************************************/
sl_status_t sl_load_ssl_certificates()
{
  sl_status_t status;

  /// Load SSL CA certificate
  status =
    sl_net_set_credential(SL_NET_TLS_SERVER_CREDENTIAL_ID(
                            SSL_CERTIFICATE_INDEX),
                          SL_NET_SIGNING_CERTIFICATE,
                          silabs_dgcert_ca,
                          sizeof(silabs_dgcert_ca) - 1);
  if (status != SL_STATUS_OK) {
    printf(
      "\r\nsl_load_ssl_certificates : Loading TLS CA certificate in to FLASH Failed, Error Code : 0x%lX\r\n",
      status);
    return status;
  }
  printf(
    "\r\nsl_load_ssl_certificates : Loading TLS CA certificate at index %d Successful\r\n",
    SSL_CERTIFICATE_INDEX);

#ifdef DEMO_CONFIG_DEVICE_SYMMETRIC_KEY
  // clear SSL Client certificate
  status =
    sl_si91x_delete_credential(SL_NET_TLS_CLIENT_CREDENTIAL_ID(
                                 SSL_CERTIFICATE_INDEX),
                               SL_NET_CERTIFICATE);
  if (status != SL_STATUS_OK) {
    printf(
      "\r\nsl_load_ssl_certificates : TLS certificate location not cleared, Error Code : 0x%lX\r\n",
      status);
    return status;
  }
  printf(
    "\r\nsl_load_ssl_certificates : TLS Client certificate at index %d cleared successfully\r\n",
    SSL_CERTIFICATE_INDEX);

  // clear SSL Client private key
  status =
    sl_si91x_delete_credential(SL_NET_TLS_CLIENT_CREDENTIAL_ID(
                                 SSL_CERTIFICATE_INDEX),
                               SL_NET_PRIVATE_KEY);
  if (status != SL_STATUS_OK) {
    printf(
      "\r\nsl_load_ssl_certificates : TLS Client private location not cleared, Error Code : 0x%lX\r\n",
      status);
    return status;
  }
  printf(
    "\r\nsl_load_ssl_certificates : TLS Client private key at index %d cleared successfully\r\n",
    SSL_CERTIFICATE_INDEX);
#else
  // Load SSL Client certificate
  status =
    sl_net_set_credential(SL_NET_TLS_CLIENT_CREDENTIAL_ID(
                            SSL_CERTIFICATE_INDEX),
                          SL_NET_CERTIFICATE,
                          sl_wifi_asset_tracking_cert,
                          sizeof(sl_wifi_asset_tracking_cert) - 1);
  if (status != SL_STATUS_OK) {
    printf(
      "\r\nsl_load_ssl_certificates : Loading TLS certificate in to FLASH failed, Error Code : 0x%lX\r\n",
      status);
    return status;
  }
  printf(
    "\r\nsl_load_ssl_certificates : Loading TLS Client certificate at index %d Successful\r\n",
    SSL_CERTIFICATE_INDEX);

  // Load SSL Client private key
  status =
    sl_net_set_credential(SL_NET_TLS_CLIENT_CREDENTIAL_ID(
                            SSL_CERTIFICATE_INDEX),
                          SL_NET_PRIVATE_KEY,
                          sl_wifi_asset_tracking_key,
                          sizeof(sl_wifi_asset_tracking_key) - 1);
  if (status != SL_STATUS_OK) {
    printf(
      "\r\nsl_load_ssl_certificates : Loading TLS Client private key in to FLASH Failed, Error Code : 0x%lX\r\n",
      status);
    return status;
  }
  printf(
    "\r\nsl_load_ssl_certificates : Loading TLS Client private key at index %d Successful\r\n",
    SSL_CERTIFICATE_INDEX);
#endif

  return SL_STATUS_OK;
}

/******************************************************************************
* Function is used to create TLS transport for Azure IoT Hub connection.
******************************************************************************/
sl_status_t sl_create_tls_client_connection(void)
{
  sl_status_t status;
  uint8_t count = DNS_REQ_COUNT;
  int socket_return_value = 0;
  socklen_t socket_length = sizeof(struct sockaddr_in);
  struct sockaddr_in server_address = { 0 };
  sl_ip_address_t dns_query_rsp = { 0 };
  sl_si91x_time_value timeout = { 0 };

  /// DNS query to resolve Azure IoT Hub host name
  do {
    status = sl_net_host_get_by_name(
      (const char *)DEMO_CONFIG_IOT_HUB_HOST_NAME,
      DNS_TIMEOUT,
      SL_NET_DNS_TYPE_IPV4,
      &dns_query_rsp);
    if (status == SL_STATUS_OK) {
      break;
    }
    count--;
  } while (count != 0);

  if (status != SL_STATUS_OK) {
    /// Return failure, if DNS resolution fails
    printf(
      "\r\nsl_create_tls_client_connection : Azure server DNS resolution failed\r\n");
    return SL_STATUS_FAIL;
  }

  printf("\r\nsl_create_tls_client_connection : Azure server port : %d, ip : ",
         AZURE_SERVER_PORT);
  print_sl_ip_address(&dns_query_rsp);

  sl_get_wifi_asset_tracking_resource()->client_socket_id = socket(AF_INET,
                                                                   SOCK_STREAM,
                                                                   IPPROTO_TCP);
  if (sl_get_wifi_asset_tracking_resource()->client_socket_id < 0) {
    printf(
      "\r\nsl_create_tls_client_connection : Socket creation failed with bsd error: %d\r\n",
      errno);
    return SL_STATUS_FAIL;
  }

  printf("\r\nsl_create_tls_client_connection : Socket ID : %d\r\n",
         sl_get_wifi_asset_tracking_resource()->client_socket_id);

  socket_return_value =
    setsockopt(sl_get_wifi_asset_tracking_resource()->client_socket_id,
               SOL_TCP, TCP_ULP, TLS, sizeof(TLS));

  if (socket_return_value < 0) {
    printf(
      "\r\nsl_create_tls_client_connection : Set Socket option failed with bsd error: %d\r\n",
      errno);

    close(sl_get_wifi_asset_tracking_resource()->client_socket_id);
    return SL_STATUS_FAIL;
  }

  timeout.tv_usec = TRANSPORT_SEND_RECV_TIMEOUT_MS * 1000;
  socket_return_value =
    setsockopt(sl_get_wifi_asset_tracking_resource()->client_socket_id,
               SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));

  if (socket_return_value < 0) {
    printf(
      "\r\nsl_create_tls_client_connection : Set Socket option failed with bsd error: %d\r\n",
      errno);

    close(sl_get_wifi_asset_tracking_resource()->client_socket_id);
    return SL_STATUS_FAIL;
  }

  server_address.sin_family = AF_INET;
  server_address.sin_port = AZURE_SERVER_PORT;
  server_address.sin_addr.s_addr = dns_query_rsp.ip.v4.value;

  socket_return_value =
    connect(sl_get_wifi_asset_tracking_resource()->client_socket_id,
            (struct sockaddr *)&server_address,
            socket_length);

  if (socket_return_value < 0) {
    printf(
      "\r\nsl_create_tls_client_connection : Socket Connect failed with bsd error: %d\r\n",
      errno);

    close(sl_get_wifi_asset_tracking_resource()->client_socket_id);
    return SL_STATUS_FAIL;
  }
  printf(
    "\r\nsl_create_tls_client_connection : Socket : %d connected to TLS server \r\n",
    sl_get_wifi_asset_tracking_resource()->client_socket_id);

  return SL_STATUS_OK;
}

/******************************************************************************
* Callback function used to receive data from TLS socket connection.
******************************************************************************/
int32_t sl_tls_sock_recv(NetworkContext_t *network_context,
                         void *buffer,
                         size_t bytes_to_recv)
{
  UNUSED_PARAMETER(network_context);

  int32_t recv_bytes =
    recv(sl_get_wifi_asset_tracking_resource()->client_socket_id,
         buffer, bytes_to_recv, 0);

  if (recv_bytes < 0) {
    if (SL_STATUS_SI91X_SOCKET_READ_TIMEOUT
        == sl_si91x_get_saved_firmware_status()) {
#if DEMO_CONFIG_DEBUG_LOGS
      printf("sl_tls_sock_recv : Receive timed out\r\n");
#endif /// < DEMO_CONFIG_DEBUG_LOGS
      return 0;
    }
  }
#if DEMO_CONFIG_DEBUG_LOGS
  printf("\r\nsl_tls_sock_recv : bytes received: %ld\r\n", recv_bytes);
#endif /// < DEMO_CONFIG_DEBUG_LOGS

  return recv_bytes;
}

/******************************************************************************
* Callback function used to send data on TLS socket connection.
******************************************************************************/
int32_t sl_tls_sock_send(NetworkContext_t *network_context,
                         const void *buffer,
                         size_t bytes_to_send)
{
  UNUSED_PARAMETER(network_context);

  int32_t sent_bytes =
    send(sl_get_wifi_asset_tracking_resource()->client_socket_id,
         buffer, bytes_to_send, 0);
#if DEMO_CONFIG_DEBUG_LOGS
  printf("\r\nsl_tls_sock_send : bytes sent : %ld\r\n", sent_bytes);
#endif /// < DEMO_CONFIG_DEBUG_LOGS

  return sent_bytes;
}

/******************************************************************************
* Function used to disconnect Azure IoT Hub connection and clean up TLS socket.
******************************************************************************/
sl_status_t sl_disconnect_azure_iot_hub()
{
  /// Disconnect Azure IoT Hub Connection
  AzureIoTHubClient_Disconnect(&(sl_get_wifi_asset_tracking_resource()->
                                 azure_iot_hub_client));

  /// close TLS socket
  if (sl_get_wifi_asset_tracking_resource()->client_socket_id >= 0) {
    close(sl_get_wifi_asset_tracking_resource()->client_socket_id);
  }

  /// reset socket variable
  sl_get_wifi_asset_tracking_resource()->client_socket_id = -1;

  return SL_STATUS_OK;
}

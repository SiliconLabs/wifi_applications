/***************************************************************************/ /**
 * @file sl_wifi_asset_tracking_azure_handler.h
 * @brief Azure IoT hub connection and data sending related header file
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

#ifndef SL_WIFI_ASSET_TRACKING_AZURE_HANDLER_H_
#define SL_WIFI_ASSET_TRACKING_AZURE_HANDLER_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <sl_status.h>
#include <azure_iot_hub_client.h>
#include <sl_transport_tls_socket.h>

/*******************************************************************************
 ***************************  Defines / Macros  ********************************
 ******************************************************************************/
#define AZURE_CLOUD_CONN_RETRY_COUNT              10    ///< In numbers
#define AZURE_CLOUD_CONN_DELAY_BETN_RETRY         5000  ///< In ms
#define QUEUE_EMPTY                               0     ///< Empty queue status
#define MAX_JSON_MESSAGE_SIZE                     200   ///< Maximum size of JSON message
#define SSL_CERTIFICATE_INDEX                     0     ///< SSL certificate index
#define DNS_REQ_COUNT                             5     ///< Maximum DNS request count
#define DNS_TIMEOUT                               20000 ///< DNS timeout in ms
#define AZURE_SERVER_PORT                         8883  ///< Azure server port number

/**
 * @brief Transport timeout in milliseconds for transport send and receive.
 */
#define TRANSPORT_SEND_RECV_TIMEOUT_MS            (2000U)

/**
 * @brief Timeout for receiving CONNACK packet in milliseconds.
 */
#define TRANSPORT_MQTT_CONNACK_RECV_TIMEOUT_MS    (10 * 1000U)

/**
 * @brief  The content type of the Tele-metry message published in this example.
 * @remark Message properties must be url-encoded.
 *         This message property is not required to send tele-metry.
 */
#define TRANSPORT_MQTT_MESSAGE_CONTENT_TYPE       "application%2Fjson"

/**
 * @brief  The content encoding of the Tele-metry message published in this example.
 * @remark Message properties must be url-encoded.
 *         This message property is not required to send tele-metry.
 */
#define TRANSPORT_MQTT_MESSAGE_CONTENT_ENCODING   "us-ascii"

/*******************************************************************************
 **************************   ENUMS and Structures   ***************************
 ******************************************************************************/

/// @brief Structure for MQTT package data queue object
typedef struct {
  int32_t mqtt_buffer_len;                    ///< MQTT buffer length
  uint8_t mqtt_buffer[MAX_JSON_MESSAGE_SIZE]; ///< MQTT JSON message buffer
} sl_wifi_asset_tracking_mqtt_package_queue_data_t;

/// @brief Structure to store network context
struct NetworkContext {
  void *pParams; ///< pointer to network context
};

// -----------------------------------------------------------------------------
// Prototypes

/**************************************************************************/ /**
 * @brief Callback function to start cloud communication from SiWG917 device.
 ******************************************************************************/
void sl_azure_cloud_communication_task();

/**************************************************************************/ /**
 * @brief Function to initialize and start Azure cloud connection using
 * configured authentication method.
 * @return The following values are returned:
 * -  \ref SL_STATUS_OK on success
 * -  \ref SL_STATUS_FAIL - on sensor disable failed
 ******************************************************************************/
sl_status_t sl_start_azure_cloud_connection();

/******************************************************************************
 *  @brief Function to connect with Azure IoT Hub configured
 *  authentication method.
 * @return The following values are returned:
 * -  \ref SL_STATUS_OK on success
 * -  \ref SL_STATUS_FAIL - on connect failed with Azure IoT Hub
 *****************************************************************************/
sl_status_t sl_connect_azure_iot_hub();

/**************************************************************************/ /**
 * @brief Function will retry for Azure cloud connection with configured
 * authentication method for five times at every five seconds.
 * @return The following values are returned:
 * -  \ref SL_STATUS_OK on success
 * -  \ref SL_STATUS_FAIL - on sensor disable failed
 ******************************************************************************/
sl_status_t sl_retry_azure_cloud_connection();

/**************************************************************************/ /**
 * @brief Function is used to load SSL certificates based on
 * authentication method.
 * @return The following values are returned:
 * -  \ref SL_STATUS_OK on success
 * -  \ref SL_STATUS_FAIL - on SSL certificates loading failed
 ******************************************************************************/
sl_status_t sl_load_ssl_certificates();

/******************************************************************************
 * @brief Function is used to create TLS transport for Azure IoT Hub connection.
 * @return The following values are returned:
 * -  \ref SL_STATUS_OK on success
 * -  \ref SL_STATUS_FAIL - on TLS connection creation failed
 ******************************************************************************/
sl_status_t sl_create_tls_client_connection();

/**************************************************************************/ /**
 * @brief Callback function used to receive data from TLS socket connection.
 * @param network_context: Pointer to the Network context.
 * @param buffer: Buffer used for receiving data.
 * @param bytes_to_recv: Size of the buffer.
 * @return An int32_t number of bytes copied.
 ******************************************************************************/
int32_t sl_tls_sock_recv(NetworkContext_t *network_context,
                         void *buffer,
                         size_t bytes_to_recv);

/**************************************************************************/ /**
 * @brief Callback function used to send data on TLS socket connection.
 * @param network_context: Pointer to the Network context.
 * @param buffer: Buffer that contains data to be sent.
 * @param bytes_to_send: Length of the data to be sent.
 * @return An int32_t number of bytes successfully sent.
 ******************************************************************************/
int32_t sl_tls_sock_send(NetworkContext_t *network_context,
                         const void *buffer,
                         size_t bytes_to_send);

/**************************************************************************/ /**
 * @brief Get Unix time in milliseconds.
 * @return An uint32_t time in milliseconds.
 ******************************************************************************/
uint32_t sl_get_unix_time(void);

/**************************************************************************/ /**
 * @brief Function used to create Crypto HMAC configuration.
 * @param[in] pucKey : Key string.
 * @param[in] ulKeyLength : Key string length.
 * @param[in] pucData : Pointer to an Data.
 * @param[in] ulDataLength : Length of Data.
 * @param[out] pucOutput : Pointer to store an output Data.
 * @param[in] ulOutputLength : Output data length.
 * @param[out] pulBytesCopied : No of bytes to be copied.
 * @return An uint32_t returns bytes copied.
 ******************************************************************************/
uint32_t sl_create_crypto_hmac(const uint8_t *pucKey,
                               uint32_t ulKeyLength,
                               const uint8_t *pucData,
                               uint32_t ulDataLength,
                               uint8_t *pucOutput,
                               uint32_t ulOutputLength,
                               uint32_t *pulBytesCopied);

/**************************************************************************/ /**
 * @brief Function will create properties for tele-metry messages.
 * @return The following values are returned:
 * -  \ref SL_STATUS_OK on success
 * -  \ref SL_STATUS_FAIL - on properties creation failure
 ******************************************************************************/
sl_status_t sl_create_telemetry_message_properties();

/**************************************************************************/ /**
 * @brief Function used to disconnect Azure IoT Hub connection
 * and clean up TLS socket.
 * @return The following values are returned:
 * -  \ref SL_STATUS_OK on success
 * -  \ref SL_STATUS_FAIL - on properties creation failure
 ******************************************************************************/
sl_status_t sl_disconnect_azure_iot_hub();

#ifdef __cplusplus
}
#endif

#endif /* SL_WIFI_ASSET_TRACKING_AZURE_HANDLER_H_ */

/******************************************************************************/
/* EOF                                                                        */
/******************************************************************************/

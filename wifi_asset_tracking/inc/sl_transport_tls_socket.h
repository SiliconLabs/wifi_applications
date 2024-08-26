/***************************************************************************/ /**
 * @file sl_transport_tls_socket.h
 * @brief TLS transport related structures and prototypes
 *******************************************************************************
 * # License
 * <b>Copyright 2023 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * SPDX-License-Identifier: Zlib
 *
 * The licensor of this software is Silicon Laboratories Inc.
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 *
 ******************************************************************************/

#ifndef SL_TRANSPORT_TLS_SOCKET_H_
#define SL_TRANSPORT_TLS_SOCKET_H_

#ifdef __cplusplus
extern "C" {
#endif

typedef void *SSLContextHandle;                  ///< SSL Context Handle
typedef void *SocketHandle;                      ///< Socket Handle
typedef struct NetworkContext NetworkContext_t;  ///< Network Context type

/**
 * @brief Contains the TLS transport parameters.
 */
typedef struct TlsTransportParams {
  SocketHandle xTCPSocket;      ///< Socket handler for TCP socket
  SSLContextHandle xSSLContext; ///< Context Handler for SSL context
} TlsTransportParams_t;

/**
 * @brief Contains the credentials necessary for TLS connection setup.
 */
typedef struct NetworkCredentials {
  const char **ppcAlpnProtos;   ///<  To use ALPN, set this to a NULL-terminated list of supported protocols in decreasing order of preference.
  BaseType_t xDisableSni;       ///<  Disable server name indication (SNI) for a TLS session.
  const uint8_t *pucRootCa;     ///<  String representing a trusted server root certificate.
  size_t xRootCaSize;           ///<  Size associated with NetworkCredentials.pucRootCa.
  const uint8_t *pucClientCert; ///<  String representing the client certificate.
  size_t xClientCertSize;       ///<  Size associated with NetworkCredentials.pucClientCert.
  const uint8_t *pucPrivateKey; ///<  String representing the client certificate's private key.
  size_t xPrivateKeySize;       ///<  Size associated with NetworkCredentials.pucPrivateKey.
} NetworkCredentials_t;

/**
 * @brief TLS Connect / Disconnect return status.
 */
typedef enum TlsTransportStatus {
  eTLSTransportSuccess = 0,        ///< Function successfully completed.
  eTLSTransportInvalidParameter,   ///< At least one parameter was invalid.
  eTLSTransportInsufficientMemory, ///< Insufficient memory required to establish connection.
  eTLSTransportInvalidCredentials, ///< Provided credentials were invalid.
  eTLSTransportHandshakeFailed,    ///< Performing TLS handshake with server failed.
  eTLSTransportInternalError,      ///< A call to a system API resulted in an internal error.
  eTLSTransportConnectFailure,     ///< Initial connection to the server failed.
  eTLSTransportCAVerifyFailed      ///< Verification of TLS CA cert failed.
} TlsTransportStatus_t;

/**
 * @brief Connect to TLS endpoint
 *
 * @param[in] pxNetworkContext Pointer to the Network context.
 * @param[in] pcHostName Pointer to NULL terminated hostname.
 * @param[in] usPort Port to connect to.
 * @param[in] pxNetworkCredentials Pointer to network credentials.
 * @param[in] ulReceiveTimeoutMs Receive timeout.
 * @param[in] ulSendTimeoutMs Send timeout.
 * @return A #TlsTransportStatus_t with the result of the operation.
 */
TlsTransportStatus_t TLS_Socket_Connect(NetworkContext_t *pxNetworkContext,
                                        const char *pcHostName,
                                        uint16_t usPort,
                                        const NetworkCredentials_t *pxNetworkCredentials,
                                        uint32_t ulReceiveTimeoutMs,
                                        uint32_t ulSendTimeoutMs);

/**
 * @brief Disconnect the TLS connection
 *
 * @param[in] pxNetworkContext Pointer to the Network context.
 */
void TLS_Socket_Disconnect(NetworkContext_t *pxNetworkContext);

/**
 * @brief Receive data from TLS.
 *
 * @param pxNetworkContext Pointer to the Network context.
 * @param pvBuffer Buffer used for receiving data.
 * @param xBytesToRecv Size of the buffer.
 * @return An int32_t number of bytes copied.
 */
int32_t TLS_Socket_Recv(NetworkContext_t *pxNetworkContext,
                        void *pvBuffer,
                        size_t xBytesToRecv);

/**
 * @brief Send data using TLS.
 *
 * @param pxNetworkContext Pointer to the Network context.
 * @param pvBuffer Buffer that contains data to be sent.
 * @param xBytesToSend Length of the data to be sent.
 * @return An int32_t number of bytes successfully sent.
 */
int32_t TLS_Socket_Send(NetworkContext_t *pxNetworkContext,
                        const void *pvBuffer,
                        size_t xBytesToSend);

#ifdef __cplusplus
}
#endif

#endif /* SL_TRANSPORT_TLS_SOCKET_H_ */

/******************************************************************************/
/* EOF                                                                        */
/******************************************************************************/

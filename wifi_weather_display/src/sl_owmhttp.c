/*
 * @file sl_owmhttp.c
 * @brief Top level OpenWeather Map HTTP functions
 *******************************************************************************
 * # License
 * <b>Copyright 2022 Silicon Laboratories Inc. www.silabs.com</b>
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

#include "sl_owmhttp.h"

/******************************************************
*                    Constants
******************************************************/
//! HTTP Client Configurations
#define HTTP_CLIENT_USERNAME   "admin"
#define HTTP_CLIENT_PASSWORD   "admin"

#define IP_VERSION             SL_IPV4
#define HTTP_VERSION           SL_HTTP_V_1_1

//! Set 1 - Enable and 0 - Disable
#define HTTPS_ENABLE           1

//! Set 1 - Enable and 0 - Disable
#define EXTENDED_HEADER_ENABLE 0

#if HTTPS_ENABLE
//! Set TLS version
#define TLS_VERSION            SL_TLS_V_1_2

#define CERTIFICATE_INDEX      SL_HTTPS_CLIENT_CERTIFICATE_INDEX_1

//! Include SSL CA certificate
#include "cacert.pem.h"

//! Load certificate to device flash :
//! Certificate could be loaded once and need not be loaded for every boot up
#define LOAD_CERTIFICATE    1
#endif

//! Server port number
#if HTTPS_ENABLE
//! Set default HTTPS port
#define HTTP_PORT           443
#else
//! Set default HTTP port
#define HTTP_PORT           80
#endif

#if EXTENDED_HEADER_ENABLE
//! Extended headers
#define KEY1                "Content-Type"
#define VAL1                "text/html; charset=utf-8"

#define KEY2                "Content-Encoding"
#define VAL2                "br"

#define KEY3                "Content-Language"
#define VAL3                "de-DE"

#define KEY4                "Content-Location"
#define VAL4                "/index.html"
#endif

//! End of data indications
// No data pending from host
#define HTTP_END_OF_DATA    1

#define HTTP_SYNC_RESPONSE  0
#define HTTP_ASYNC_RESPONSE 1

#define DNS_TIMEOUT         20000
#define MAX_DNS_RETRY_COUNT 5
#define RSI_TCP_IP_BYPASS   RSI_DISABLE

#define DHCP_HOST_NAME      NULL

sl_status_t callback_status = SL_STATUS_OK;

/******************************************************
*                      Macros
******************************************************/
#define CLEAN_HTTP_CLIENT_IF_FAILED(status, client_handle, is_sync) \
  {                                                                 \
    if (status != SL_STATUS_OK)                                     \
    {                                                               \
      sl_http_client_deinit(client_handle);                         \
      return ((is_sync == 0) ? status : callback_status);           \
    }                                                               \
  }

// Function the resolve the IP address based on the hostname parameter
sl_status_t sl_resolve_IP(char *hostname, uint8_t *server_ip)
{
  sl_status_t status = SL_STATUS_OK;

  sl_ip_address_t dns_query_rsp = { 0 };
  uint32_t server_address;
  int32_t dns_retry_count = MAX_DNS_RETRY_COUNT;

  do
  {
    //! Getting IP address of the remote server using DNS request
    status = sl_net_host_get_by_name((const char *)hostname,
                                     DNS_TIMEOUT,
                                     SL_NET_DNS_TYPE_IPV4,
                                     &dns_query_rsp);
    dns_retry_count--;
  } while ((dns_retry_count != 0) && (status != SL_STATUS_OK));

  if (status != SL_STATUS_OK) {
    return status;
  }

  server_address = dns_query_rsp.ip.v4.value;
  sprintf((char *)server_ip,
          "%ld.%ld.%ld.%ld",
          server_address & 0x000000ff,
          (server_address & 0x0000ff00) >> 8,
          (server_address & 0x00ff0000) >> 16,
          (server_address & 0xff000000) >> 24);

  printf("\r\nServer IP address = %s\r\n", server_ip);

  return status;
}

// Helper fucntion to generate URL for the API call
sl_status_t sl_gen_url(char *base_url,
                       char *api_key,
                       char *latitude,
                       char *longitude,
                       char *full_url)
{
  snprintf(full_url, 300, base_url, latitude, longitude, api_key);

  return SL_STATUS_OK;
}

sl_status_t http_response_status(volatile uint8_t *response)
{
  while (!(*response))
  {
    /* Wait till response arrives */
  }

  if (*response != HTTP_SUCCESS_RESPONSE) {
    return SL_STATUS_FAIL;
  }

  // Reset response
  *response = 0;

  return SL_STATUS_OK;
}

// Function to initialize and perform a HTTP get request
sl_status_t sl_http_client_begin(uint8_t *server_ip,
                                 char *url,
                                 uint8_t *rsp_received)
{
  sl_status_t status = SL_STATUS_OK;
  sl_http_client_t client_handle = 0;
  sl_http_client_configuration_t client_configuration = { 0 };
  sl_http_client_request_t client_request = { 0 };

// Load the CA root certificate
#if HTTPS_ENABLE && LOAD_CERTIFICATE
  // Load SSL CA certificate
  status =
    sl_net_set_credential(SL_NET_TLS_SERVER_CREDENTIAL_ID(CERTIFICATE_INDEX),
                          SL_NET_SIGNING_CERTIFICATE,
                          cacert,
                          sizeof(cacert) - 1);
  if (status != SL_STATUS_OK) {
    printf(
      "\r\nLoading TLS CA certificate in to FLASH Failed, Error Code : 0x%lX\r\n",
      status);
  }
  printf("\r\nLoad TLS CA certificate at index %d Success\r\n",
         CERTIFICATE_INDEX);
#endif

  //! Set HTTP Client credentials
  uint16_t username_length = strlen(HTTP_CLIENT_USERNAME);
  uint16_t password_length = strlen(HTTP_CLIENT_PASSWORD);
  uint32_t credential_size = sizeof(sl_http_client_credentials_t)
                             + username_length + password_length;

  sl_http_client_credentials_t *client_credentials =
    (sl_http_client_credentials_t *)malloc(credential_size);
  SL_VERIFY_POINTER_OR_RETURN(client_credentials, SL_STATUS_ALLOCATION_FAILED);
  memset(client_credentials, 0, credential_size);
  client_credentials->username_length = username_length;
  client_credentials->password_length = password_length;

  memcpy(&client_credentials->data[0], HTTP_CLIENT_USERNAME, username_length);
  memcpy(&client_credentials->data[username_length],
         HTTP_CLIENT_PASSWORD,
         password_length);

  status =
    sl_net_set_credential(SL_NET_HTTP_CLIENT_CREDENTIAL_ID(CERTIFICATE_INDEX),
                          SL_NET_HTTP_CLIENT_CREDENTIAL,
                          client_credentials,
                          credential_size);
  if (status != SL_STATUS_OK) {
    free(client_credentials);
    return status;
  }

  //! Fill HTTP client_configurations
  client_configuration.network_interface = SL_NET_WIFI_CLIENT_INTERFACE;
  client_configuration.ip_version = IP_VERSION;
  client_configuration.http_version = HTTP_VERSION;
#if HTTPS_ENABLE
  client_configuration.https_enable = true;
  client_configuration.tls_version = TLS_VERSION;
  client_configuration.certificate_index = CERTIFICATE_INDEX;
#endif

  //! Fill HTTP client_request configurations
  client_request.ip_address = (uint8_t *)server_ip;
  client_request.port = HTTP_PORT;
  client_request.resource = (uint8_t *)url;
  client_request.extended_header = NULL;

  status = sl_http_client_init(&client_configuration, &client_handle);
  VERIFY_STATUS_AND_RETURN(status);
  printf("\r\nHTTP Client init success\r\n");

#if EXTENDED_HEADER_ENABLE
  //! Add extended headers
  status = sl_http_client_add_header(&client_request, KEY1, VAL1);
  CLEAN_HTTP_CLIENT_IF_FAILED(status, &client_handle, HTTP_SYNC_RESPONSE);

  status = sl_http_client_add_header(&client_request, KEY2, VAL2);
  CLEAN_HTTP_CLIENT_IF_FAILED(status, &client_handle, HTTP_SYNC_RESPONSE);

  status = sl_http_client_add_header(&client_request, KEY3, VAL3);
  CLEAN_HTTP_CLIENT_IF_FAILED(status, &client_handle, HTTP_SYNC_RESPONSE);

  status = sl_http_client_add_header(&client_request, KEY4, VAL4);
  CLEAN_HTTP_CLIENT_IF_FAILED(status, &client_handle, HTTP_SYNC_RESPONSE);
#endif

  //! Configure HTTP GET request
  client_request.http_method_type = SL_HTTP_GET;

  //! Initialize callback method for HTTP GET request
  status = sl_http_client_request_init(&client_request,
                                       http_get_response_callback_handler,
                                       "This is HTTP client");
  CLEAN_HTTP_CLIENT_IF_FAILED(status, &client_handle, HTTP_SYNC_RESPONSE);

  //! Send HTTP GET request
  status = sl_http_client_send_request(&client_handle, &client_request);
  if (status == SL_STATUS_IN_PROGRESS) {
    status = http_response_status(rsp_received);
    CLEAN_HTTP_CLIENT_IF_FAILED(status, &client_handle, HTTP_ASYNC_RESPONSE);
  } else {
    CLEAN_HTTP_CLIENT_IF_FAILED(status, &client_handle, HTTP_SYNC_RESPONSE);
  }

  printf("\r\nHTTP GET request Success\r\n");

  status = sl_http_client_deinit(&client_handle);
  VERIFY_STATUS_AND_RETURN(status);

  free(client_credentials);

  return status;
}

// Function to parse the JSON data and store the information in aqi_data and weather_data
sl_status_t parse_jsonString(char *json_str,
                             aqi_data_t *aqi_data,
                             weather_data_t *weather_data,
                             uint8_t parse)
{
  sl_status_t status = SL_STATUS_OK;

  if (parse == PARSE_AQI_DATA) {
    UNUSED_PARAMETER(weather_data);

    cJSON *json = cJSON_Parse(json_str);
    if (json == NULL) {
      printf("Error parsing JSON.\n");
      return SL_STATUS_FAIL;
    }

    // Get "list" array
    cJSON *list = cJSON_GetObjectItem(json, "list");
    if ((list == NULL) || !cJSON_IsArray(list)) {
      printf("Error: 'list' array not found.\n");
      cJSON_Delete(json);
      return SL_STATUS_FAIL;
    }

    // Get the first item in the "list" array
    cJSON *list_item = cJSON_GetArrayItem(list, 0);

    // Parse the AQI (Air Quality Index)
    cJSON *main = cJSON_GetObjectItem(list_item, "main");
    aqi_data->aqi = cJSON_GetObjectItem(main, "aqi")->valueint;

    // Parse the components (pollutants)
    cJSON *components = cJSON_GetObjectItem(list_item, "components");
    aqi_data->co = cJSON_GetObjectItem(components, "co")->valuedouble;
    aqi_data->no2 = cJSON_GetObjectItem(components, "no2")->valuedouble;
    aqi_data->so2 = cJSON_GetObjectItem(components, "so2")->valuedouble;
    aqi_data->ozone = cJSON_GetObjectItem(components, "o3")->valuedouble;
    aqi_data->pm2_5 = cJSON_GetObjectItem(components, "pm2_5")->valuedouble;
    aqi_data->pm10 = cJSON_GetObjectItem(components, "pm10")->valuedouble;

    cJSON_Delete(json);
  } else if (parse == PARSE_WEATHER_DATA) {
    UNUSED_PARAMETER(aqi_data);

    cJSON *json = cJSON_Parse(json_str);
    if (json == NULL) {
      printf("Error parsing JSON data.\n");
      return SL_STATUS_FAIL;
    }

    // Extract "description" from the "weather" array
    cJSON *weather_array = cJSON_GetObjectItemCaseSensitive(json, "weather");
    if (cJSON_IsArray(weather_array)) {
      cJSON *weather_item = cJSON_GetArrayItem(weather_array, 0);       // Get the first item
      if (weather_item != NULL) {
        cJSON *description = cJSON_GetObjectItemCaseSensitive(weather_item,
                                                              "description");
        if (cJSON_IsString(description) && (description->valuestring != NULL)) {
          strncpy(weather_data->description, description->valuestring,
                  sizeof(weather_data->description) - 1);
        }
      }
    }

    // Extract "temp", "feels_like", "humidity" from "main"
    cJSON *main_obj = cJSON_GetObjectItemCaseSensitive(json, "main");
    if (main_obj != NULL) {
      cJSON *temp = cJSON_GetObjectItemCaseSensitive(main_obj, "temp");
      cJSON *feels_like = cJSON_GetObjectItemCaseSensitive(main_obj,
                                                           "feels_like");
      cJSON *humidity = cJSON_GetObjectItemCaseSensitive(main_obj, "humidity");

      if (cJSON_IsNumber(temp)) {
        weather_data->temp = temp->valuedouble;
      }
      if (cJSON_IsNumber(feels_like)) {
        weather_data->feels_like = feels_like->valuedouble;
      }
      if (cJSON_IsNumber(humidity)) {
        weather_data->humidity = humidity->valueint;
      }
    }

    // Extract "rain" value
    cJSON *rain_obj = cJSON_GetObjectItemCaseSensitive(json, "rain");
    if (rain_obj != NULL) {
      // The key can be "1h" or "3h", depending on the data
      cJSON *rain_value = cJSON_GetObjectItemCaseSensitive(rain_obj, "1h");
      if (rain_value == NULL) {
        rain_value = cJSON_GetObjectItemCaseSensitive(rain_obj, "3h");
      }
      if (cJSON_IsNumber(rain_value)) {
        weather_data->rain = rain_value->valuedouble;
      }
    } else {
      weather_data->rain = 0.0;       // If no rain data, set to 0
    }

    cJSON_Delete(json);
  }

  return status;
}

// Function to get the AQI category
const char *get_aqi_category(uint8_t aqi)
{
  switch (aqi)
  {
    case 1:
      return "Good";
    case 2:
      return "Fair";
    case 3:
      return "Moderate";
    case 4:
      return "Poor";
    case 5:
      return "Very Poor";
    default:
      return "Invalid AQI index";
  }
}

/*
 * @file sl_owmhttp.h
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

#ifndef SL_OWMHTTP_H_
#define SL_OWMHTTP_H_

#include "cmsis_os2.h"
#include "sl_wifi.h"
#include "sl_net.h"
#include "sl_http_client.h"
#include <string.h>
#include "sl_net_dns.h"
#include "sl_wifi_callback_framework.h"
#include "sl_net_ping.h"
#include "sl_net_wifi_types.h"
#include "sl_net_default_values.h"
#include "sl_utility.h"

#include "cJSON.h"

#include "cJSON_Utils.h"

#define HTTP_ASYNC_AWAIT      0
#define HTTP_SUCCESS_RESPONSE 1
#define HTTP_FAILURE_RESPONSE 2

#define PARSE_WEATHER_DATA    0
#define PARSE_AQI_DATA        1

// Weather Data struct
typedef struct weather_data
{
  char description[256];
  double temp;
  double feels_like;
  int humidity;
  double rain;
  int32_t local_temperature;
  uint32_t local_humidity;
} weather_data_t;

// Air Quality Index struct
typedef struct aqi_data
{
  uint8_t aqi;
  double ozone;
  double pm2_5;
  double pm10;
  double co;
  double so2;
  double no2;
} aqi_data_t;

sl_status_t http_get_response_callback_handler(const sl_http_client_t *client,
                                               sl_http_client_event_t event,
                                               void *data,
                                               void *request_context);

// Function to initialize and perform a HTTP get request
sl_status_t sl_http_client_begin(uint8_t *server_ip,
                                 char *url,
                                 uint8_t *rsp_received);

// Function to resolve the IP address based on the hostname parameter
sl_status_t sl_resolve_IP(char *hostname, uint8_t *server_ip);

// Function to generate URL for the API call
sl_status_t sl_gen_url(char *base_url,
                       char *api_key,
                       char *latitude,
                       char *longitude,
                       char *full_url);

// Function to parse the JSON data and store the information in aqi_data and weather_data
sl_status_t parse_jsonString(char *json_str,
                             aqi_data_t *aqi_data,
                             weather_data_t *weather_data,
                             uint8_t parse);

// Function to get the AQI category
const char *get_aqi_category(uint8_t aqi);

#endif /* SL_OWMHTTP_H_ */

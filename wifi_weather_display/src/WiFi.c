/*
 * @file wi-Fi.c
 * @brief Top level Wi-Fi functions
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

#include "WiFi.h"

static const sl_wifi_device_configuration_t client_configuration = {
  .boot_option = LOAD_NWP_FW,
  .mac_address = NULL,
  .band = SL_SI91X_WIFI_BAND_2_4GHZ,
  .boot_config = { .oper_mode = SL_SI91X_CLIENT_MODE,
                   .coex_mode = SL_SI91X_WLAN_ONLY_MODE,
                   .feature_bit_map =
                     (SL_SI91X_FEAT_SECURITY_PSK | SL_SI91X_FEAT_AGGREGATION),
                   .tcp_ip_feature_bit_map =
                     (SL_SI91X_TCP_IP_FEAT_DHCPV4_CLIENT
                      | SL_SI91X_TCP_IP_FEAT_HTTP_CLIENT
                      | SL_SI91X_TCP_IP_FEAT_DNS_CLIENT
                      | SL_SI91X_TCP_IP_FEAT_SSL
                      | SL_SI91X_TCP_IP_FEAT_SNTP_CLIENT),
                   .custom_feature_bit_map =
                     (SL_SI91X_CUSTOM_FEAT_EXTENTION_VALID
                      | SL_SI91X_CUSTOM_FEAT_ASYNC_CONNECTION_STATUS
                      | SL_SI91X_TCP_IP_FEAT_EXTENSION_VALID),
                   .ext_custom_feature_bit_map =
                     (SL_SI91X_EXT_FEAT_SSL_VERSIONS_SUPPORT
                      | SL_SI91X_EXT_FEAT_XTAL_CLK
                      | MEMORY_CONFIG
                      | SL_SI91X_EXT_FEAT_RSA_KEY_WITH_4096_SUPPORT
                      | SL_SI91X_EXT_FEAT_SSL_CERT_WITH_4096_KEY_SUPPORT
#ifdef SLI_SI917
                      |
                      SL_SI91X_EXT_FEAT_FRONT_END_SWITCH_PINS_ULP_GPIO_4_5_0
#endif
                     ),
                   .bt_feature_bit_map = 0,
                   .ext_tcp_ip_feature_bit_map =
                     (SL_SI91X_EXT_TCP_IP_WINDOW_SCALING
                      | SL_SI91X_EXT_TCP_IP_TOTAL_SELECTS(10)
                      | SL_SI91X_EXT_TCP_IP_FEAT_SSL_THREE_SOCKETS
                      | SL_SI91X_EXT_TCP_IP_FEAT_SSL_MEMORY_CLOUD),
                   .ble_feature_bit_map = 0,
                   .ble_ext_feature_bit_map = 0,
                   .config_feature_bit_map = 0 }
};

sl_status_t WiFi_init(void)
{
  sl_status_t status = SL_STATUS_OK;

  status = sl_net_init(SL_NET_WIFI_CLIENT_INTERFACE,
                       &client_configuration,
                       NULL,
                       NULL);
  if ((status != SL_STATUS_OK) && (status != SL_STATUS_ALREADY_INITIALIZED)) {
    return status;
  }

  status = sl_net_up(SL_NET_WIFI_CLIENT_INTERFACE,
                     SL_NET_DEFAULT_WIFI_CLIENT_PROFILE_ID);
  if (status != SL_STATUS_OK) {
    return status;
  }

  return status;
}

sl_status_t WiFi_deinit(void)
{
  sl_status_t status = SL_STATUS_OK;
  status = sl_net_deinit(SL_NET_WIFI_CLIENT_INTERFACE);
  if (status != SL_STATUS_OK) {
    return status;
  }
  return status;
}

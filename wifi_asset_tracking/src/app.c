/***************************************************************************/ /**
 * @file app.c
 * @brief Top level application functions
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
#include "sl_wifi_asset_tracking_app.h"
#include "app.h"

/*******************************************************************************
 * Initialize application.
 ******************************************************************************/
void app_init(void)
{
  if (SL_STATUS_OK != sl_start_wifi_asset_tracking_application()) {
    printf("\r\napp_init : failed to start wi-fi asset tracking application\r\n");
  }
}

/*******************************************************************************
 * App ticking function.
 ******************************************************************************/
void app_process_action(void)
{
}

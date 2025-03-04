/***************************************************************************/ /**
 * @file app.h
 * @brief Top level application functions
 *******************************************************************************
 * # License
 * <b>Copyright 2020 Silicon Laboratories Inc. www.silabs.com</b>
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

#ifndef APP_H
#define APP_H

typedef enum DISPLAY_STATE
{
  INITIALIZATION = 0u,
  DISP_TIME = 1u,
  DISP_DATE = 2u,
  DISP_OUTDOOR_CLIMATE = 3u,
  DISP_INDOOR_CLIMATE = 4u,
  DISP_WEATHER_REPORT = 5u,
  DISP_AQI_REPORT = 6u,
} DISPLAY_STATE_T;

/***************************************************************************/ /**
 * Initialize application.
 ******************************************************************************/
void app_init(void);

/***************************************************************************/ /**
 * App ticking function.
 ******************************************************************************/
void app_process_action(void);

#endif // APP_H

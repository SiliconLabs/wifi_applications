/*
 * @file display.h
 * @brief Top level display functions
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

#ifndef DISPLAY_H_
#define DISPLAY_H_

#include "glib.h"
#include "sl_memlcd.h"
#include "dmd.h"
#include "RTE_Device_917.h"
#include "sl_status.h"
#include "rsi_ccp_user_config.h"
#include "stdio.h"
#include "stdlib.h"
#include <stdint.h>
#include "sl_assert.h"
#include "sl_status.h"
#include "sl_constants.h"
#include "sl_owmhttp.h"
#include "sl_wd_ntp.h"
#include "ctype.h"

#define SL_BOARD_ENABLE_DISPLAY_PIN  0
#define SL_BOARD_ENABLE_DISPLAY_PORT 0

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

// Initialize the MEMLCD
sl_status_t sl_display_init(GLIB_Context_t *glibContextText,
                            GLIB_Context_t *glibContextNumbers);

// Display time on the MEMLCD
sl_status_t sl_display_time(GLIB_Context_t *glibContextText,
                            GLIB_Context_t *glibContextNumbers,
                            sl_calendar_datetime_config_t *datetime);

// Display Date on the MEMLCD
sl_status_t sl_display_date(GLIB_Context_t *glibContextText,
                            GLIB_Context_t *glibContextNumbers,
                            sl_calendar_datetime_config_t *datetime);

// Display the Outdoor Climate information on the MEMLCD
sl_status_t sl_display_outdoorClimate(GLIB_Context_t *glibContextText,
                                      GLIB_Context_t *glibContextNumbers,
                                      weather_data_t *weather_data);

// Display the Indoor Climate information on the MEMLCD
sl_status_t sl_display_indoorClimate(GLIB_Context_t *glibContextText,
                                     GLIB_Context_t *glibContextNumbers,
                                     weather_data_t *weather_data);

// Display the Weather Report on the MEMLCD
sl_status_t sl_display_weatherReport(GLIB_Context_t *glibContextText,
                                     GLIB_Context_t *glibContextNumbers,
                                     weather_data_t *weather_data);

// Display the AQI Report on the MEMLCD
sl_status_t sl_display_aqiReport(GLIB_Context_t *glibContextText,
                                 GLIB_Context_t *glibContextNumbers,
                                 aqi_data_t *aqi_data);

#endif /* DISPLAY_H_ */

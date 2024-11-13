
/*
 * @file display.c
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

#include "display.h"

// Initialize the MEMLCD
sl_status_t sl_display_init(GLIB_Context_t *glibContextText,
                            GLIB_Context_t *glibContextNumbers)
{
  sl_status_t status;

  /* Enabling MEMLCD display */
  sl_memlcd_display_enable();

  /* Initialize the DMD support for memory lcd display */
  status = DMD_init(0);
  EFM_ASSERT(status == DMD_OK);

  /* Initialize the glib context */
  status = GLIB_contextInit(glibContextText);
  EFM_ASSERT(status == GLIB_OK);

  glibContextText->backgroundColor = White;
  glibContextText->foregroundColor = Black;

  /* Initialize the glib context */
  status = GLIB_contextInit(glibContextNumbers);
  EFM_ASSERT(status == GLIB_OK);

  glibContextNumbers->backgroundColor = White;
  glibContextNumbers->foregroundColor = Black;

  return SL_STATUS_OK;
}

// Display time on the MEMLCD
sl_status_t sl_display_time(GLIB_Context_t *glibContextText,
                            GLIB_Context_t *glibContextNumbers,
                            sl_calendar_datetime_config_t *datetime)
{
  GLIB_clear(glibContextText);
  GLIB_clear(glibContextNumbers);

  GLIB_setFont(glibContextText, (GLIB_Font_t *)&GLIB_FontNormal8x8);
  GLIB_setFont(glibContextNumbers, (GLIB_Font_t *)&GLIB_FontNumber16x20);

  char time[10] = "";
  char sec[5] = "";
  sprintf(time, "%.2d:%.2d", datetime->Hour, datetime->Minute);
  sprintf(sec, "%.2d", datetime->Second);

  GLIB_drawStringOnLine(glibContextNumbers,
                        time,
                        2,
                        GLIB_ALIGN_CENTER,
                        0,
                        0,
                        true);
  GLIB_drawStringOnLine(glibContextText, sec, 8, GLIB_ALIGN_CENTER, 0, 0, true);

  DMD_updateDisplay();

  return SL_STATUS_OK;
}

// Display Date on the MEMLCD
sl_status_t sl_display_date(GLIB_Context_t *glibContextText,
                            GLIB_Context_t *glibContextNumbers,
                            sl_calendar_datetime_config_t *datetime)
{
  GLIB_clear(glibContextText);
  GLIB_clear(glibContextNumbers);

  GLIB_setFont(glibContextText, (GLIB_Font_t *)&GLIB_FontNormal8x8);
  GLIB_setFont(glibContextNumbers, (GLIB_Font_t *)&GLIB_FontNumber16x20);

  char date[15] = "";
  sprintf(date, "%.2d:%.2d:%.2d", datetime->Day, datetime->Month,
          datetime->Year);

  GLIB_drawStringOnLine(glibContextText,
                        get_day_of_week(datetime->DayOfWeek),
                        3,
                        GLIB_ALIGN_LEFT,
                        0,
                        0,
                        true);
  GLIB_drawStringOnLine(glibContextNumbers,
                        date,
                        2,
                        GLIB_ALIGN_CENTER,
                        0,
                        0,
                        true);
  GLIB_drawStringOnLine(glibContextText,
                        get_month_name(datetime->Month),
                        8,
                        GLIB_ALIGN_RIGHT,
                        0,
                        0,
                        true);

  DMD_updateDisplay();

  return SL_STATUS_OK;
}

// Display the Outdoor Climate information on the MEMLCD
sl_status_t sl_display_outdoorClimate(GLIB_Context_t *glibContextText,
                                      GLIB_Context_t *glibContextNumbers,
                                      weather_data_t *weather_data)
{
  GLIB_clear(glibContextText);
  GLIB_clear(glibContextNumbers);

  GLIB_setFont(glibContextText, (GLIB_Font_t *)&GLIB_FontNormal8x8);
  GLIB_setFont(glibContextNumbers, (GLIB_Font_t *)&GLIB_FontNumber16x20);

  char temp[5], humi[5];
  sprintf(temp, "%.2d", (uint8_t)weather_data->temp);
  sprintf(humi, "%.2d", (uint8_t)weather_data->humidity);

  GLIB_drawStringOnLine(glibContextText,
                        "Outdoor Climate",
                        1,
                        GLIB_ALIGN_CENTER,
                        0,
                        0,
                        true);
  GLIB_drawStringOnLine(glibContextText, "T:", 4, GLIB_ALIGN_LEFT, 32, 0, true);
  GLIB_drawStringOnLine(glibContextNumbers,
                        temp,
                        1,
                        GLIB_ALIGN_CENTER,
                        0,
                        7,
                        true);
  GLIB_drawStringOnLine(glibContextText, "H:", 9, GLIB_ALIGN_LEFT, 32, 0, true);
  GLIB_drawStringOnLine(glibContextNumbers,
                        humi,
                        3,
                        GLIB_ALIGN_CENTER,
                        0,
                        7,
                        true);

  DMD_updateDisplay();

  return SL_STATUS_OK;
}

// Display the Indoor Climate information on the MEMLCD
sl_status_t sl_display_indoorClimate(GLIB_Context_t *glibContextText,
                                     GLIB_Context_t *glibContextNumbers,
                                     weather_data_t *weather_data)
{
  GLIB_clear(glibContextText);
  GLIB_clear(glibContextNumbers);

  GLIB_setFont(glibContextText, (GLIB_Font_t *)&GLIB_FontNormal8x8);
  GLIB_setFont(glibContextNumbers, (GLIB_Font_t *)&GLIB_FontNumber16x20);

  char temp[5], humi[5];
  sprintf(temp, "%.2d", (uint8_t)weather_data->local_temperature);
  sprintf(humi, "%.2d", (uint8_t)weather_data->local_humidity);

  GLIB_drawStringOnLine(glibContextText,
                        "Indoor Climate",
                        1,
                        GLIB_ALIGN_CENTER,
                        0,
                        0,
                        true);
  GLIB_drawStringOnLine(glibContextText, "T:", 4, GLIB_ALIGN_LEFT, 32, 0, true);
  GLIB_drawStringOnLine(glibContextNumbers,
                        temp,
                        1,
                        GLIB_ALIGN_CENTER,
                        0,
                        7,
                        true);
  GLIB_drawStringOnLine(glibContextText, "H:", 9, GLIB_ALIGN_LEFT, 32, 0, true);
  GLIB_drawStringOnLine(glibContextNumbers,
                        humi,
                        3,
                        GLIB_ALIGN_CENTER,
                        0,
                        7,
                        true);

  DMD_updateDisplay();

  return SL_STATUS_OK;
}

// Display the Weather Report on the MEMLCD
sl_status_t sl_display_weatherReport(GLIB_Context_t *glibContextText,
                                     GLIB_Context_t *glibContextNumbers,
                                     weather_data_t *weather_data)
{
  UNUSED_PARAMETER(glibContextNumbers);
  GLIB_clear(glibContextText);

  GLIB_setFont(glibContextText, (GLIB_Font_t *)&GLIB_FontNormal8x8);

  char temp[25], humidity[25], feels_like[25], rain[25];

  sprintf(temp, "Temp: %.1f°C\n", weather_data->temp);
  sprintf(feels_like, "Feels Like: %.1f°C\n", weather_data->feels_like);
  sprintf(humidity, "Humidity: %d%%\n", weather_data->humidity);
  sprintf(rain, "Rain(1h): %.1fmm\n", weather_data->rain);

  weather_data->description[0] = toupper(weather_data->description[0]);

  GLIB_drawStringOnLine(glibContextText,
                        "Weather Report",
                        0,
                        GLIB_ALIGN_CENTER,
                        0,
                        0,
                        true);
  GLIB_drawStringOnLine(glibContextText,
                        weather_data->description,
                        2,
                        GLIB_ALIGN_CENTER,
                        0,
                        0,
                        true);
  GLIB_drawStringOnLine(glibContextText,
                        feels_like,
                        4,
                        GLIB_ALIGN_LEFT,
                        0,
                        0,
                        true);
  GLIB_drawStringOnLine(glibContextText, temp, 6, GLIB_ALIGN_LEFT, 0, 0, true);
  GLIB_drawStringOnLine(glibContextText,
                        humidity,
                        8,
                        GLIB_ALIGN_LEFT,
                        0,
                        0,
                        true);
  GLIB_drawStringOnLine(glibContextText, rain, 10, GLIB_ALIGN_LEFT, 0, 0, true);

  DMD_updateDisplay();

  return SL_STATUS_OK;
}

// Display the AQI Report on the MEMLCD
sl_status_t sl_display_aqiReport(GLIB_Context_t *glibContextText,
                                 GLIB_Context_t *glibContextNumbers,
                                 aqi_data_t *aqi_data)
{
  UNUSED_PARAMETER(glibContextNumbers);

  GLIB_clear(glibContextText);

  GLIB_setFont(glibContextText, (GLIB_Font_t *)&GLIB_FontNormal8x8);

  char aqi[30], ozone[20], pm2[20], pm10[20], co[20];

  sprintf(aqi, "AQI: %s\n", get_aqi_category(aqi_data->aqi));
  sprintf(ozone, "Ozone(O3): %.1f\n", aqi_data->ozone);
  sprintf(pm2, "PM2.5: %.1f\n", aqi_data->pm2_5);
  sprintf(pm10, "PM10: %.1f\n", aqi_data->pm10);
  sprintf(co, "CO: %.1f\n", aqi_data->co);

  GLIB_drawStringOnLine(glibContextText,
                        "AQI Report",
                        0,
                        GLIB_ALIGN_CENTER,
                        0,
                        0,
                        true);
  GLIB_drawStringOnLine(glibContextText, aqi, 2, GLIB_ALIGN_LEFT, 0, 0, true);
  GLIB_drawStringOnLine(glibContextText, ozone, 4, GLIB_ALIGN_LEFT, 0, 0, true);
  GLIB_drawStringOnLine(glibContextText, pm2, 6, GLIB_ALIGN_LEFT, 0, 0, true);
  GLIB_drawStringOnLine(glibContextText, pm10, 8, GLIB_ALIGN_LEFT, 0, 0, true);
  GLIB_drawStringOnLine(glibContextText, co, 10, GLIB_ALIGN_LEFT, 0, 0, true);

  DMD_updateDisplay();

  return SL_STATUS_OK;
}

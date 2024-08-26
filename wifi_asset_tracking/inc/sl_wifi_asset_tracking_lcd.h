/***************************************************************************/ /**
 * @file sl_wifi_asset_tracking_lcd.h
 * @brief Wi-Fi asset tracking application LCD interface
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

#ifndef SL_WIFI_ASSET_TRACKING_LCD_H
#define SL_WIFI_ASSET_TRACKING_LCD_H

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
 ***************************  Defines / Macros  ********************************
 ******************************************************************************/
#define MAX_LCD_SUBSTRING_LENGTH           20    ///< Max length that can be print on one line of LCD
#define MAX_LCD_LINES                      12    ///< Maximum no. of lines that can be print on LCD without losing data

/*******************************************************************************
 **************************   ENUMS and Structures   ***************************
 ******************************************************************************/

// -----------------------------------------------------------------------------
// Prototypes

/**************************************************************************/ /**
 * @brief send string message to LCD queue
 * @param[in] lcd_msg_index : Index number of string message to be print as per
 * sl_wifi_asset_tracking_lcd_strings array.
 *****************************************************************************/
void sl_wifi_asset_tracking_lcd_print(uint8_t lcd_msg_index);

/***************************************************************************/ /**
 * Create LCD task for wi-fi asset tracking example
 ******************************************************************************/
void sl_wifi_asset_tracking_lcd_task();

#ifdef __cplusplus
}
#endif

#endif /* SL_WIFI_ASSET_TRACKING_LCD_H_ */

/******************************************************************************/
/* EOF                                                                        */
/******************************************************************************/

/***************************************************************************/ /**
 * @file sl_wifi_asset_tracking_lcd.c
 * @brief wi-fi asset tracking LCD interface functions
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
#include <glib.h>
#include <sl_memlcd.h>
#include <dmd.h>
#include <RTE_Device_917.h>
#include <rsi_ccp_user_config.h>
#include <sl_status.h>
#include <sl_assert.h>
#include <FreeRTOS.h>
#include <task.h>
#include <semphr.h>
#include <sl_wifi_asset_tracking_lcd.h>
#include <sl_wifi_asset_tracking_app.h>
#include <sl_wifi_asset_tracking_demo_config.h>

static uint8_t sl_lcd_current_line = 0; /// Current LCD line number
static GLIB_Context_t sl_lcd_glib_context; /// LCD glib context

/**************************************************************************/ /**
 * @brief Print provided string on LCD after breaking provided string into sub
 * strings
 * @param[in] pString : Pointer to the string that is to be print on LCD
 * @return none
 *****************************************************************************/
static void sl_si91x_lcd_print(char *pString);

/**************************************************************************/ /**
 * @brief  Clear present LCD content
 * @param none
 * @return none
 *****************************************************************************/
static void sl_si91x_clear_lcd(void);

/**************************************************************************/ /**
 * @brief Breaks main string into sub strings
 * @param[in]  str : Pointer to the string that is to be print on LCD.
 * @param[out] sub_str_buf : Pointer to store sub string.
 * @param[in]  start : Start index from where string to be break.
 * @param[in]  end : Index up-to which string to be break.
 * @return The following values are returned:
 * - \ref SL_STATUS_OK on success.
 * - \ref SL_STATUS_FAIL on failure.
 ******************************************************************************/
static sl_status_t sl_si91x_lcd_substring(const char *str,
                                          char *sub_str_buf,
                                          int start,
                                          int end);

/**************************************************************************/ /**
 * @brief  Function to clear present LCD content
 * @return The following values are returned:
 * - \ref SL_STATUS_OK on success.
 * - \ref SL_STATUS_FAIL on failure.
 *****************************************************************************/
static void sl_si91x_clear_lcd(void)
{
  /// Clears the clipping region by filling it with the background color of the GLIB_Context_t
  GLIB_clearRegion(&sl_lcd_glib_context);

  /// Reset the display driver clipping area to the whole display
  GLIB_resetDisplayClippingArea(&sl_lcd_glib_context);

  /// Apply the clipping region from the GLIB_Context_t in the DMD driver
  GLIB_applyClippingRegion(&sl_lcd_glib_context);
}

/**************************************************************************/ /**
 * @brief  Function to initialize LCD display
 * @return The following values are returned:
 * - \ref SL_STATUS_OK on success.
 * - \ref SL_STATUS_FAIL on failure.
 *****************************************************************************/
static sl_status_t sl_wifi_asset_tracking_lcd_init(void)
{
  sl_status_t status;

  /// Enabling MEMLCD display
  sl_memlcd_display_enable();

  /// Initialize the DMD support for memory LCD display
  status = DMD_init(0);
  if (DMD_OK != status) {
    printf(
      "\r\nsl_wifi_asset_tracking_lcd_init : Failed to initialize DMD LCD\r\n");
    goto error;
  }

  /// Initialize the GLIB context
  status = GLIB_contextInit(&sl_lcd_glib_context);
  if (GLIB_OK != status) {
    printf(
      "\r\nsl_wifi_asset_tracking_lcd_init : Failed to initialize GLIB context for LCD\r\n");
    goto error;
  }

  /// Set LCD background and foreground color
  sl_lcd_glib_context.backgroundColor = White;
  sl_lcd_glib_context.foregroundColor = Black;

  /// Fill LCD with background color
  GLIB_clear(&sl_lcd_glib_context);

  /// Use Narrow font
  GLIB_setFont(&sl_lcd_glib_context, (GLIB_Font_t *)&GLIB_FontNarrow6x8);

  return SL_STATUS_OK;

  error:
  return SL_STATUS_FAIL;
}

/**************************************************************************/ /**
 * @brief  Function to print passed string on LCD
 * param[in] print_str : pointer to a string buffer.
 * @return The following values are returned:
 * - \ref SL_STATUS_OK on success.
 * - \ref SL_STATUS_FAIL on failure.
 *****************************************************************************/
static void sl_si91x_lcd_print_string(char *print_str)
{
  /// Print string to LCD
  sl_si91x_lcd_print(print_str);

  /// Update LCD Content
  DMD_updateDisplay();
}

/**************************************************************************/ /**
 * @brief Utility function to breaks string into substrings
 * @param[in]  str : Pointer to the string that is drawn.
 * @param[out] sub_str_buf : Pointer to the output substring that is created from main string.
 * @param[in]  start : Start index from where string to be break.
 * @param[in]  end : Index up-to which substring to be break
 * @return The following values are returned:
 * -  \ref SL_STATUS_OK On success
 * -  \ref SL_STATUS_FAIL - On initialization failure
 ******************************************************************************/
static sl_status_t sl_si91x_lcd_substring(const char *str,
                                          char *sub_str_buf,
                                          int start,
                                          int end)
{
  uint8_t substr_length;
  uint8_t length = strlen(str);

  /// Clear sub-string buffer
  memset(sub_str_buf, '\0', strlen(sub_str_buf));

  /// Validate input
  if ((start < 0) || (end < 0) || (start >= length) || (end >= length)
      || (start > end)) {
    return SL_STATUS_FAIL;   /// Return failure for invalid input
  }

  /// Find out sub-string length
  substr_length = end - start + 1;

  /// Copy substring data
  for (int i = 0; i < substr_length; ++i) {
    sub_str_buf[i] = str[start + i];
  }

  /// Null-terminate the substring
  sub_str_buf[substr_length] = '\0';

  return SL_STATUS_OK;
}

/*******************************************************************************
 * Function to print string on LCD and clear LCD if MAX_LCD_LINES are reached
 ******************************************************************************/
static void sl_si91x_lcd_print(char *pString)
{
  sl_status_t status;
  uint8_t count = 0;

  /// Create character buffer of size MAX_LCD_SUBSTRING_LENGTH to store substring
  char sub_str_buffer[MAX_LCD_SUBSTRING_LENGTH];

  /// Start index from where string to be divided
  uint8_t start = 0;

  /// End index up-to which string to be divided
  uint8_t end = MAX_LCD_SUBSTRING_LENGTH - 1;

  /// Storing main string length into variable
  uint8_t print_str_len = strlen(pString);

  if (print_str_len != 0) {
    /// No. of lines to be print on LCD
    uint8_t no_of_lines = print_str_len / MAX_LCD_SUBSTRING_LENGTH;

    if ((print_str_len % MAX_LCD_SUBSTRING_LENGTH) > 0) {
      ++no_of_lines;
    }

    /// If line-count is greater than no. of lines left in LCD then,
    /// clear LCD and again starts from line 0
    if (no_of_lines > (MAX_LCD_LINES - sl_lcd_current_line)) {
      /// Update LCD current line variable
      sl_lcd_current_line = 0;

      /// Clear present LCD content
      sl_si91x_clear_lcd();
    }

    while (count < no_of_lines) {
      /// Checking if substring character length is MAX_LCD_SUBSTRING_LENGTH
      /// Substring character length is less than MAX_LCD_SUBSTRING_LENGTH
      if ((count == no_of_lines - 1)
          && (print_str_len % MAX_LCD_SUBSTRING_LENGTH > 0)) {
        end = print_str_len - 1;
        status = sl_si91x_lcd_substring(pString, sub_str_buffer, start, end);

        if (SL_STATUS_OK == status) {
          /// Draws a string on the specified line on the display
          GLIB_drawStringOnLine(&sl_lcd_glib_context,
                                sub_str_buffer,
                                sl_lcd_current_line++,
                                GLIB_ALIGN_LEFT,
                                5,
                                5,
                                true);
        }
      } else {   /// Substring character length is equal to MAX_LCD_SUBSTRING_LENGTH
        status = sl_si91x_lcd_substring(pString, sub_str_buffer, start, end);

        if (SL_STATUS_OK == status) {
          GLIB_drawStringOnLine(&sl_lcd_glib_context,
                                sub_str_buffer,
                                sl_lcd_current_line++,
                                GLIB_ALIGN_LEFT,
                                5,
                                5,
                                true);
        }
      }

      /// Incrementing Start and end index for next 20 index
      start += MAX_LCD_SUBSTRING_LENGTH;
      end += MAX_LCD_SUBSTRING_LENGTH;
      ++count;
    }
  } else {
    printf("\r\nsl_si91x_lcd_print : Invalid string passed to print on LCD\r\n");
  }
}

/*******************************************************************************
 * Function to send string message into LCD data queue
 ******************************************************************************/
void sl_wifi_asset_tracking_lcd_print(
  sl_siwx917_wifi_asset_tracking_lcd_logs_indexing_e lcd_msg_index)
{
  sl_wifi_asset_tracking_lcd_queue_data_t lcd_string;

  if (!sl_get_wifi_asset_tracking_status()->lcd_init_status) {
#if DEMO_CONFIG_DEBUG_LOGS
    printf(
      "\r\nsl_wifi_asset_tracking_lcd_print : LCD module is not in initialized state, so returning\r\n");
#endif
    return;
  }

  if (sl_get_wifi_asset_tracking_status()->app_shutdown) {
    printf(
      "\r\nsl_wifi_asset_tracking_lcd_print : application is shutdown, so returning\r\n");
    return;
  }

  if ((INDEX_SENSOR_SHUTDOWN == lcd_msg_index)
      || (INDEX_WIFI_SHUTDOWN == lcd_msg_index)
      || (INDEX_CLOUD_SHUTDOWN == lcd_msg_index)) {
    sl_get_wifi_asset_tracking_status()->app_shutdown = true;
  }

  strcpy((char *)lcd_string.lcd_buffer,
         sl_get_wifi_asset_tracking_lcd_string()[lcd_msg_index]);

  if (errQUEUE_FULL
      == xQueueSend(sl_get_wifi_asset_tracking_resource()->lcd_queue_handler,
                    &lcd_string,
                    0)) {
    /// Receive LCD data as LCD data queue is full
    xQueueReceive(sl_get_wifi_asset_tracking_resource()->lcd_queue_handler,
                  &lcd_string,
                  0);

    printf(
      "\r\nsl_wifi_asset_tracking_lcd_print : received LCD data as LCD data queue is full\r\n");

    /// re-write the content
    strcpy((char *)lcd_string.lcd_buffer,
           sl_get_wifi_asset_tracking_lcd_string()[lcd_msg_index]);

    /// again send latest LCD data to LCD data queue
    xQueueSend(sl_get_wifi_asset_tracking_resource()->lcd_queue_handler,
               &lcd_string,
               0);
  }

#if DEMO_CONFIG_DEBUG_LOGS
  printf(
    "\r\nsl_wifi_asset_tracking_lcd_print : LCD string message is sent to LCD data queue\r\n");
#endif

  vTaskResume(sl_get_wifi_asset_tracking_resource()->task_list.lcd_task_handler);
}

/******************************************************************************
 *  Create LCD task for wi-fi asset tracking example
 *****************************************************************************/
void sl_wifi_asset_tracking_lcd_task()
{
  sl_wifi_asset_tracking_lcd_queue_data_t buffer;
  char string_buffer[MAX_LCD_STRING_SIZE];

  /// initialize LCD
  if (SL_STATUS_OK != sl_wifi_asset_tracking_lcd_init()) {
    sl_get_wifi_asset_tracking_status()->lcd_init_status = false;

    printf(
      "\r\nlcd_task : failed to initialize LCD therefore deleting LCD task!!!\r\n");
    vTaskDelete(
      sl_get_wifi_asset_tracking_resource()->task_list.lcd_task_handler);
  }

  sl_wifi_asset_tracking_lcd_print(INDEX_APP_START);
  sl_wifi_asset_tracking_lcd_print(INDEX_BLANK_LINE);

  while (1) {
    if (QUEUE_EMPTY
        == uxQueueMessagesWaiting(sl_get_wifi_asset_tracking_resource()->
                                  lcd_queue_handler)) {
#if DEMO_CONFIG_DEBUG_LOGS
      printf("\r\nlcd_task : suspend LCD task as LCD data queue is empty\r\n");
#endif /// < DEMO_CONFIG_DEBUG_LOGS
      vTaskSuspend(
        sl_get_wifi_asset_tracking_resource()->task_list.lcd_task_handler);
    } else {
      xQueueReceive(sl_get_wifi_asset_tracking_resource()->lcd_queue_handler,
                    &buffer,
                    0);

      strcpy(string_buffer, (char *)buffer.lcd_buffer);

#if DEMO_CONFIG_DEBUG_LOGS
      printf("\r\nlcd_task : LCD is updated with message : %s\r\n",
             buffer.lcd_buffer);
#endif /// < DEMO_CONFIG_DEBUG_LOGS

      sl_si91x_lcd_print_string(string_buffer);
    }
  }
}

/*
 * @file sensor.c
 * @brief Top level sensor functions
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

#include "sensor.h"

#define SL_BOARD_ENABLE_DISPLAY_PIN  0
#define SL_BOARD_ENABLE_DISPLAY_PORT 0

#define TX_THRESHOLD                 0 // tx threshold value
#define RX_THRESHOLD                 0 // rx threshold value
#define I2C                          SL_I2C2 // I2C 2 instance
#define USER_REG_1                   0xBA // writing data into user register
#define DELAY_PERIODIC_MS1           2000 // sleeptimer1 periodic timeout in ms
#define MODE_0                       0 // Initializing GPIO MODE_0 value
#define OUTPUT_VALUE                 1 // GPIO output value

typedef sl_i2c_config_t sl_i2c_configuration_t;

void delay(uint32_t idelay)
{
  for (uint32_t x = 0; x < 4600 * idelay; x++) // 1.002ms delay
  {
    __NOP();
  }
}

/*******************************************************************************
 * RHT example initialization function
 ******************************************************************************/
sl_status_t si70xx_init(sl_i2c_instance_t i2c_instance, uint8_t i2c_address)
{
  //  UNUSED_PARAMETER(i2c_instance);
  UNUSED_PARAMETER(i2c_address);

  sl_status_t status;

  sl_i2c_configuration_t i2c_config;

  i2c_config.mode = SL_I2C_LEADER_MODE;
  i2c_config.transfer_type = SL_I2C_USING_INTERRUPT;
  i2c_config.operating_mode = SL_I2C_STANDARD_MODE;
  //  i2c_config.i2c_callback   = i2c_leader_callback;
  do
  {
#if defined(SENSOR_ENABLE_GPIO_MAPPED_TO_UULP)
    if (sl_si91x_gpio_driver_get_uulp_npss_pin(SENSOR_ENABLE_GPIO_PIN) != 1) {
      // Enable GPIO ULP_CLK
      status = sl_si91x_gpio_driver_enable_clock(
        (sl_si91x_gpio_select_clock_t)ULPCLK_GPIO);
      if (status != SL_STATUS_OK) {
        printf("sl_si91x_gpio_driver_enable_clock, Error code: %lu", status);
        break;
      }

      // printf("GPIO driver clock enable is successful \n");

      // Set NPSS GPIO pin MUX
      status = sl_si91x_gpio_driver_set_uulp_npss_pin_mux(
        SENSOR_ENABLE_GPIO_PIN,
        NPSS_GPIO_PIN_MUX_MODE0);
      if (status != SL_STATUS_OK) {
        printf("sl_si91x_gpio_driver_set_uulp_npss_pin_mux, Error code: %lu",
               status);
        break;
      }

      // printf("GPIO driver uulp pin mux selection is successful \n");

      // Set NPSS GPIO pin direction
      status =
        sl_si91x_gpio_driver_set_uulp_npss_direction(SENSOR_ENABLE_GPIO_PIN,
                                                     (sl_si91x_gpio_direction_t)GPIO_OUTPUT);
      if (status != SL_STATUS_OK) {
        printf("sl_si91x_gpio_driver_set_uulp_npss_direction, Error code: %lu",
               status);
        break;
      }

      // printf("GPIO driver uulp pin direction selection is successful \n");

      // Set UULP GPIO pin
      status = sl_si91x_gpio_driver_set_uulp_npss_pin_value(
        SENSOR_ENABLE_GPIO_PIN,
        SET);
      if (status != SL_STATUS_OK) {
        printf("sl_si91x_gpio_driver_set_uulp_npss_pin_value, Error code: %lu",
               status);
        break;
      }

      // printf("GPIO driver set uulp pin value is successful \n");
    }
#else
    sl_gpio_t sensor_enable_port_pin =
    { SENSOR_ENABLE_GPIO_PORT, SENSOR_ENABLE_GPIO_PIN };
    uint8_t pin_value;

    status = sl_gpio_driver_get_pin(&sensor_enable_port_pin, &pin_value);
    if (status != SL_STATUS_OK) {
      printf("sl_gpio_driver_get_pin, Error code: %lu", status);
      break;
    }
    if (pin_value != 1) {
      // Enable GPIO CLK
#ifdef SENSOR_ENABLE_GPIO_MAPPED_TO_ULP
      status = sl_si91x_gpio_driver_enable_clock(
        (sl_si91x_gpio_select_clock_t)ULPCLK_GPIO);
#else
      status = sl_si91x_gpio_driver_enable_clock(
        (sl_si91x_gpio_select_clock_t)M4CLK_GPIO);
#endif
      if (status != SL_STATUS_OK) {
        printf("sl_si91x_gpio_driver_enable_clock, Error code: %lu", status);
        break;
      }
      printf("GPIO driver clock enable is successful \n");

      // Set the pin mode for GPIO pins.
      status = sl_gpio_driver_set_pin_mode(&sensor_enable_port_pin,
                                           MODE_0,
                                           OUTPUT_VALUE);
      if (status != SL_STATUS_OK) {
        printf("sl_gpio_driver_set_pin_mode, Error code: %lu", status);
        break;
      }
      printf("GPIO driver pin mode select is successful \n");
      // Select the direction of GPIO pin whether Input/ Output
      status = sl_si91x_gpio_driver_set_pin_direction(SENSOR_ENABLE_GPIO_PORT,
                                                      SENSOR_ENABLE_GPIO_PIN,
                                                      (sl_si91x_gpio_direction_t)GPIO_OUTPUT);
      if (status != SL_STATUS_OK) {
        printf("sl_si91x_gpio_driver_set_pin_direction, Error code: %lu",
               status);
        break;
      }
      // Set GPIO pin
      status = sl_gpio_driver_set_pin(&sensor_enable_port_pin); // Set ULP GPIO pin
      if (status != SL_STATUS_OK) {
        printf("sl_gpio_driver_set_pin, Error code: %lu", status);
        break;
      }
      printf("GPIO driver set pin value is successful \n");
    }
#endif

    /* Wait for sensor to become ready */
    delay(80);

    // Initialize I2C bus
    status = sl_i2c_driver_init(i2c_instance, &i2c_config);
    if (status != SL_I2C_SUCCESS) {
      printf("sl_i2c_driver_init : Invalid Parameters, Error Code: 0x%ld \n",
             status);
      break;
    }

    // configure the Tx and Rx thresholds
    status = sl_i2c_driver_configure_fifo_threshold(i2c_instance,
                                                    TX_THRESHOLD,
                                                    RX_THRESHOLD);
    if (status != SL_I2C_SUCCESS) {
      printf(
        "sl_i2c_driver_configure_fifo_threshold : Invalid Parameters, Error Code: 0x%ld \n",
        status);
      break;
    }
  } while (false);

  return SL_STATUS_OK;
}

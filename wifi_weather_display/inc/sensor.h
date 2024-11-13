/*
 * @file sensor.h
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

#ifndef SENSOR_H_
#define SENSOR_H_

#include "sl_si91x_driver_gpio.h"
#include "sl_si91x_si70xx.h"
#include "rsi_time_period.h"
#include "sl_constants.h"

// Delay function for the Si70xx initialization
void delay(uint32_t idelay);

// Initialize the Si70xx sensor
sl_status_t si70xx_init(sl_i2c_instance_t i2c_instance, uint8_t i2c_address);

#endif /* SENSOR_H_ */

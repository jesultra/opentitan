// Copyright lowRISC contributors.
// Licensed under the Apache License, Version 2.0, see LICENSE for details.
// SPDX-License-Identifier: Apache-2.0

#ifndef OPENTITAN_SW_DEVICE_EXAMPLE_SPI_DISPLAY_MAIN_DEMO_H_
#define OPENTITAN_SW_DEVICE_EXAMPLE_SPI_DISPLAY_MAIN_DEMO_H_

#include "context.h"
#include "sw/device/lib/base/status.h"
#include "sw/device/lib/dif/dif_gpio.h"
#include "sw/device/lib/dif/dif_i2c.h"
#include "sw/device/lib/dif/dif_spi_host.h"

status_t run_demo(dif_spi_host_t *spi_lcd, dif_spi_host_t *spi_flash,
                  dif_spi_device_handle_t *spid, dif_i2c_t *i2c,
                  dif_gpio_t *gpio, dif_aes_t *aes, display_pin_map_t dsp_pins,
                  btn_pin_map_t btn_pins, status_led_pin_map_t led_pins,
                  LCD_Orientation orientation);

status_t self_test(context_t *ctx);

#endif  // OPENTITAN_SW_DEVICE_EXAMPLE_SPI_DISPLAY_MAIN_DEMO_H_

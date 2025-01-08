// Copyright lowRISC contributors.
// Licensed under the Apache License, Version 2.0, see LICENSE for details.
// SPDX-License-Identifier: Apache-2.0
#ifndef LCD_ST_7735_FRACTAL
#define LCD_ST_7735_FRACTAL

#include "context.h"
#include "screen.h"
#include "sw/device/lib/base/status.h"

status_t run_aes(context_t *app);
status_t spi_passthrough_demo(context_t *ctx);
status_t i2c_eeprom(context_t *ctx);
status_t i2c_light_sensor(context_t *ctx);

#endif

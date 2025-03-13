// Copyright lowRISC contributors.
// Licensed under the Apache License, Version 2.0, see LICENSE for details.
// SPDX-License-Identifier: Apache-2.0

#include <stdint.h>

#include "btn.h"
#include "context.h"
#include "demos.h"
#include "screen.h"
//#include "sw/device/lib/crypto/include/hash.h"
#include "sw/device/lib/runtime/print.h"
#include "sw/device/lib/testing/i2c_testutils.h"
#include "sw/device/lib/testing/spi_flash_testutils.h"
#include "sw/device/lib/testing/test_framework/check.h"

enum {
  kProximitySensorAddr = 0x60,
  kLightSensorAddr = 0x44,
  kEepromAddr = 0xa0 >> 1,
  // Default timeout for i2c reads.
  kDefaultTimeoutMicros = 5000,
  // "Acknowledgement polling" timeout - used for waiting until EEPROM has
  // finished writing.
  kAckPollTimeoutMicros = 80000,
};

static status_t proximity_sensor_test(context_t *ctx);
static status_t light_sensor_test(context_t *ctx);
static status_t eeprom_test(context_t *ctx);
static status_t spi_flash_test(context_t *ctx);

status_t self_test(context_t *ctx) {
  size_t line = 0;
  lcd_st7735_clean(ctx->lcd);
  lcd_st7735_set_font_colors(ctx->lcd, RGBColorBlue, RGBColorWhite);
  screen_println(ctx->lcd, "Self test", alined_center, line++, true);
  lcd_st7735_set_font_colors(ctx->lcd, RGBColorWhite, RGBColorBlue);

  screen_println(ctx->lcd, "proximity..", alined_left, line, true);
  TRY(proximity_sensor_test(ctx));
  screen_println(ctx->lcd, "proximity..Ok", alined_left, line++, true);

  screen_println(ctx->lcd, "light_sensor..", alined_left, line, true);
  TRY(light_sensor_test(ctx));
  screen_println(ctx->lcd, "light_sensor..Ok", alined_left, line++, true);

  screen_println(ctx->lcd, "eeprom..", alined_left, line, true);
  TRY(eeprom_test(ctx));
  screen_println(ctx->lcd, "eeprom..Ok", alined_left, line++, true);

  screen_println(ctx->lcd, "spi_flash..", alined_left, line, true);
  TRY(spi_flash_test(ctx));
  screen_println(ctx->lcd, "spi_flash..Ok", alined_left, line++, true);

  busy_spin_micros(3 * 1000 * 1000);
  lcd_st7735_clean(ctx->lcd);
  return OK_STATUS();
}

static status_t proximity_sensor_test(context_t *ctx) {
  uint8_t pid[2] = {
      0x00,
  };
  const uint8_t kAddr[1] = {0x0c};
  TRY(i2c_testutils_write(ctx->i2c, kProximitySensorAddr, sizeof(kAddr), kAddr,
                          true));
  TRY(i2c_testutils_read(ctx->i2c, kProximitySensorAddr, sizeof(pid), pid,
                         kDefaultTimeoutMicros));
  LOG_INFO("pid: 0x%x%x", pid[0], pid[1]);
  if (pid[0] != 0x86 || pid[1] != 0x01) {
    return INTERNAL();
  }
  return OK_STATUS();
}

static status_t light_sensor_test(context_t *ctx) {
  uint8_t mid[3] = {
      0x00,
  };
  {
    const uint8_t kAddr[1] = {0x7e};
    TRY(i2c_testutils_write(ctx->i2c, kLightSensorAddr, sizeof(kAddr), kAddr,
                            true));
    TRY(i2c_testutils_read(ctx->i2c, kLightSensorAddr, sizeof(mid) - 1, mid,
                           kDefaultTimeoutMicros));
  }

  uint8_t pid[2] = {
      0x00,
  };
  {
    const uint8_t kAddr[1] = {0x7f};
    TRY(i2c_testutils_write(ctx->i2c, kLightSensorAddr, sizeof(kAddr), kAddr,
                            true));
    TRY(i2c_testutils_read(ctx->i2c, kLightSensorAddr, sizeof(pid), pid,
                           kDefaultTimeoutMicros));
  }
  LOG_INFO("mid: %s, pid: 0x%x%x", mid, pid[0], pid[1]);
  if (mid[0] != 'T' || mid[1] != 'I' || pid[0] != 0x30 || pid[1] != 0x01) {
    return INTERNAL();
  }
  return OK_STATUS();
}

static status_t poll_while_busy(dif_i2c_t *i2c) {
  return i2c_testutils_read(i2c, kEepromAddr, 0, NULL, kAckPollTimeoutMicros);
}

static status_t eeprom_test(context_t *ctx) {
  const uint8_t kAddr[1] = {0x00};
  uint8_t data[] = {kAddr[0], 'L', 'o', 'w', 'r', 'i', 's', 'c', 0};
  TRY(i2c_testutils_write(ctx->i2c, kEepromAddr, sizeof(data), data, false));
  TRY(poll_while_busy(ctx->i2c));

  uint8_t read_data[sizeof(data)] = {
      kAddr[0],
  };
  TRY(i2c_testutils_write(ctx->i2c, kEepromAddr, sizeof(kAddr), kAddr, true));
  TRY(i2c_testutils_read(ctx->i2c, kEepromAddr, sizeof(read_data) - 1,
                         &read_data[1], kDefaultTimeoutMicros));

  LOG_INFO("i2c_eeprom_read: %s", read_data);

  TRY_CHECK_ARRAYS_EQ(data, read_data, ARRAYSIZE(data));

  return OK_STATUS();
}

static status_t spi_flash_test(context_t *ctx) {
  TRY(dif_spi_host_output_set_enabled(ctx->spi_flash, true));
  spi_flash_testutils_jedec_id_t jdec;
  TRY(spi_flash_testutils_read_id(ctx->spi_flash, &jdec));
  LOG_INFO("did: 0x%x, mid: 0x%x", jdec.device_id, jdec.manufacturer_id);
  TRY_CHECK(jdec.manufacturer_id == 0xef && jdec.device_id == 0x1840);
  return OK_STATUS();
}

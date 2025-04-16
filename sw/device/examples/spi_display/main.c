// Copyright lowRISC contributors.
// Licensed under the Apache License, Version 2.0, see LICENSE for details.
// SPDX-License-Identifier: Apache-2.0
#include <assert.h>

#include "app.h"
#include "sw/device/lib/arch/device.h"
#include "sw/device/lib/base/mmio.h"
#include "sw/device/lib/dif/dif_gpio.h"
#include "sw/device/lib/dif/dif_i2c.h"
#include "sw/device/lib/dif/dif_pinmux.h"
#include "sw/device/lib/dif/dif_spi_host.h"
#include "sw/device/lib/runtime/log.h"
#include "sw/device/lib/testing/i2c_testutils.h"
#include "sw/device/lib/testing/pinmux_testutils.h"
#include "sw/device/lib/testing/test_framework/check.h"
#include "sw/device/lib/testing/test_framework/ottf_main.h"

#include "hw/top_earlgrey/sw/autogen/top_earlgrey.h"

OTTF_DEFINE_TEST_CONFIG();

typedef struct Platform {
  pinmux_testutils_mio_dict_t csb;
  pinmux_testutils_mio_dict_t sd0;
  pinmux_testutils_mio_dict_t clk;
  pinmux_testutils_mio_dict_t i2c_sda;
  pinmux_testutils_mio_dict_t i2c_clk;
  pinmux_testutils_mio_dict_t reset;
  pinmux_testutils_mio_dict_t dc;
  pinmux_testutils_mio_dict_t dsp_led;
  pinmux_testutils_mio_dict_t btn_up;
  pinmux_testutils_mio_dict_t btn_down;
  pinmux_testutils_mio_dict_t btn_left;
  pinmux_testutils_mio_dict_t btn_right;
  pinmux_testutils_mio_dict_t btn_ok;
  // Status LEDs
  pinmux_testutils_mio_dict_t allowed_led;
  pinmux_testutils_mio_dict_t blocked_led;
  pinmux_testutils_mio_dict_t status_led;
  pinmux_testutils_mio_dict_t boot_ok_led;
  pinmux_testutils_mio_dict_t app_ok_led;
  pinmux_testutils_mio_dict_t verify_fail_led;
  pinmux_testutils_mio_dict_t secure_violation_led;
  size_t spi_speed;
  LCD_Orientation orientation;
} Platform_t;

static const Platform_t kCw340Board = {
    .csb = PINMUX_TESTUTILS_NEW_MIO_DICT(Iob6),
    .sd0 = PINMUX_TESTUTILS_NEW_MIO_DICT(Iob0),
    .clk = PINMUX_TESTUTILS_NEW_MIO_DICT(Iob1),
    .reset = PINMUX_TESTUTILS_NEW_MIO_DICT(Iob4),
    .dc = PINMUX_TESTUTILS_NEW_MIO_DICT(Iob2),
    .dsp_led = PINMUX_TESTUTILS_NEW_MIO_DICT(Iob3),
    .spi_speed = 3000000,  // 3Mhz
    .orientation = LCD_Rotate0,
};

static const Platform_t kVoyager1Board = {
    .csb = PINMUX_TESTUTILS_NEW_MIO_DICT(Ioc6),
    .sd0 = PINMUX_TESTUTILS_NEW_MIO_DICT(Iob1),
    .clk = PINMUX_TESTUTILS_NEW_MIO_DICT(Iob2),
    .i2c_sda = PINMUX_TESTUTILS_NEW_MIO_DICT(Iob3),
    .i2c_clk = PINMUX_TESTUTILS_NEW_MIO_DICT(Iob4),
    .reset = PINMUX_TESTUTILS_NEW_MIO_DICT(Ioc7),
    .dc = PINMUX_TESTUTILS_NEW_MIO_DICT(Ioc9),
    .dsp_led = PINMUX_TESTUTILS_NEW_MIO_DICT(Iob0),
    .btn_up = PINMUX_TESTUTILS_NEW_MIO_DICT(Ioc12),
    .btn_down = PINMUX_TESTUTILS_NEW_MIO_DICT(Ioc10),
    .btn_right = PINMUX_TESTUTILS_NEW_MIO_DICT(Ioc11),
    .btn_left = PINMUX_TESTUTILS_NEW_MIO_DICT(Ior10),
    .btn_ok = PINMUX_TESTUTILS_NEW_MIO_DICT(Ior11),
    .blocked_led = PINMUX_TESTUTILS_NEW_MIO_DICT(Ioa2),
    .allowed_led = PINMUX_TESTUTILS_NEW_MIO_DICT(Ioa3),
    .status_led = PINMUX_TESTUTILS_NEW_MIO_DICT(Ioa4),
    .boot_ok_led = PINMUX_TESTUTILS_NEW_MIO_DICT(Ioa5),
    .app_ok_led = PINMUX_TESTUTILS_NEW_MIO_DICT(Ioa6),
    .verify_fail_led = PINMUX_TESTUTILS_NEW_MIO_DICT(Ioa7),
    .secure_violation_led = PINMUX_TESTUTILS_NEW_MIO_DICT(Ioa8),
    .spi_speed = 16000000,  // 16Mhz
    .orientation = LCD_Rotate180,
};

static status_t pinmux_select(const dif_pinmux_t *pinmux, Platform_t pinmap) {
  // SPI.
  TRY(dif_pinmux_output_select(pinmux, pinmap.csb.out,
                               kTopEarlgreyPinmuxOutselSpiHost1Csb));
  TRY(dif_pinmux_output_select(pinmux, pinmap.sd0.out,
                               kTopEarlgreyPinmuxOutselSpiHost1Sd0));
  TRY(dif_pinmux_output_select(pinmux, pinmap.clk.out,
                               kTopEarlgreyPinmuxOutselSpiHost1Sck));

  // I2C
  TRY(dif_pinmux_output_select(pinmux, pinmap.i2c_sda.out,
                               kTopEarlgreyPinmuxOutselI2c0Sda));
  TRY(dif_pinmux_output_select(pinmux, pinmap.i2c_clk.out,
                               kTopEarlgreyPinmuxOutselI2c0Scl));

  TRY(dif_pinmux_input_select(pinmux, kTopEarlgreyPinmuxPeripheralInI2c0Sda,
                              pinmap.i2c_sda.in));
  TRY(dif_pinmux_input_select(pinmux, kTopEarlgreyPinmuxPeripheralInI2c0Scl,
                              pinmap.i2c_clk.in));

  // RESET.
  TRY(dif_pinmux_output_select(pinmux, pinmap.reset.out,
                               kTopEarlgreyPinmuxOutselGpioGpio0));
  // A0/DC.
  TRY(dif_pinmux_output_select(pinmux, pinmap.dc.out,
                               kTopEarlgreyPinmuxOutselGpioGpio1));
  // BACKLIGHT LED.
  TRY(dif_pinmux_output_select(pinmux, pinmap.dsp_led.out,
                               kTopEarlgreyPinmuxOutselGpioGpio2));

  TRY(dif_pinmux_input_select(pinmux, kTopEarlgreyPinmuxPeripheralInGpioGpio4,
                              pinmap.btn_up.in));

  TRY(dif_pinmux_input_select(pinmux, kTopEarlgreyPinmuxPeripheralInGpioGpio5,
                              pinmap.btn_down.in));

  TRY(dif_pinmux_input_select(pinmux, kTopEarlgreyPinmuxPeripheralInGpioGpio6,
                              pinmap.btn_left.in));

  TRY(dif_pinmux_input_select(pinmux, kTopEarlgreyPinmuxPeripheralInGpioGpio7,
                              pinmap.btn_right.in));

  TRY(dif_pinmux_input_select(pinmux, kTopEarlgreyPinmuxPeripheralInGpioGpio8,
                              pinmap.btn_ok.in));
  // STATUS LEDs.
  TRY(dif_pinmux_output_select(pinmux, pinmap.allowed_led.out,
                               kTopEarlgreyPinmuxOutselGpioGpio10));
  TRY(dif_pinmux_output_select(pinmux, pinmap.blocked_led.out,
                               kTopEarlgreyPinmuxOutselGpioGpio11));
  TRY(dif_pinmux_output_select(pinmux, pinmap.status_led.out,
                               kTopEarlgreyPinmuxOutselGpioGpio12));
  TRY(dif_pinmux_output_select(pinmux, pinmap.boot_ok_led.out,
                               kTopEarlgreyPinmuxOutselGpioGpio13));
  TRY(dif_pinmux_output_select(pinmux, pinmap.app_ok_led.out,
                               kTopEarlgreyPinmuxOutselGpioGpio14));
  TRY(dif_pinmux_output_select(pinmux, pinmap.verify_fail_led.out,
                               kTopEarlgreyPinmuxOutselGpioGpio15));
  TRY(dif_pinmux_output_select(pinmux, pinmap.secure_violation_led.out,
                               kTopEarlgreyPinmuxOutselGpioGpio16));

  if (kDeviceType == kDeviceSilicon) {
    dif_pinmux_pad_attr_t out_attr;
    dif_pinmux_pad_attr_t in_attr = {
        .slew_rate = 1,
        .drive_strength = 3,
        .flags = kDifPinmuxPadAttrPullResistorEnable |
                 kDifPinmuxPadAttrPullResistorUp};

    TRY(dif_pinmux_pad_write_attrs(pinmux, pinmap.clk.pad, kDifPinmuxPadKindMio,
                                   in_attr, &out_attr));
    TRY(dif_pinmux_pad_write_attrs(pinmux, pinmap.sd0.pad, kDifPinmuxPadKindMio,
                                   in_attr, &out_attr));
    TRY(dif_pinmux_pad_write_attrs(pinmux, pinmap.csb.pad, kDifPinmuxPadKindMio,
                                   in_attr, &out_attr));
  }

  return OK_STATUS();
}

bool test_main(void) {
  dif_i2c_t i2c;
  dif_spi_host_t spi_lcd;
  dif_spi_host_t spi_flash;
  const volatile Platform_t *config = NULL;
  switch (kDeviceType) {
    case kDeviceFpgaCw340:
      LOG_INFO("FPGA CW340 detected!");
      config = &kCw340Board;
      break;
    case kDeviceSilicon:
      LOG_INFO("Silicon detected!");
      config = &kVoyager1Board;
      break;
    default:
      CHECK(false, "Platform not supported");
  }

  dif_pinmux_t pinmux;
  mmio_region_t addr = mmio_region_from_addr(TOP_EARLGREY_PINMUX_AON_BASE_ADDR);
  CHECK_DIF_OK(dif_pinmux_init(addr, &pinmux));
  pinmux_select(&pinmux, *config);

  addr = mmio_region_from_addr(TOP_EARLGREY_I2C0_BASE_ADDR);
  CHECK_DIF_OK(dif_i2c_init(addr, &i2c));
  CHECK_STATUS_OK(i2c_testutils_set_speed(&i2c, kDifI2cSpeedStandard, /*sda_rise_nanos=*/400, /*sda_fall_nanos=*/110));
  CHECK_DIF_OK(dif_i2c_host_set_enabled(&i2c, kDifToggleEnabled));

  addr = mmio_region_from_addr(TOP_EARLGREY_SPI_HOST1_BASE_ADDR);
  CHECK_DIF_OK(dif_spi_host_init(addr, &spi_lcd));
  CHECK_DIF_OK(dif_spi_host_configure(
                   &spi_lcd,
                   (dif_spi_host_config_t){
                       .spi_clock = config->spi_speed,
                       .peripheral_clock_freq_hz = (uint32_t)kClockFreqUsbHz,
                   }),
               "SPI_HOST 1 config failed!");
  CHECK_DIF_OK(dif_spi_host_output_set_enabled(&spi_lcd, true));

  addr = mmio_region_from_addr(TOP_EARLGREY_SPI_HOST0_BASE_ADDR);
  CHECK_DIF_OK(dif_spi_host_init(addr, &spi_flash));
  CHECK_DIF_OK(
      dif_spi_host_configure(&spi_flash,
                             (dif_spi_host_config_t){
                                 .spi_clock = config->spi_speed,
                                 .peripheral_clock_freq_hz =
                                     (uint32_t)kClockFreqHiSpeedPeripheralHz,
                             }),
      "SPI_HOST0 config failed!");

  dif_spi_device_handle_t spid;
  addr = mmio_region_from_addr(TOP_EARLGREY_SPI_DEVICE_BASE_ADDR);
  CHECK_DIF_OK(dif_spi_device_init_handle(addr, &spid));

  addr = mmio_region_from_addr(TOP_EARLGREY_GPIO_BASE_ADDR);
  dif_gpio_t gpio;
  CHECK_DIF_OK(dif_gpio_init(addr, &gpio));
  CHECK_DIF_OK(dif_gpio_output_set_enabled_all(&gpio, 0x0fc0f));

  addr = mmio_region_from_addr(TOP_EARLGREY_AES_BASE_ADDR);
  dif_aes_t aes;
  CHECK_DIF_OK(dif_aes_init(addr, &aes));
  CHECK_DIF_OK(dif_aes_reset(&aes));

  CHECK_STATUS_OK(run_demo(
      &spi_lcd, &spi_flash, &spid, &i2c, &gpio, &aes,
      (display_pin_map_t){0, 1, 2, 3}, (btn_pin_map_t){4, 5, 6, 7, 8},
      (status_led_pin_map_t){10, 11, 12, 13, 14, 15, 16}, config->orientation));

  return true;
}

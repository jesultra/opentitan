// Copyright lowRISC contributors.
// Licensed under the Apache License, Version 2.0, see LICENSE for details.
// SPDX-License-Identifier: Apache-2.0

#include "app.h"

#include <core/lucida_console_10pt.h>
#include <st7735/lcd_st7735.h>

#include "btn.h"
#include "demos.h"
#include "i2c_eeprom.h"
#include "images/logo_opentitan_160_39.h"
#include "screen.h"
#include "sw/device/lib/dif/dif_spi_host.h"
#include "sw/device/lib/runtime/ibex.h"
#include "sw/device/lib/runtime/print.h"
#include "sw/device/lib/testing/test_framework/check.h"

size_t lcd_height;
size_t lcd_width;

// Local functions declaration.
static uint32_t spi_transfer(void *handle, uint8_t *data, size_t len);
static uint32_t gpio_write(void *handle, bool cs, bool dc);
static void timer_delay(uint32_t ms);
static status_t aes_demo(context_t *ctx);
static bool check_secret_menu(btn_t btn);
static status_t credits(context_t *ctx);

status_t run_demo(dif_spi_host_t *spi_lcd, dif_spi_host_t *spi_flash,
                  dif_spi_device_handle_t *spid, dif_i2c_t *i2c,
                  dif_gpio_t *gpio, dif_aes_t *aes, display_pin_map_t dsp_pins,
                  btn_pin_map_t btn_pins, status_led_pin_map_t led_pins,
                  LCD_Orientation orientation) {
  LOG_INFO("%s: Initializing pins", __func__);
  // Set the initial state of the LCD control pins.
  TRY(dif_gpio_write(gpio, dsp_pins.dc, 0x0));
  TRY(dif_gpio_write(gpio, dsp_pins.led, 0x0));


  // Reset LCD.
  LOG_INFO("%s: Reseting display", __func__);
  TRY(dif_gpio_write(gpio, dsp_pins.reset, 0x0));
  busy_spin_micros(150 * 1000);
  TRY(dif_gpio_write(gpio, dsp_pins.reset, 0x1));

  // Init LCD driver and set the SPI driver.
  St7735Context lcd;
  context_t ctx = {spi_lcd, spi_flash, spid,     i2c,      gpio,
                   aes,     dsp_pins,  btn_pins, led_pins, &lcd};
  LCD_Interface interface = {
      .handle = &ctx,              // SPI handle.
      .spi_transfer = spi_transfer,      // SPI write callback.
      .gpio_write = gpio_write,    // GPIO write callback.
      .timer_delay = timer_delay,  // Timer delay callback.
      .reset = NULL,
      .set_backlight_pwm = NULL
  };
  LOG_INFO("%s: Initializing display", __func__);
  lcd_st7735_init(&lcd, &interface);

  // Set the LCD orientation.
  lcd_st7735_set_orientation(&lcd, orientation);
 lcd_st7735_get_resolution(&lcd, &lcd_height, &lcd_width);

  // Setup text font bitmaps to be used and the colors.
  lcd_st7735_set_font(&lcd, &lucidaConsole_10ptFont);
  lcd_st7735_set_font_colors(&lcd, RGBColorWhite, RGBColorBlack);
  size_t font_h = lcd.parent.font->height;

  LOG_INFO("%s: Clearing...", __func__);
  // Clean display with a white rectangle.
  lcd_st7735_clean(&lcd);
  TRY(dif_gpio_write(gpio, dsp_pins.led, 0x1));

  LOG_INFO("%s: Ot logo...", __func__);
  screen_println(&lcd, "Opentitan", alined_center, 6, true);
  screen_println(&lcd, "Boot successful!", alined_center, 7, true);

  lcd_st7735_set_font_colors(&lcd, RGBColorWhite, RGBColorGrey);
  lcd_st7735_puts(&lcd, (LCD_Point){.x = 30, .y = lcd_height - font_h},
      "Demo v0.1");
  lcd_st7735_set_font_colors(&lcd, RGBColorWhite, RGBColorBlack);

  TRY(dif_gpio_write(gpio, led_pins.boot_ok, 0x1));
  TRY(dif_gpio_write(gpio, led_pins.app_ok, 0x1));
  // Draw the splash screen with a RGB 565 bitmap and text in the bottom.
  lcd_st7735_draw_rgb565(
      &lcd,
      (LCD_rectangle){.origin = {.x = 0, .y = 20}, .width = lcd_width, .height = 39},
      (uint8_t *)logo_opentitan_160_39);
  busy_spin_micros(2500 * 1000);

  LOG_INFO("%s: Starting menu.", __func__);
  // Show the main menu.
  const char *items[] = {
      "1. AES ECB/CDC", "2. SPI Passthrough", "3. CTAP (FIDO)",
      "4. TPM",         "5. Selftest",        "6. Credits",
  };
  Menu_t main_menu = {
      .title = "Demo mode",
      .color = RGBColorBlue,
      .selected_color = RGBColorRed,
      .background = RGBColorWhite,
      .items_count = ARRAYSIZE(items),
      .items = items,
  };
  lcd_st7735_clean(&lcd);
  enum {
    kDefaultSelection = 5,
    kIdleTimeoutMillis = 5000000,
    kTimer1Second = 1000000,
  };
  int selected = kDefaultSelection;

  ibex_timeout_t idle_timeout = ibex_timeout_init(kIdleTimeoutMillis);
  ibex_timeout_t counter_timer = ibex_timeout_init(1);
  size_t idle_count_down = 5;

  screen_show_menu(&lcd, &main_menu, kDefaultSelection);
  do {
    if (ibex_timeout_check(&counter_timer)) {
      char buffer[3] = {0, 0, 0};
      base_snprintf(buffer, sizeof(buffer), "%u", --idle_count_down);
      lcd_st7735_puts(ctx.lcd, (LCD_Point){.x = lcd_width - 8, .y = lcd_height - font_h},
                      buffer);
      counter_timer = ibex_timeout_init(kTimer1Second);
    }

    status_t ret = scan_buttons(&ctx, 1000);

    if (!status_ok(ret)) {
      if (ibex_timeout_check(&idle_timeout)) {
        TRY(credits(&ctx));
        screen_show_menu(&lcd, &main_menu, (size_t)selected);
        idle_timeout = ibex_timeout_init(kIdleTimeoutMillis);
        idle_count_down = 5;
      }
      continue;
    }
    idle_timeout = ibex_timeout_init(kIdleTimeoutMillis);
    counter_timer = ibex_timeout_init(kTimer1Second);
    idle_count_down = 5;

    if (check_secret_menu((btn_t)UNWRAP(ret))) {
      main_menu.items_count++;
      screen_show_menu(&lcd, &main_menu, (size_t)selected);
      busy_spin_micros(1000 * 1000);
      continue;
    }

    switch (UNWRAP(ret)) {
      case kBtnUp:
        selected--;
        break;
      case kBtnDown:
        selected++;
        break;
      case kBtnRight:
      case kBtnOk:
        switch (selected) {
          case 0:
            TRY(aes_demo(&ctx));
            break;
          case 1:
            TRY(spi_passthrough_demo(&ctx));
            break;
          case 4:
            TRY(self_test(&ctx));
            break;
          case 5:
            TRY(credits(&ctx));
            break;
          case 6:
            TRY(i2c_eeprom(&ctx));
            break;
          default:
            screen_println(&lcd, "Option not avail!", alined_center, 8, true);
            break;
        }
        break;
      case kBtnLeft:
      default:
        break;
    }
    const int kMaxSelection = ARRAYSIZE(items) - 1;
    if (selected >= kMaxSelection) {
      selected = kMaxSelection;
    } else if (selected < 0) {
      selected = kDefaultSelection;
    }
    screen_show_menu(&lcd, &main_menu, (size_t)selected);
  } while (1);
}

static status_t aes_demo(context_t *ctx) {
  while (1) {
    TRY(run_aes(ctx));
    busy_spin_micros(5000 * 1000);
  }

  return OK_STATUS();
}

static status_t credits(context_t *ctx) {
  enum { kTimeoutMillis = 15000000 };
  ibex_timeout_t timeout = ibex_timeout_init(kTimeoutMillis);

  lcd_st7735_clean(ctx->lcd);

  EepromReader reader;
  Rendable rendable;
  if (status_ok(eeprom_init_rendable(&reader, ctx))) {
    while (status_ok(eeprom_reader_next_rendable(&reader, &rendable))) {
      switch (rendable.type) {
        case kTypeOtImage:
          lcd_st7735_draw_rgb565(
              ctx->lcd,
              (LCD_rectangle){.origin = {.x = rendable.u.ot_image.posx,
                                         .y = rendable.u.ot_image.posy},
                              .width = lcd_width,
                              .height = 39},
              (uint8_t *)logo_opentitan_160_39);
          break;
        case kTypeText:
          break;
        case kTypePositionedText:
          break;
        case kTypeFormattedText:
          lcd_st7735_set_font_colors(
              ctx->lcd, bgr565_to_rgb24(rendable.u.collored_text.bg_collor),
              bgr565_to_rgb24(rendable.u.collored_text.fg_collor));
          lcd_st7735_puts(ctx->lcd,
                          (LCD_Point){.x = rendable.u.collored_text.posx,
                                      .y = rendable.u.collored_text.posy},
                          (char *)rendable.u.collored_text.string);
          break;
      }
    }
  } else {
    lcd_st7735_draw_rgb565(
        ctx->lcd,
        (LCD_rectangle){.origin = {.x = 0, .y = 0}, .width = lcd_width, .height = 39},
        (uint8_t *)logo_opentitan_160_39);

    size_t row = 3;
    screen_println(ctx->lcd, "the first opensource", alined_center, row++,
                   false);
    screen_println(ctx->lcd, "Root of Trust ", alined_center, row++, false);
    screen_println(ctx->lcd, "Visit:", alined_center, row++, false);
    screen_println(ctx->lcd, "www.opentitan.org", alined_center, row++, false);
  }

  lcd_st7735_set_font_colors(ctx->lcd, RGBColorWhite, RGBColorGrey);
  lcd_st7735_puts(ctx->lcd, (LCD_Point){.x = 0, .y = 126 - 10},
                  "(Move stick to exit)");

  while (!(status_ok(scan_buttons(ctx, 1000)) || ibex_timeout_check(&timeout)))
    ;

  lcd_st7735_clean(ctx->lcd);
  lcd_st7735_set_font_colors(ctx->lcd, RGBColorWhite, RGBColorBlue);
  return OK_STATUS();
}

static bool check_secret_menu(btn_t btn) {
  static const btn_t kSecret[] = {
      kBtnDown, kBtnDown, kBtnDown, kBtnDown, kBtnLeft,  kBtnUp,
      kBtnUp,   kBtnUp,   kBtnUp,   kBtnLeft, kBtnRight,
  };
  static size_t index = 0;
  if (kSecret[index] == btn) {
    if (index == ARRAYSIZE(kSecret) - 1) {
      LOG_INFO("Unloked");
      return true;
    }
    index = (index + 1) % ARRAYSIZE(kSecret);
    LOG_INFO("secret: %u", index);
  } else {
    index = 0;
  }
  return false;
}

static uint32_t spi_transfer(void *handle, uint8_t *data, size_t len) {
  context_t *ctx = (context_t *)handle;
  const uint32_t data_sent = len;

  dif_spi_host_segment_t transaction = {.type = kDifSpiHostSegmentTypeTx,
                                        .tx = {
                                            .width = kDifSpiHostWidthStandard,
                                            .buf = data,
                                            .length = len,
                                        }};
  CHECK_DIF_OK(
      dif_spi_host_transaction(ctx->spi_lcd, /*csid=*/0, &transaction, 1));
  ibex_timeout_t deadline = ibex_timeout_init(5000);
  dif_spi_host_status_t status;
  do {
    CHECK_DIF_OK(dif_spi_host_get_status(ctx->spi_lcd, &status));
    if (ibex_timeout_check(&deadline)) {
      LOG_INFO("%s, Timeout", __func__);
      return 0;
    }
  } while (!status.tx_empty);
  return data_sent;
}

static uint32_t gpio_write(void *handle, bool cs, bool dc) {
  context_t *ctx = (context_t *)handle;
  // CHECK_DIF_OK(dif_gpio_write(ctx->gpio, ctx->dsp_pins.cs, cs));
  CHECK_DIF_OK(dif_gpio_write(ctx->gpio, ctx->dsp_pins.dc, dc));
  return 0;
}

static void timer_delay(uint32_t ms) { busy_spin_micros(ms * 100); }

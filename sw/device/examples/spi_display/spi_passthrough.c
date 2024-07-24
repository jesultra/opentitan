// Copyright lowRISC contributors.
// Licensed under the Apache License, Version 2.0, see LICENSE for details.
// SPDX-License-Identifier: Apache-2.0

#include <stdint.h>

#include "btn.h"
#include "context.h"
#include "demos.h"
#include "screen.h"
#include "sw/device/lib/runtime/print.h"
#include "sw/device/lib/testing/aes_testutils.h"
#include "sw/device/lib/testing/profile.h"
#include "sw/device/lib/testing/spi_device_testutils.h"
#include "sw/device/lib/testing/spi_flash_testutils.h"
#include "sw/device/lib/testing/test_framework/check.h"

// static const uint8_t kHash[] = {0x8c, 0x12, 0x76, 0xd2, 0x40, 0x9a, 0x29,
// 0x73,
//                                 0xf8, 0xe,  0x16, 0x6a, 0x51, 0x70, 0x19,
//                                 0x9d, 0x24, 0x11, 0xce, 0x8b, 0xde, 0xc,
//                                 0xc7, 0xa8, 0x17, 0xbb, 0x29, 0xd1, 0x64,
//                                 0xa4, 0x16, 0x6a};

static const uint8_t kHash[] = {0x60, 0x5c, 0xc1, 0x11, 0x8d, 0x01, 0xe4, 0xad,
                                0x15, 0x36, 0xef, 0x15, 0xe5, 0x2e, 0x2b, 0xf2,
                                0x3c, 0x74, 0xda, 0x82, 0xf2, 0x14, 0x4b, 0x89,
                                0xc6, 0x63, 0xab, 0xa2, 0xd3, 0xf5, 0x0f, 0x49};

static const uint32_t kHashAddr = 1 * 1024 * 1024;

static status_t check_hash(context_t *ctx);
static status_t configure_spi_flash_mode(dif_spi_device_handle_t *spid);
static status_t enable_secure_boot(context_t *ctx);

status_t spi_passthrough_demo(context_t *ctx) {
  uint32_t bg, fg;
  bg = ctx->lcd->rgb_background;
  fg = ctx->lcd->rgb_foreground;
  lcd_st7735_clean(ctx->lcd);
  size_t selected = 0;
  const char *items[] = {
      "Enable SecureBoot",
      "Disable SecureBoot",
      "Exit",
  };
  Menu_t main_menu = {
      .title = "Passthrough mode",
      .color = BGRColorBlue,
      .selected_color = BGRColorRed,
      .background = BGRColorWhite,
      .items_count = sizeof(items) / sizeof(items[0]),
      .items = items,
  };
  lcd_st7735_clean(ctx->lcd);
  bool authenticated = false;
  screen_show_menu(ctx->lcd, &menu, selected);
  enable_secure_boot(ctx);
  busy_spin_micros(500 * 1000);
  bool loop = true;

  do {
    status_t ret = scan_buttons(ctx, 100);
    if (!status_ok(ret)) {
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
          case 0:  // Secure boot
            authenticated = true ? status_ok(enable_secure_boot(ctx)) : false;
            break;
          case 1:  // Passthough
            TRY(dif_spi_device_set_passthrough_mode(ctx->spid,
                                                    kDifToggleEnabled));
            TRY(spi_device_testutils_configure_passthrough(
                ctx->spid,
                /*filters=*/0x00,
                /*upload_write_commands=*/false));
            lcd_st7735_set_font_colors(ctx->lcd, BGRColorRed, BGRColorWhite);
            screen_println(ctx->lcd, "Secure Boot", alined_center, 5, true);
            screen_println(ctx->lcd, "disabled!", alined_center, 6, true);
            break;
          case 2:
            loop = false;
            break;
          default:
            break;
        }
        break;
      case kBtnLeft:
      default:
        break;
    }

    bool flag;
    TRY(dif_spi_device_get_csb_status(ctx->spid, &flag));
    if (!authenticated && !flag) {
      screen_println(ctx->lcd, "Not authenticated!", alined_center, 5, true);
      busy_spin_micros(2000 * 1000);
    }
    lcd_st7735_set_font_colors(ctx->lcd, bg, fg);
    const int kMaxSelection = ARRAYSIZE(items) - 1;
    if (selected >= kMaxSelection) {
      selected = kMaxSelection;
    } else if (selected < 0) {
      selected = 0;
    }
    screen_show_menu(ctx->lcd, &menu, (size_t)selected);
  } while (loop);

  authenticated = false;
  screen_println(ctx->lcd, "Disabling boot!", alined_center, 5, true);
  screen_println(ctx->lcd, "                         ", alined_center, 6, true);
  TRY(dif_spi_device_set_passthrough_mode(ctx->spid, kDifToggleDisabled));
  TRY(dif_spi_host_output_set_enabled(ctx->spi_flash, false));
  busy_spin_micros(2000 * 1000);
  lcd_st7735_clean(ctx->lcd);
  return OK_STATUS();
}

static status_t check_hash(context_t *ctx) {
  TRY(dif_spi_host_output_set_enabled(ctx->spi_flash, true));

  dif_spi_host_segment_t op = {
      .type = kDifSpiHostSegmentTypeOpcode,
      .opcode = {.opcode = kSpiDeviceFlashOpResetEnable,
                 .width = kDifSpiHostWidthStandard}};
  TRY(dif_spi_host_transaction(ctx->spi_flash, /*cs_id=*/0, &op, 1));

  uint8_t buf[32];
  TRY(spi_flash_testutils_read_op(ctx->spi_flash, kSpiDeviceFlashOpReadNormal,
                                  buf, sizeof(buf), kHashAddr,
                                  /*addr_is_4b=*/false,
                                  /*width=*/1,
                                  /*dummy=*/0));
  if (memcmp(buf, kHash, ARRAYSIZE(kHash))) {
    screen_println(ctx->lcd, "Tamper detected!", alined_center, 5, true);
    screen_println(ctx->lcd, "                         ", alined_center, 6,
                   true);
    busy_spin_micros(3000 * 1000);
    return UNAUTHENTICATED();
  }
  return OK_STATUS();
}

static status_t configure_spi_flash_mode(dif_spi_device_handle_t *spid) {
  dif_spi_device_config_t spi_device_config = {
      .tx_order = kDifSpiDeviceBitOrderMsbToLsb,
      .rx_order = kDifSpiDeviceBitOrderMsbToLsb,
      .device_mode = kDifSpiDeviceModeFlashEmulation,
  };
  TRY(dif_spi_device_configure(spid, spi_device_config));

  // Configure the READ_JEDEC_ID command (CMD_INFO_3).
  dif_spi_device_flash_command_t cmd = {
      .opcode = kSpiDeviceFlashOpReadJedec,
      .address_type = kDifSpiDeviceFlashAddrDisabled,
      .dummy_cycles = 0,
      .payload_io_type = kDifSpiDevicePayloadIoSingle,
      .payload_dir_to_host = false,
      .upload = true,
  };

  TRY(dif_spi_device_set_flash_command_slot(
      spid, kSpiDeviceReadCommandSlotBase + 3, kDifToggleEnabled, cmd));

  // LOG_INFO("SYNC: Flash mode");

  // busy_spin_micros(1 * 1000 * 1000);

  dif_spi_device_irq_state_snapshot_t spi_device_irqs =
      (dif_spi_device_irq_state_snapshot_t)0xffffffff;

  TRY(dif_spi_device_irq_restore_all(&spid->dev, &spi_device_irqs));
  return OK_STATUS();
}

static status_t enable_secure_boot(context_t *ctx) {
  TRY(dif_spi_device_set_passthrough_mode(ctx->spid, kDifToggleDisabled));
  TRY(dif_spi_host_output_set_enabled(ctx->spi_flash, false));
  lcd_st7735_set_font_colors(ctx->lcd, BGRColorRed, BGRColorWhite);
  if (status_err(check_hash(ctx))) {
    TRY(configure_spi_flash_mode(ctx->spid));
    return PERMISSION_DENIED();
  }
  TRY(dif_spi_device_set_passthrough_mode(ctx->spid, kDifToggleEnabled));
  TRY(spi_device_testutils_configure_passthrough(
      ctx->spid,
      /*filters=*/0x00,
      /*upload_write_commands=*/false));
  lcd_st7735_set_font_colors(ctx->lcd, BGRColorGreen, BGRColorWhite);
  screen_println(ctx->lcd, "SecureBoot", alined_center, 5, true);
  screen_println(ctx->lcd, "enabled!", alined_center, 6, true);
  return OK_STATUS();
}

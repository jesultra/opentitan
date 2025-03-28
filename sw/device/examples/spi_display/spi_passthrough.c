// Copyright lowRISC contributors.
// Licensed under the Apache License, Version 2.0, see LICENSE for details.
// SPDX-License-Identifier: Apache-2.0

#include <stdint.h>

#include "btn.h"
#include "context.h"
#include "demos.h"
#include "screen.h"
// #include "sw/device/lib/runtime/print.h"
// #include "sw/device/lib/testing/profile.h"
#include "sw/device/lib/crypto/include/sha2.h"
#include "sw/device/lib/runtime/print.h"
#include "sw/device/lib/testing/aes_testutils.h"
#include "sw/device/lib/testing/spi_device_testutils.h"
#include "sw/device/lib/testing/spi_flash_testutils.h"
#include "sw/device/lib/testing/test_framework/check.h"

#define MANIFEST_HEADER 0xbebacafe
#define MANIFEST_TAIL 0xdeadbeef
typedef struct Manifest {
  uint32_t header;
  uint32_t bin_addr;
  uint32_t bin_size;
  uint32_t boot_addr;
  uint32_t signature[4];
  uint32_t tail;
} Manifest_t;

enum { kManifestAddr = 0x100000 };

static status_t authenticate(context_t *ctx);
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
  Menu_t menu = {
      .title = "Passthrough mode",
      .color = RGBColorBlue,
      .selected_color = RGBColorRed,
      .background = RGBColorWhite,
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
            lcd_st7735_set_font_colors(ctx->lcd, RGBColorAmber, RGBColorWhite);
            screen_println(ctx->lcd, "Secure Boot", alined_center, 5, true);
            screen_println(ctx->lcd, "disabled!", alined_center, 6, true);
            TRY(dif_gpio_write(ctx->gpio, ctx->led_pins.verify_fail, 0x0));
            TRY(dif_gpio_write(ctx->gpio, ctx->led_pins.security_violation,
                               0x0));
            TRY(dif_gpio_write(ctx->gpio, ctx->led_pins.blocked, 0x0));
            TRY(dif_gpio_write(ctx->gpio, ctx->led_pins.allowed, 0x0));
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

static status_t check_signature(context_t *ctx, uint8_t *sig, uint8_t *hash,
                                size_t len) {
  static dif_aes_transaction_t transaction = {
      .operation = kDifAesOperationEncrypt,
      .mode = kDifAesModeEcb,
      .key_len = kDifAesKey128,
      .key_provider = kDifAesKeySoftwareProvided,
      .mask_reseeding = kDifAesReseedPer64Block,
      .manual_operation = kDifAesManualOperationAuto,
      .reseed_on_key_change = false,
      .ctrl_aux_lock = false,
  };
  static const uint8_t kKeyShare1[] = {
      0x0f, 0x1f, 0x2f, 0x3F, 0x4f, 0x5f, 0x6f, 0x7f,
      0x8f, 0x9f, 0xaf, 0xbf, 0xcf, 0xdf, 0xef, 0xff,
  };
  static const uint8_t kAesModesKey128[] = {0x60, 0x3d, 0xeb, 0x10, 0x15, 0xca,
                                            0x71, 0xbe, 0x2b, 0x73, 0xae, 0xf0,
                                            0x85, 0x7d, 0x77, 0x81};

  uint8_t key_share0[sizeof(kAesModesKey128)];
  for (int i = 0; i < sizeof(kAesModesKey128); ++i) {
    key_share0[i] = kAesModesKey128[i] ^ kKeyShare1[i];
  }

  dif_aes_key_share_t key;
  memcpy(key.share0, key_share0, sizeof(key.share0));
  memcpy(key.share1, kKeyShare1, sizeof(key.share1));
  TRY(dif_aes_start(ctx->aes, &transaction, &key, NULL));
  dif_aes_data_t in_data;
  memcpy(in_data.data, hash, sizeof(in_data.data));
  TRY(dif_aes_load_data(ctx->aes, in_data));
  AES_TESTUTILS_WAIT_FOR_STATUS(ctx->aes, kDifAesStatusOutputValid, true, 5000);
  dif_aes_data_t out_data;
  TRY(dif_aes_read_output(ctx->aes, &out_data));
  TRY(dif_aes_end(ctx->aes));

  LOG_INFO("Signature");
  base_hexdump((const char *)&out_data, sizeof(out_data));
  if (memcmp(sig, (uint8_t *)out_data.data, len) == 0) {
    return OK_STATUS();
  }
  return UNAUTHENTICATED();
}

static status_t authenticate(context_t *ctx) {
  TRY(dif_spi_host_output_set_enabled(ctx->spi_flash, true));
  // Send a software reset command to the flash.
  dif_spi_host_segment_t op = {
      .type = kDifSpiHostSegmentTypeOpcode,
      .opcode = {.opcode = kSpiDeviceFlashOpResetEnable,
                 .width = kDifSpiHostWidthStandard}};
  TRY(dif_spi_host_transaction(ctx->spi_flash, /*cs_id=*/0, &op, 1));

  // Read manifest
  Manifest_t manifest = {0};
  TRY(spi_flash_testutils_read_op(ctx->spi_flash, kSpiDeviceFlashOpReadNormal,
                                  (uint8_t *)&manifest, sizeof(manifest),
                                  kManifestAddr,
                                  /*addr_is_4b=*/false,
                                  /*width=*/1,
                                  /*dummy=*/0));

  LOG_INFO("Manifest %d bytes", sizeof(manifest));
  base_hexdump((const char *)&manifest, sizeof(manifest));
  if (manifest.header != MANIFEST_HEADER || manifest.tail != MANIFEST_TAIL) {
    return UNAUTHENTICATED();
  }
  uint8_t buf[256];

  otcrypto_sha2_context_t crypto;
  TRY(otcrypto_sha2_init(kOtcryptoHashModeSha256, &crypto));
  for (uint32_t addr = manifest.bin_addr, size = manifest.bin_size; size > 0;
       addr += sizeof(buf)) {
    uint32_t block_len = size > sizeof(buf) ? sizeof(buf) : size;
    size -= block_len;
    TRY(spi_flash_testutils_read_op(ctx->spi_flash, kSpiDeviceFlashOpReadNormal,
                                    buf, block_len, addr,
                                    /*addr_is_4b=*/false,
                                    /*width=*/1,
                                    /*dummy=*/0));
    otcrypto_const_byte_buf_t msg_buf = {
        .data = buf,
        .len = block_len,
    };
    TRY(otcrypto_sha2_update(&crypto, &msg_buf));
  }
  uint32_t hash[32 / sizeof(uint32_t)];
  otcrypto_hash_digest_t digest_buf = {
      .data = hash,
      .len = ARRAYSIZE(hash),
  };
  CHECK_STATUS_OK(otcrypto_sha2_final(&crypto, &digest_buf));

  LOG_INFO("\r\n\nSha256 from addr %x to %x:", manifest.bin_addr,
           manifest.bin_addr + manifest.bin_size);
  base_hexdump((const char *)&hash, sizeof(hash));
  status_t status =
      check_signature(ctx, (uint8_t *)manifest.signature, (uint8_t *)hash,
                      sizeof(manifest.signature));
  return status;
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

  dif_spi_device_irq_state_snapshot_t spi_device_irqs =
      (dif_spi_device_irq_state_snapshot_t)0xffffffff;

  TRY(dif_spi_device_irq_restore_all(&spid->dev, &spi_device_irqs));
  return OK_STATUS();
}

static status_t enable_secure_boot(context_t *ctx) {
  TRY(dif_spi_device_set_passthrough_mode(ctx->spid, kDifToggleDisabled));
  TRY(dif_spi_host_output_set_enabled(ctx->spi_flash, false));
  lcd_st7735_set_font_colors(ctx->lcd, RGBColorRed, RGBColorWhite);
  if (status_err(authenticate(ctx))) {
    screen_println(ctx->lcd, "Tamper detected!", alined_center, 5, true);
    screen_println(ctx->lcd, "                         ", alined_center, 6,
                   true);
    TRY(dif_gpio_write(ctx->gpio, ctx->led_pins.verify_fail, 0x1));
    TRY(dif_gpio_write(ctx->gpio, ctx->led_pins.security_violation, 0x1));
    TRY(dif_gpio_write(ctx->gpio, ctx->led_pins.blocked, 0x1));
    TRY(dif_gpio_write(ctx->gpio, ctx->led_pins.allowed, 0x0));
    busy_spin_micros(3000 * 1000);
    TRY(configure_spi_flash_mode(ctx->spid));
    return PERMISSION_DENIED();
  }
  TRY(dif_spi_device_set_passthrough_mode(ctx->spid, kDifToggleEnabled));
  TRY(spi_device_testutils_configure_passthrough(
      ctx->spid,
      /*filters=*/0x00,
      /*upload_write_commands=*/false));
  lcd_st7735_set_font_colors(ctx->lcd, RGBColorGreen, RGBColorWhite);
  screen_println(ctx->lcd, "SecureBoot", alined_center, 5, true);
  screen_println(ctx->lcd, "enabled!", alined_center, 6, true);
  TRY(dif_gpio_write(ctx->gpio, ctx->led_pins.verify_fail, 0x0));
  TRY(dif_gpio_write(ctx->gpio, ctx->led_pins.security_violation, 0x0));
  TRY(dif_gpio_write(ctx->gpio, ctx->led_pins.blocked, 0x0));
  TRY(dif_gpio_write(ctx->gpio, ctx->led_pins.allowed, 0x1));
  return OK_STATUS();
}

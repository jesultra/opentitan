// Copyright lowRISC contributors.
// Licensed under the Apache License, Version 2.0, see LICENSE for details.
// SPDX-License-Identifier: Apache-2.0

#include "i2c_eeprom.h"

#include <stdint.h>

#include "btn.h"
#include "context.h"
#include "demos.h"
#include "screen.h"
//#include "sw/device/lib/crypto/include/hash.h"
#include "sw/device/lib/runtime/print.h"
#include "sw/device/lib/testing/i2c_testutils.h"
#include "sw/device/lib/testing/test_framework/check.h"

enum {
  kDeviceAddr = 0xa0 >> 1,
  // Default timeout for i2c reads.
  kTimeoutMicros = 5000,
  // "Acknowledgement polling" timeout - used for waiting until EEPROM has
  // finished writing.
  kAckPollTimeoutMicros = 80000,
};

static status_t read(context_t *ctx);
static status_t write(context_t *ctx);
static status_t eeprom_write_bytes(context_t *ctx, uint8_t addr, void *bytes,
                                   size_t len);

status_t eeprom_init_rendable(EepromReader *reader, context_t *ctx) {
  reader->i2c = ctx->i2c;
  EepromHeader header;
  const uint8_t kAddr[1] = {0x00};
  TRY(i2c_testutils_write(reader->i2c, kDeviceAddr, sizeof(kAddr), kAddr,
                          true));
  TRY(i2c_testutils_read(reader->i2c, kDeviceAddr, sizeof(header),
                         (uint8_t *)&header, kTimeoutMicros));

  LOG_INFO("header: %x%x, version: %u, count: %u", header.header[0],
           header.header[1], header.version, header.item_count);

  TRY_CHECK(header.header[0] == 0xa9 && header.header[1] == 0x55);
  reader->items_count = header.item_count;
  reader->addr = sizeof(EepromHeader);
  return OK_STATUS();
}

status_t eeprom_reader_next_rendable(EepromReader *reader, Rendable *rendable) {
  if (reader->items_count-- > 0) {
    uint8_t addr = reader->addr;
    TRY(i2c_testutils_write(reader->i2c, kDeviceAddr, sizeof(addr), &addr,
                            true));
    TRY(i2c_testutils_read(reader->i2c, kDeviceAddr, sizeof(Rendable),
                           (uint8_t *)rendable, kTimeoutMicros));

    switch (rendable->type) {
      case kTypeOtImage:
        LOG_INFO("type: %x, image, posx:%u, posy:%u", rendable->type,
                 rendable->u.ot_image.posx, rendable->u.ot_image.posy);
        addr += sizeof(rendable->type) + sizeof(OtImage);
        break;
      case kTypeText:
        rendable->u.text.string[rendable->u.text.len] = 0;
        LOG_INFO("type: %x, len: %u, string: %s", rendable->type,
                 rendable->u.text.len, rendable->u.text.string);
        break;
      case kTypePositionedText:
        rendable->u.positioned_text.string[rendable->u.positioned_text.len] = 0;
        LOG_INFO("type: %x, posx:%u, posy:%u, len: %u, string: %s",
                 rendable->type, rendable->u.positioned_text.posx,
                 rendable->u.positioned_text.posy,
                 rendable->u.positioned_text.len,
                 rendable->u.positioned_text.string);
        break;
      case kTypeFormattedText:
        rendable->u.collored_text.string[rendable->u.collored_text.len] = 0;
        LOG_INFO(
            "type: %x, posx:%u, posy:%u, bg_collor:%x, fg_collor:%x, len: %u, "
            "string: %s",
            rendable->type, rendable->u.collored_text.posx,
            rendable->u.collored_text.posy, rendable->u.collored_text.bg_collor,
            rendable->u.collored_text.fg_collor, rendable->u.collored_text.len,
            rendable->u.collored_text.string);
        addr += sizeof(rendable->type) + offsetof(ColloredText, string) +
                rendable->u.collored_text.len;
        break;
    }
    reader->addr = addr;
    return OK_STATUS();
  }
  return RESOURCE_EXHAUSTED();
}

status_t i2c_eeprom(context_t *ctx) {
  lcd_st7735_clean(ctx->lcd);
  size_t selected = 0;
  const char *items[] = {
      "Read",
      "Write",
      "Exit",
  };
  Menu_t menu = {
      .title = "I2C EEPROM",
      .color = RGBColorBlue,
      .selected_color = RGBColorRed,
      .background = RGBColorWhite,
      .items_count = ARRAYSIZE(items),
      .items = items,
  };
  lcd_st7735_clean(ctx->lcd);
  screen_show_menu(ctx->lcd, &menu, selected);
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
          case 0:
            TRY(read(ctx));
            break;
          case 1:
            TRY(write(ctx));
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

    const int kMaxSelection = ARRAYSIZE(items) - 1;
    if (selected >= kMaxSelection) {
      selected = kMaxSelection;
    } else if (selected < 0) {
      selected = 0;
    }
    screen_show_menu(ctx->lcd, &menu, (size_t)selected);
  } while (loop);

  busy_spin_micros(2000 * 1000);
  lcd_st7735_clean(ctx->lcd);
  return OK_STATUS();
}

static status_t poll_while_busy(dif_i2c_t *i2c) {
  uint8_t dummy;
  return i2c_testutils_read(i2c, kDeviceAddr, 1, &dummy, kAckPollTimeoutMicros);
}

static status_t read(context_t *ctx) {
  EepromReader reader;
  Rendable rendable;
  TRY(eeprom_init_rendable(&reader, ctx));
  while (status_ok(eeprom_reader_next_rendable(&reader, &rendable))) {
    LOG_INFO("Next");
  }
  return OK_STATUS();
}

static status_t write(context_t *ctx) {
  uint8_t addr = 0;
  {
    EepromHeader header = {
        .header = {0xa9, 0x55},
        .version = 1,
        .item_count = 5,
    };
    TRY(eeprom_write_bytes(ctx, addr, &header, sizeof(header)));
    addr = sizeof(header);
  }
  {
    Rendable rendable = {.type = kTypeOtImage,
                         .u.ot_image = {.posx = 0, .posy = 0}};

    size_t size = sizeof(rendable.type) + sizeof(OtImage);
    TRY(eeprom_write_bytes(ctx, addr, &rendable, size));
    addr += size;
  }
  {
    Rendable rendable = {
        .type = kTypeFormattedText,
        .u.collored_text = {
            .posx = 0,
            .posy = 50,
            .bg_collor = rgb24_to_bgr565(0xffffff),
            .fg_collor = rgb24_to_bgr565(0x0000ff),
            .len = 9,
            .string = {'T', 'h', 'a', 'n', 'k', ' ', 'y', 'o', 'u'},
        }};
    size_t size = sizeof(rendable.type) + offsetof(ColloredText, string) +
                  rendable.u.collored_text.len;
    TRY(eeprom_write_bytes(ctx, addr, &rendable, size));
    addr += size;
  }

  {
    Rendable rendable = {.type = kTypeColloredText,
                         .u.collored_text = {
                             .posx = 0,
                             .posy = 62,
                             .bg_collor = rgb24_to_bgr565(0xffffff),
                             .fg_collor = rgb24_to_bgr565(0xffbfff),
                             .len = 12,
                             .string = {'G', 'a', 'v', 'i', 'n', ' ', 'F', 'e',
                                        'r', 'r', 'i', 's'},
                         }};
    size_t size = sizeof(rendable.type) + offsetof(ColloredText, string) +
                  rendable.u.collored_text.len;
    TRY(eeprom_write_bytes(ctx, addr, &rendable, size));
    addr += size;
  }

  {
    Rendable rendable = {
        .type = kTypeColloredText,
        .u.collored_text = {
            .posx = 0,
            .posy = 74,
            .bg_collor = rgb24_to_bgr565(0xffffff),
            .fg_collor = rgb24_to_bgr565(0x0000ff),
            .len = 19,
            .string = {'F', 'o', 'r', ' ', 't', 'h', 'e', ' ', 'e', 'f', 'f',
                       'o', 'r', 't', 's', ' ', 'o', 'n'},
        }};
    size_t size = sizeof(rendable.type) + offsetof(ColloredText, string) +
                  rendable.u.collored_text.len;
    TRY(eeprom_write_bytes(ctx, addr, &rendable, size));
    addr += size;
  }
  {
    Rendable rendable = {
        .type = kTypeColloredText,
        .u.collored_text = {
            .posx = 0,
            .posy = 86,
            .bg_collor = rgb24_to_bgr565(0xffffff),
            .fg_collor = rgb24_to_bgr565(0x0000ff),
            .len = 9,
            .string = {'O', 'p', 'e', 'n', 't', 'i', 't', 'a', 'n'},
        }};
    size_t size = sizeof(rendable.type) + offsetof(ColloredText, string) +
                  rendable.u.collored_text.len;
    TRY(eeprom_write_bytes(ctx, addr, &rendable, size));
    addr += size;
  }

  return OK_STATUS();
}

static status_t eeprom_write_bytes(context_t *ctx, uint8_t addr, void *buffer,
                                   size_t len) {
  enum { kPageSize = 8 };
  uint8_t data[kPageSize + sizeof(addr)];
  uint8_t aligned_addr = (addr + kPageSize - 1) & ~(kPageSize - 1);
  uint8_t *bytes = (uint8_t *)buffer;

  if (addr < aligned_addr) {
    data[0] = addr;
    size_t chunk = aligned_addr - addr;
    chunk = chunk < len ? chunk : len;
    memcpy(&data[sizeof(addr)], bytes, chunk);
    TRY(i2c_testutils_write(ctx->i2c, kDeviceAddr, chunk + sizeof(addr), data,
                            false));
    TRY(poll_while_busy(ctx->i2c));
    bytes += chunk;
    len -= chunk;
    addr = aligned_addr;
    LOG_INFO("Writting unaligned: %u bytes at 0x%x", chunk, data[0]);
  }

  while (len >= kPageSize) {
    data[0] = addr;
    memcpy(&data[sizeof(addr)], bytes, kPageSize);
    TRY(i2c_testutils_write(ctx->i2c, kDeviceAddr, kPageSize + sizeof(addr),
                            data, false));
    TRY(poll_while_busy(ctx->i2c));
    len -= kPageSize;
    bytes += kPageSize;
    addr += kPageSize;
    LOG_INFO("Writting aligned: %u bytes at 0x%x", kPageSize, data[0]);
  }

  if (len > 0) {
    data[0] = addr;
    memcpy(&data[sizeof(addr)], bytes, len);
    TRY(i2c_testutils_write(ctx->i2c, kDeviceAddr, len + sizeof(addr), data,
                            false));
    TRY(poll_while_busy(ctx->i2c));
    LOG_INFO("Writting reminder: %u bytes at 0x%x", len, data[0]);
  }

  return OK_STATUS();
}

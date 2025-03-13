// Copyright lowRISC contributors.
// Licensed under the Apache License, Version 2.0, see LICENSE for details.
// SPDX-License-Identifier: Apache-2.0

#include "context.h"
#include "sw/device/lib/testing/test_framework/check.h"

enum { kMaxStringSize = 19 * 10 };

typedef struct __attribute__((packed)) {
  uint8_t header[2];
  uint8_t version;
  uint8_t item_count;
} EepromHeader;

typedef enum __attribute__((packed)) {
  kTypeOtImage = 0,
  kTypeText = 1,
  kTypePositionedText = 2,
  kTypeFormattedText = 3,
} Type;

typedef struct __attribute__((packed)) {
  uint8_t posx;
  uint8_t posy;
} OtImage;
typedef struct __attribute__((packed)) {
  uint8_t len;
  uint8_t string[kMaxStringSize];
} Text;
typedef struct __attribute__((packed)) {
  uint8_t posx;
  uint8_t posy;
  uint8_t len;
  uint8_t string[kMaxStringSize];
} PositionedText;
typedef struct __attribute__((packed)) {
  uint8_t posx;
  uint8_t posy;
  uint16_t bg_collor;
  uint16_t fg_collor;
  uint8_t len;
  uint8_t string[kMaxStringSize];
} ColloredText;

typedef struct __attribute__((packed)) {
  Type type;
  union {
    OtImage ot_image;
    Text text;
    PositionedText positioned_text;
    ColloredText collored_text;
  } u;
} Rendable;

typedef struct {
  dif_i2c_t *i2c;
  uint8_t items_count;
  uint8_t addr;
} EepromReader;

status_t eeprom_init_rendable(EepromReader *reader, context_t *ctx);
status_t eeprom_reader_next_rendable(EepromReader *reader, Rendable *rendable);

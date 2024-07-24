// Copyright lowRISC contributors.
// Licensed under the Apache License, Version 2.0, see LICENSE for details.
// SPDX-License-Identifier: Apache-2.0

#include "btn.h"

#include "sw/device/lib/runtime/hart.h"
#include "sw/device/lib/runtime/ibex.h"
#include "sw/device/lib/runtime/print.h"

enum {
  kBtnDebounceMillis = 100,
};

status_t scan_buttons(context_t *ctx, uint32_t timeout) {
  ibex_timeout_t deadline = ibex_timeout_init(timeout * 1000);
  dif_gpio_pin_t pins[] = {ctx->pins.btn_up, ctx->pins.btn_down,
                           ctx->pins.btn_left, ctx->pins.btn_right,
                           ctx->pins.btn_ok};
  static size_t i = 0;
  do {
    i = (i + 1) % ARRAYSIZE(pins);
    bool state = true;
    TRY(dif_gpio_read(ctx->gpio, pins[i], &state));
    if (!state) {
      busy_spin_micros(kBtnDebounceMillis * 1000);
      TRY(dif_gpio_read(ctx->gpio, pins[i], &state));
      if (!state) {
        // LOG_INFO("Pin[%u]:%u pressed", i, pins[i]);
        return OK_STATUS((int32_t)i);
      }
    }

  } while (!ibex_timeout_check(&deadline));

  // LOG_INFO("Btn scan timeout");
  return DEADLINE_EXCEEDED();
}

// Copyright lowRISC contributors.
// Licensed under the Apache License, Version 2.0, see LICENSE for details.
// SPDX-License-Identifier: Apache-2.0

#ifndef SPI_DISPLAY_BTN_H
#define SPI_DISPLAY_BTN_H

#include "context.h"
#include "sw/device/lib/base/status.h"

status_t scan_buttons(context_t *ctx, uint32_t timeout);

#endif

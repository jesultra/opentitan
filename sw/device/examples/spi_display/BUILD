# Copyright lowRISC contributors.
# Licensed under the Apache License, Version 2.0, see LICENSE for details.
# SPDX-License-Identifier: Apache-2.0

load(
    "@bazel_skylib//lib:dicts.bzl",
    "dicts",
)
load(
    "//rules/opentitan:defs.bzl",
    "EARLGREY_SILICON_OWNER_ROM_EXT_ENVS",
    "cw310_params",
    "opentitan_test",
    "silicon_params",
)

package(default_visibility = ["//visibility:public"])

opentitan_test(
    name = "spi_display_demo",
    srcs = [
        "main.c",
    ],
    copts = [
        "-nostdlib",
        "-ffreestanding",
        # Disable the date-time warning only for the hello world program.
        "-Wno-date-time",
    ],
    exec_env = dicts.add(
        EARLGREY_SILICON_OWNER_ROM_EXT_ENVS,
        {
            "//hw/top_earlgrey:fpga_cw340_rom_with_fake_keys": None,
        },
    ),
    silicon = silicon_params(
        timeout = "eternal",
        interface = "ftdi",
    ),
    deps = [
        ":spi_display_app",
        "//sw/device/lib/base:mmio",
    ],
)

cc_library(
    name = "spi_display_app",
    srcs = [
        "aes_demo.c",
        "app.c",
        "btn.c",
        "i2c_eeprom.c",
        "self_test.c",
        "screen.c",
        "spi_passthrough.c",
    ],
    hdrs = [
        "app.h",
        "btn.h",
        "context.h",
        "demos.h",
        "screen.h",
        "i2c_eeprom.h",
    ],
    deps = [
        ":opentitan_libs",
        "//sw/device/examples/spi_display/images:logo_lowrisc_05_80px",
        "//sw/device/examples/spi_display/images:logo_opentitan_160_39px",
        "//sw/device/examples/spi_display/images:logo_tux_80_95px",
        "//sw/device/examples/spi_display/images:shape_opentitan_80_80px",
        "//sw/device/lib/crypto",
        "@display_drivers//:display_drivers_lib",
    ],
)

cc_library(
    name = "opentitan_libs",
    deps = [
        "//hw/top_earlgrey/sw/autogen:top_earlgrey",
        "//sw/device/examples:demos",
        "//sw/device/lib/arch:device",
        "//sw/device/lib/crt",
        "//sw/device/lib/dif:aes",
        "//sw/device/lib/dif:gpio",
        "//sw/device/lib/dif:i2c",
        "//sw/device/lib/dif:pinmux",
        "//sw/device/lib/dif:spi_device",
        "//sw/device/lib/dif:spi_host",
        "//sw/device/lib/dif:uart",
        "//sw/device/lib/runtime:hart",
        "//sw/device/lib/runtime:log",
        "//sw/device/lib/runtime:print",
        "//sw/device/lib/testing:aes_testutils",
        "//sw/device/lib/testing:i2c_testutils",
        "//sw/device/lib/testing:pinmux_testutils",
        "//sw/device/lib/testing:profile",
        "//sw/device/lib/testing:spi_device_testutils",
        "//sw/device/lib/testing:spi_flash_testutils",
        "//sw/device/lib/testing/test_framework:check",
        "//sw/device/lib/testing/test_framework:ottf_main",
        "//sw/device/lib/testing/test_framework:ottf_start",
        "//sw/device/lib/testing/test_framework:ottf_test_config",
    ],
)


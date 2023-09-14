# Copyright lowRISC contributors (OpenTitan project).
# Licensed under the Apache License, Version 2.0, see LICENSE for details.
# SPDX-License-Identifier: Apache-2.0

load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")

display_drivers = module_extension(
    implementation = lambda _: _display_drivers_repos(),
)

def _display_drivers_repos():
    http_archive(
        name = "display_drivers",
        build_file = Label("//third_party/display_drivers:BUILD.display_drivers.bazel"),
        sha256 = "b47bb2541eac520d664d5d2a91a85f6a060b0560d0d3caa10afe2f89f229c88a",
        strip_prefix = "display_drivers-0.1.0",
        urls = [
            "https://github.com/engdoreis/display_drivers/archive/refs/tags/v0.1.0.tar.gz",
        ],
    )

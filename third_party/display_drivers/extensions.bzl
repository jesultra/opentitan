# Copyright lowRISC contributors (OpenTitan project).
# Licensed under the Apache License, Version 2.0, see LICENSE for details.
# SPDX-License-Identifier: Apache-2.0

load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")
load("@bazel_tools//tools/build_defs/repo:git.bzl", "git_repository")

display_drivers = module_extension(
    implementation = lambda _: _display_drivers_repos(),
)

def _display_drivers_repos():
    http_archive(
        name = "display_drivers",
        build_file = Label("//third_party/display_drivers:BUILD.display_drivers.bazel"),
        sha256 = "91337e6262f9786310123af654f801fb99527ce12dba2bd5d021ec0f881e2944",
        strip_prefix = "display_drivers-0.2.0",
        urls = [
            "https://github.com/engdoreis/display_drivers/archive/refs/tags/v0.2.0.tar.gz",
        ],
    )

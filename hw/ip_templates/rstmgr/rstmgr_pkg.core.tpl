CAPI=2:
# Copyright lowRISC contributors (OpenTitan project).
# Licensed under the Apache License, Version 2.0, see LICENSE for details.
# SPDX-License-Identifier: Apache-2.0
name: ${instance_vlnv("lowrisc:ip:rstmgr_pkg:0.1")}
description: "Reset manager package"
virtual:
  - lowrisc:ip_interfaces:rstmgr_pkg

filesets:
  files_rtl:
    depend:
      - ${instance_vlnv("lowrisc:ip:pwrmgr_pkg", pwrmgr_vlnv_prefix)}
    files:
      - rtl/rstmgr_reg_pkg.sv
      - rtl/rstmgr_pkg.sv
    file_type: systemVerilogSource

targets:
  default:
    filesets:
      - files_rtl

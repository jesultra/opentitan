# SPI Display Demo

## Overview

`Spi_display_demo` is the demo program used to show some key functionalities of opentitan such as the spi_passthough and AES.
The App is designed to work on either FPGA with a display mounted on the PMOD port or on the Voyager board.
On boot it shows a menu on the display with a few demo modes:
* AES ECB/CDC
* SPI passthrough

### AES ECB/CDC
 Once selected on the main menu, the demo will loop drawing two tux images side by side and them encrypting both images, one with AES ECB and the other with AES CDC.

### SPI passthrough
Once selected on the main menu, the demo with show a menu with the options:
 * Enable Secure Boot: This will authenticate the flash and show and enable spi passthrough if the signature is correct, otherwise it will show an error message.
 * Disable Secure Boot: This will enable spi passthrough inconditionally.
 * Exit: Return to the main menu.

## Loading
On the FPGA CW340
```sh
bazel test --test_output=streamed \
//sw/device/examples/spi_display:spi_display_demo_fpga_cw340_rom_with_fake_keys
```

On the Voyager board
```sh
bazel test --//signing:token=//signing/tokens:cloud_kms_proda \
--test_output=streamed \
//sw/device/examples/spi_display:spi_display_demo_silicon_owner_proda_rom_ext
```

## Voyager board
![](block_diagram.drawio.svg)


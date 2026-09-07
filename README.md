# nanoDAP C6 (CMSIS-DAP-C6)

Standalone CMSIS-DAP v2 debug probe firmware for the
STM32F103C6T6 "mini" board, derived from this repos' STM32F103C8T6
CMSIS-DAP_SWO project, reduced to a minimal pin-compatible C6 build.

## Features

- CMSIS-DAP over HID (SWD transport) - flash / RAM debug of Cortex-M targets
- Combined HID + CDC composite USB device, serial number from chip UID
- CDC virtual COM port bridging USART1 (PA9 TX / PA10 RX, 115200 8-N-1)

## Layout

- `doc/` architecture, build, verified results, feature lists
- `hardware/` pinout and wiring
- `test/` CDC loopback and SWD regression scripts
- `src/` sources (`dap/`, `usb/`, `config/`, `cmsis/`)
- `startup/` vector startup and linker script
- `cmake/` arm-none-eabi toolchain file

See `doc/README.md` for build and usage, `doc/features.md` for the
implemented / not-implemented matrix, `hardware/pinout.md` for wiring.

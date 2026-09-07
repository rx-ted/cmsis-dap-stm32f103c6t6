# CMSIS-DAP-C6

Firmware for an STM32F103C6T6 (TSSOP20 / "mini") board used as a CMSIS-DAP
debug probe, derived from the STM32F103C8T6 CMSIS-DAP_SWO variant of the
nanoDAP project.

## Architecture

```txt
                 USB (FS 12Mbps, PA11/PA12)
        +------------------------------------+
        |  Composite device:                 |
        |   EP0   control                    |
        |   EP1   CDC notification (IN)      |
        |   EP2   CDC data (IN/OUT)   <---+  |
        |   EP3   HID IN/OUT (DAP)      <-|--|---- CMSIS-DAP (HID)
        +---------+------------+-----------+  |      DAP.c / SW_DP.c
                  |            |              |
                  |  EP2 OUT   |  EP2 IN      |
                  |  (host->)   |  (->host)    |
                  |            |              |
        +---------v----+   +---v------------+  |
        |  CDC TX ring |   | CDC RX ring    |  |
        |  (512 B)     |   | (512 B)        |  |
        +---------+----+   +---^------------+  |
                  |            |               |
        +---------v----+   +---v------------+  |
        |  USART1 TX   |   | USART1 RX      |--+  PA9  (TX, AF PP)
        |  (ISR only)  |   | (ISR, RXNEIE)  |     PA10 (RX, floating)
        +--------------+   +----------------+    USART1 115200 8-N-1

        SWD header:  PA2 SWDIO / PA4 SWCLK / PA6 nRESET
        LEDs:        PB8 running (1 Hz blink) / PB12 connected (USB config)
        Serial no.:  "C6" + 8 hex digits from the 96-bit chip UID
```

Data path design:

- Both bridge directions are single-producer/single-consumer rings that use
  head/tail indices only (no shared counter), so ISR preemption cannot race.
- The USART TX side is driven solely by the TXE interrupt - the main loop
  only enables TXEIE, it never writes DR (this removed a data-loss race).
- The USART RX ISR clears sticky overrun/error flags (ORE/NE/FE/PE).
- EP2 OUT uses the peripheral's natural single-buffered auto-NAK as flow
  control: the endpoint is re-armed only while at least one packet of ring
  space remains, and re-armed again from the drain path once freed.
- EP2 IN and EP2 OUT staging buffers are separate (no shared buffer).

## Build

Requirements: CMake >= 3.22, arm-none-eabi-gcc 15 (xPack used here) on PATH.

The CMake layout mirrors the sibling LED project (CubeMX-style): compiler
flags live in `cmake/gcc-arm-none-eabi.cmake`, sources in
`cmake/stm32cubemx/`, and builds are driven by `CMakePresets.json`.
Presets use the Unix Makefiles generator (no Ninja required); switch the
`generator` to `Ninja` in `CMakePresets.json` if Ninja is installed.

```sh
cmake --preset Release        # flashing build (-Os), -> build/Release/
cmake --build --preset Release
```

Output: `build/Release/nanoDAP-C6.hex` / `build/Release/nanoDAP-C6.bin`.
Current footprint: FLASH 10368/32768 B (31.6%), RAM 2588/10240 B (25.3%).

Other presets: `Debug` (-O0 -g3) for debugging, `ubtest`
(`-DCDC_USB_LOOPBACK_TEST`) for the USB-internal loopback variant; each
builds into its own `build/<presetName>/` directory.

Flash the probe with an ST-Link (SM3/NRST/GND wiring, see `doc/en/pinout.md`).

Flashing, erasing, read-protection and other OpenOCD device operations are
covered in `doc/en/flash.md`.

## Verified results

SWD / CMSIS-DAP (via macOS + pyocd):

- `pyocd list` -> `ARM CMSIS-DAP-C6` UID `C64691CB05`
- `pyocd commander -u C64691CB05 -t cortex_m` status / halt / resume / reset
- CPUID 0x411FC231 (Cortex-M3), RAM reads/writes, 64-byte flash block reads

Flashing a target Blue Pill (STM32F103C8T6):

```sh
pyocd pack install STM32F103C8        # one time
pyocd flash -t stm32f103c8 -M halt --base-address 0x08000000 LED.bin
pyocd erase -t stm32f103c8 -M halt --chip
```

`-M halt` is used because this board's nRESET semantics do not match what
pyocd's stm32 pack targets expect for under-reset connect (see features).

CDC loopback (PA9<->PA10 shorted): byte-perfect at the line rate - small
bursts, multi-burst with gaps, and a paced 8 KB/s 1 KB stream all round-trip
losslessly. See `test/`.

## Notes

- Toolchain/pack details are recorded in `test/README.md`.
- Known behaviours and gaps are listed in `doc/en/features.md`.

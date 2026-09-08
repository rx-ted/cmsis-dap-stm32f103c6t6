# Flashing & Device Operations

## Firmware footprint

Build presets:

| Preset  | Build type | Footprint                  | Use                          |
| ------- | ---------- | -------------------------- | ---------------------------- |
| Release | -Os -g0    | FLASH 10368 B / RAM 2588 B | flashing build (default)     |
| Debug   | -Os -g3    | FLASH 10368 B / RAM 2588 B | GDB / VS Code debug symbols  |
| ubtest  | Debug      | —                          | CDC USB loopback test        |

Output files: `build/Release/nanoDAP-C6.hex` and `build/Release/nanoDAP-C6.bin`.

## Flashing the C6 board itself (local)

The C6 mini board has a SWD header (PA2 SWDIO / PA4 SWCLK / GND / 3.3V).
Connect an ST-Link V2 to these pins plus NRST (PA6 optional but recommended).

Wiring (ST-Link → C6):

| ST-Link | C6             |
| ------- | -------------- |
| SWDIO   | PA2            |
| SWCLK   | PA4            |
| GND     | GND            |
| 3.3V    | 3V3            |
| NRST    | PA6 (optional) |

Commands (from a configured build directory):

```sh
cmake --preset Release
cmake --build --preset Release
make -C build/Release flash       # program + verify + reset
```

Or with the native cmake driver:

```sh
cmake --build --preset Release --target flash
```

**Reset-line dependency:** the `flash` target uses `reset_config srst_only`
(i.e. the ST-Link NRST pin). If NRST is not wired, flashing still works but
the trailing `reset` fails with "Unable to reset target". Flash with a
software reset instead:

```sh
openocd -f interface/stlink.cfg \
  -c "transport select hla_swd" \
  -f target/stm32f1x.cfg \
  -c "adapter speed 10000" \
  -c "reset_config none" \
  -c "init" -c "halt" \
  -c "flash write_image erase build/Release/nanoDAP-C6.hex" \
  -c "verify_image build/Release/nanoDAP-C6.hex" \
  -c "reset run" -c "shutdown"
```

Other local targets:

```sh
make -C build/Release check       # read IDCODE, verify connectivity
make -C build/Release erase       # full chip erase
make -C build/Release lock        # enable read-protection (RDP level 1)
make -C build/Release unlock      # clear read-protection (ERASES ENTIRE CHIP)
make -C build/Release reset       # software reset halt
make -C build/Release gdbserver   # start GDB server on :3333 (foreground)
```

## Flashing a target board via C6 (CMSIS-DAP)

When the C6 acts as a CMSIS-DAP debug probe, it can flash another STM32
board (e.g. a Blue Pill). Wire the C6 SWD header to the target:

C6 → Target (SWD):

| C6  | Target |
| --- | ------ |
| PA2 | SWDIO  |
| PA4 | SWCLK  |
| PA6 | NRST   |
| GND | GND    |

Configure with CMSIS-DAP interface and re-build:

```sh
cmake --preset Release \
  -DOPENOCD_INTERFACE=interface/cmsis-dap.cfg \
  -DOPENOCD_TRANSPORT=swd
cmake --build --preset Release
make -C build/Release check       # verify target connectivity
make -C build/Release flash       # program the target
```

To flash a custom hex/bin file instead of the probe firmware, use openocd
directly:

```sh
openocd -f interface/cmsis-dap.cfg \
  -c "transport select swd" \
  -f target/stm32f1x.cfg \
  -c "adapter speed 10000" \
  -c "program /path/to/firmware.hex verify reset exit"
```

## Lock / Unlock (read protection)

```sh
make -C build/Release lock        # RDP level 1: flash contents unreadable
make -C build/Release unlock      # clear RDP: WARNING, erases entire flash
```

After unlocking, the chip is blank and must be re-programmed.

## Troubleshooting

**"Error: connect failed" / "SWD/JTAG-DP but no APs"**

- Check wiring: SWDIO, SWCLK, GND must be connected.
- Verify the target is powered (3.3V).
- Try a lower adapter speed: `-DOPENOCD_SPEED=1000`.

**"Error: couldn't open device"**

- openocd cannot find the debug probe. Check USB connection.
- Run `lsusb` (Linux) or check System Information (macOS) to confirm
  the probe is enumerated.
- For ST-Link: ensure no other GDB/OpenOCD session is holding the probe.

**"Error: read protect is enabled" (RDP level 1)**

- The chip is locked. Unlock first (erases flash):
  `make -C build/Release unlock`
- Then re-flash.

**openocd not found**

- Install: `brew install openocd`
- Or set `OPENOCD_EXECUTABLE` at configure time:
  `cmake -DOPENOCD_EXECUTABLE=/path/to/openocd ...`

**pyocd vs openocd**

The previous setup used pyocd (via the `/tmp/daptest` venv) for target
flashing. OpenOCD provides the same functionality without a Python venv
and is the recommended path going forward. Both tools can coexist.

# Pinout (BOARD_C6, STM32F103C6T6)

SWD header uses three GPIOs configured as push-pull outputs; there is no
level sensing (no VTref pin).

| Pin | Function                        | Details                                      |
|-----|---------------------------------|----------------------------------------------|
| PA2 | SWDIO (SWD data)                | push-pull out, 50 MHz                        |
| PA4 | SWCLK (SWD clock)               | push-pull out, 50 MHz                        |
| PA6 | nRESET                          | push-pull out; idle high, pulse low to reset |
| PA9 | USART1_TX (CDC host <- MCU)     | AF push-pull, 115200 8-N-1                   |
| PA10| USART1_RX (CDC host -> MCU)     | input floating                               |
| PA11| USB DM                          | AF push-pull 10 MHz                          |
| PA12| USB DP                          | AF push-pull 10 MHz; board has a fixed 3.3 V |
|     |                                 | pull-up, USB always attached, no SW control  |
| PB8 | LED_RUNNING                     | main-loop ~1 Hz blink                        |
| PB12| LED_CONNECTED                   | on when USB device is CONFIGURED             |
| 3V3 | board/device power              | USB VBUS regulated on the board              |
| GND | ground (shared with target)     |                                             |

TDI (PB11) / TDO-SWO (PA5) are declared for source compatibility but are
NOT populated on the C6 board (JTAG / trace removed from this variant).

## SWD nRESET drive

`vResetTarget` output semantics as coded:

- bit = 1 -> pulse nRESET low then high (reset the target), then SWDIO
  released high
- bit = 0 -> drive nRESET high (leave target running/reset line idle)

This is the opposite of the CMSIS-DAP convention (bit=1 usually means drive
high), which is why pyocd pack targets need `-M halt` rather than the
default under-reset connect.

## Wiring to a Blue Pill (STM32F103C8T6)

Probe pin -> target pin:

- GND   -> GND
- PA2   -> SWDIO
- PA4   -> SWCLK
- PA6   -> NRST (optional but used when reset control is needed)

Target logic must be 3.3 V; both boards must share a common ground.
Do not power the target from the probe unless the probe's 3.3 V is
available on your wiring.

## CDC loopback (for bridge tests only)

Short PA9 to PA10 on the probe. This is the setup used by `test/cdc_*.py`;
remove the jumper for normal point-to-point usage.
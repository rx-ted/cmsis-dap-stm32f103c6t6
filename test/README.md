# Tests

Microscripts used while bringing up the C6 probe. They are not a test
framework - edit the serial port constant if the board serial differs.

Prerequisites:

- Python 3 with `pyserial` (a venv at `/tmp/daptest` held pyserial 3.5)
- probe enumerated as `/dev/cu.usbmodemC64691CB052` (macOS)
- `pyocd` with the `Keil.STM32F1xx_DFP` pack for the stm32f103c8 target
- probe serial port = the only CDC ACM device of the probe

## CDC bridge tests

Put the probe in loopback: short PA9 <-> PA10, run one script, un-short.

| Script          | What it checks                                       |
|-----------------|------------------------------------------------------|
| cdc_test.py     | single 51-byte burst round-trip (FAIL on any loss)   |
| cdc_test2.py    | burst sizes 4/5/8/16/32/63                           |
| cdc_test3.py    | multi-burst with gaps (hung an old firmware bug)     |
| cdc_test4.py    | small burst sizes 2..16                              |
| cdc_test5.py    | multi-burst with gaps + unpaced 2 kB stream          |
| cdc_test6.py    | paced 8 KB/s 1 KB stream (must be OK) then unpaced 2 kB |
| cdc_test7.py    | paced 12.8 KB/s (just above line rate)               |
| cdc_test8.py    | byte-level diff of a paced 1 KB stream               |
| cdc_ub.py       | USB-internal loopback build: unpaced 2 kB / 8 kB     |
| cdc_ub2.py      | aligns received stream to find drop positions        |

Expected behaviour (current firmware):

- Line-rate traffic (paced <=11520 B/s, bursts, gaps): lossless.
- Unpaced flood over the line rate may drop ~1 % (first byte of some
  64-byte batches). See `doc/features.md`.

`cdc_ub*` scripts run against a special USB-loopback build:

```sh
cmake --preset ubtest
cmake --build --preset ubtest
```

## SWD / DAP tests

| Script              | What it checks                          |
|---------------------|-----------------------------------------|
| dap_pins.py         | drive + read back SWDIO / SWCLK / nRESET|
| dap_pyocd_protocol.py | DAP_Info etc through pyocd             |
| dap_swd_retry.py    | SWD connect with retries                |
| dump_probe.py       | read probe flash back and diff the build |

Flash readback (`dump_probe.py`) reads whatever is on the probe's SWD
lines; connect the target properly (or the ST-Link side) first.
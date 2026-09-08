# Feature matrix

## Implemented (verified)

### SWD / debug (CMSIS-DAP over HID)

- HID transport on EP3, CMSIS-DAP protocol (DAP_Info reports PROTO_VER 2.0.0).
- SWD transport: connect, read/write AP-IDR, CPUID, flash reads
  (incl. 64-byte block reads to PMA via bulk reads), RAM read/write,
  reset / halt / go / resume.
- Verified end-to-end against a Blue Pill (STM32F103C8T6): CPUID
  0x411FC231, target reset, halt/resume, memory reads (SWD).
- Target flash programming through pyocd pack algorithms
  (Keil.STM32F1xx_DFP): `pyocd flash -t stm32f103c8 -M halt ...` -> chip
  erase + page program + verify, target then runs the programmed image.

### USB composite device

- One device, three interfaces: HID (DAP), CDC ACM notify, CDC data.
  Endpoints: EP0 ctrl, EP1 CDC notify IN, EP2 CDC data IN/OUT, EP3 HID.
- Vendor "ARM", product "CMSIS-DAP-C6".
- Serial number generated at run time from the 96-bit chip unique ID as
  "C6" + 8 hex digits (e.g. C64691CB05), written into the USB serial
  descriptor and returned by DAP_Info/DAP_SER_NUM.
- No software USB connect pin: the board keeps USB always attached.

### CDC virtual serial port (USART1 bridge)

- USART1 at PA9 (TX) / PA10 (RX), 115200 8-N-1, RX interrupt driven.
- SPSC ring buffers (512 B each), ISR-only USART writes, ORE/error flags
  cleared in the RX ISR, natural-NAK flow control on EP2 OUT.
- Byte-perfect at the line rate (verified with loopback on PA9/PA10).
- USART1 IRQ at the same NVIC priority as USB so RX cannot be starved.

### Indicators

- PB8 LED_RUNNING: blinks ~1 Hz in the main loop.
- PB12 LED_CONNECTED: on once the USB device reaches CONFIGURED state.

## Not implemented / limitations

- **SWO / ITM trace**: the C6 board does not populate a SWO/TDO trace pin.
- **JTAG**: not currently supported. JTAG transport (TCK/TMS/TDI/TDO) is
  not enabled on the C6 board. Future firmware releases will add JTAG
  support using the PA9/PA10 pins currently assigned to the CDC serial
  bridge.
- **VTref / target voltage sensing**: the SWD lines are push-pull outputs;
  there is no level detector or level shifting. Target logic must be 3.3 V
  and share a common ground with the probe.
- **CDC baud-rate selection**: host baud requests are ignored; USART1 is
  fixed at 115200 (BRR computed once at 72 MHz PCLK2).
- **CDC hardware flow control**: RTS/CTS lines are not exposed; only
  TX/RX are bridged. Ring-based flow control protects the USART side.
- **Under-reset connect**: `vResetTarget` treats `value bit=1` as
  "pulse nRESET low then high" (opposite to the CMSIS-DAP convention where
  bit=1 means hold high). With pyocd stm32 pack targets the default
  under-reset connect returns No ACK, so `-M halt` is used instead and is
  reliable.
- **Host flood / over-rate**: an unpaced host writing far above the UART
  line rate can drop ~1 % of bytes (first byte of some 64-byte USB batches).
  This is a characteristic of single-buffered EP2 RX; dual-buffered bulk
  endpoints would be the robust fix. Line-rate traffic is lossless.
- **Not USB high speed**: FS 12 Mbps as provided by the STM32F103
  on-chip USB FS peripheral (no USB 2.0 HS PHY).
- **No option-byte / RDP / WRP tooling**: the probe does not implement
  target memory protection management; it leaves the target's option bytes
  untouched.
- **pyocd "Board ID C646 is not recognized"** warning is benign: the serial
  prefix is parsed as a board ID and looked up in the mbed board database.

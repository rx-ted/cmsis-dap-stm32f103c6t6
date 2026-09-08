# CMSIS-DAP-C6

基于 STM32F103C6T6（TSSOP20 / "mini" 板）的 CMSIS-DAP 调试探针固件，
源自 nanoDAP 项目的 STM32F103C8T6 CMSIS-DAP_SWO 变体，精简为引脚兼容
的 C6 构建。

## 架构

```
                 USB（全速 12Mbps，PA11/PA12）
        +------------------------------------+
        |  复合设备：                         |
        |   EP0   控制                        |
        |   EP1   CDC 通知（IN）              |
        |   EP2   CDC 数据（IN/OUT）  <---+   |
        |   EP3   HID IN/OUT（DAP）    <-|--|---- CMSIS-DAP（HID）
        +---------+------------+-----------+  |      DAP.c / SW_DP.c
                  |            |              |
                  |  EP2 OUT   |  EP2 IN      |
                  |  （主机→） |  （→主机）    |
                  |            |              |
        +---------v----+   +---v------------+  |
        |  CDC TX 环   |   | CDC RX 环      |  |
        |  （512 B）   |   | （512 B）      |  |
        +---------+----+   +---^------------+  |
                  |            |               |
        +---------v----+   +---v------------+  |
        |  USART1 TX   |   | USART1 RX      |--+  PA9  （TX，AF 推挽）
        |  （仅中断）  |   | （中断，RXNEIE）|     PA10 （RX，浮空输入）
        +--------------+   +----------------+    USART1 115200 8-N-1

        SWD 排针：  PA2 SWDIO / PA4 SWCLK / PA6 nRESET
        LED：        PB8 运行态（1 Hz 闪烁）/ PB12 已连接（USB 配置态）
        序列号：    "C6" + 96位芯片 UID 的 8 位十六进制
```

数据通路设计：

- 桥接两个方向均为单生产者/单消费者环形缓冲，只用头/尾索引（无共享
  计数器），因此中断抢占不会产生竞争。
- USART 发送侧仅由 TXE 中断驱动——主循环只使能 TXEIE，从不写 DR
  （这消除了此前一个数据丢失竞争）。
- USART RX 中断在 ISR 内清除粘滞的溢出/错误标志（ORE/NE/FE/PE）。
- EP2 OUT 使用外设原生的单缓冲自动 NAK 作为流控：仅在剩余至少一个
  包的空间时才重新使能端点，并在排空路径中再次使能。
- EP2 IN 与 EP2 OUT 的暂存缓冲相互独立（无共享缓冲）。

## 构建

要求：CMake >= 3.22，arm-none-eabi-gcc 15（此处使用 xPack）且在 PATH 上。

CMake 布局与兄弟项目 LED（CubeMX 风格）一致：编译器/链接器参数在
`cmake/gcc-arm-none-eabi.cmake`，源文件列表在 `cmake/stm32cubemx/`，
由 `CMakePresets.json` 驱动构建。预设使用 Unix Makefiles 生成器
（无需 Ninja）；若安装了 Ninja，可在 `CMakePresets.json` 中将
`generator` 改为 `Ninja`。

```sh
cmake --preset Release        # 烧录用构建（-Os），产出 build/Release/
cmake --build --preset Release
```

产物：`build/Release/nanoDAP-C6.hex` / `build/Release/nanoDAP-C6.bin`。
当前占用：FLASH 10368/32768 B（31.6%），RAM 2588/10240 B（25.3%）
（默认编译移除 JTAG；如需启用可设 `CDC_JTAG_SWITCH=1`）。

其它预设：`Debug`（-Os -g3）用于 GDB / VS Code，`ubtest`
（`-DCDC_USB_LOOPBACK_TEST`）用于 USB 内部回环变体；各自构建到
`build/<presetName>/` 目录。

用 ST-Link 烧录探针（SM3/NRST/GND 接线，见 `doc/zh/pinout.md`）。

烧录、擦除、读保护等 OpenOCD 设备操作见 `doc/zh/flash.md`。

## 已验证结果

SWD / CMSIS-DAP（macOS + pyocd）：

- `pyocd list` -> `ARM CMSIS-DAP-C6` UID `C64691CB05`
- `pyocd commander -u C64691CB05 -t cortex_m` status / halt / resume / reset
- CPUID 0x411FC231（Cortex-M3），RAM 读写，64 字节 flash 块读取

烧录目标 Blue Pill（STM32F103C8T6）：

```sh
pyocd pack install STM32F103C8        # 一次性
pyocd flash -t stm32f103c8 -M halt --base-address 0x08000000 LED.bin
pyocd erase -t stm32f103c8 -M halt --chip
```

这里用 `-M halt` 是因为该板的 nRESET 语义与 pyocd stm32 pack 目标对
under-reset 连接的预期不一致（见 features）。

CDC 回环（PA9<->PA10 短接）：在线速率下逐字节正确——小突发、带间隔
的多突发、以及 8 KB/s 的 1 KB 节流流全部无损回环。见 `test/`。

## 附注

- 工具链/pack 细节记录在 `test/README.md`。
- 已实现/未实现及限制见 `doc/zh/features.md`。
# 引脚定义（BOARD_C6，STM32F103C6T6）

SWD 排针使用三个配置为推挽输出的 GPIO；无电平检测（无 VTref 引脚）。

| 引脚  | 功能                          | 说明                                         |
|-------|-------------------------------|----------------------------------------------|
| PA2   | SWDIO（SWD 数据）             | 推挽输出，50 MHz                             |
| PA4   | SWCLK（SWD 时钟）             | 推挽输出，50 MHz                             |
| PA6   | nRESET                        | 推挽输出；空闲高，低脉冲复位                  |
| PA9   | USART1_TX（CDC 主机 <- MCU）  | 复用推挽，115200 8-N-1                        |
| PA10  | USART1_RX（CDC 主机 -> MCU）  | 浮空输入                                     |
| PA11  | USB DM                        | 复用推挽 10 MHz                              |
| PA12  | USB DP                        | 复用推挽 10 MHz；板上带固定 3.3 V            |
|       |                               | 上拉，USB 始终连接，无软件控制                |
| PB8   | LED_RUNNING                   | 主循环约 1 Hz 闪烁                            |
| PB12  | LED_CONNECTED                 | USB 设备进入 CONFIGURED 状态时点亮           |
| 3V3   | 板/设备电源                   | 板上由 USB VBUS 稳压                         |
| GND   | 地（与目标共地）              |                                              |

TDI（PB11）/ TDO-SWO（PA5）仅为源码兼容保留声明，C6 板上**未引出**
（本变体移除了 JTAG / 跟踪）。

## SWD nRESET 驱动

`vResetTarget` 按代码实现的输出语义：

- bit = 1 -> 先拉低 nRESET 再拉高（复位目标），随后 SWDIO 释放为高
- bit = 0 -> 驱动 nRESET 为高（保持目标运行/复位线空闲）

这与 CMSIS-DAP 惯例相反（惯例 bit=1 表示驱动高），因此 pyocd pack
目标需用 `-M halt` 而非默认的 under-reset 连接。

## 连接 Blue Pill（STM32F103C8T6）

探针引脚 -> 目标引脚：

- GND   -> GND
- PA2   -> SWDIO
- PA4   -> SWCLK
- PA6   -> NRST（可选，需要复位控制时使用）

目标逻辑必须为 3.3 V；两板必须共地。除非接线提供了探针的 3.3 V，
否则不要从探针给目标供电。

## CDC 回环（仅桥接测试）

将探针 PA9 短接 PA10。这是 `test/cdc_*.py` 使用的接线；常规点对点
使用时移除跳线。
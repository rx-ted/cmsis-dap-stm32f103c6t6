# 功能矩阵

## 已实现（已验证）

### SWD / 调试（HID 上的 CMSIS-DAP）
- HID 传输（EP3），CMSIS-DAP 协议（DAP_Info 报告 PROTO_VER 2.0.0）。
- SWD 传输：连接、AP-IDR 读写、CPUID、flash 读取（含经 PMA 的
  64 字节块读）、RAM 读写、复位 / 停机 / 运行 / 恢复。
- JTAG 传输（来自 V1.4 CMSIS-DAP 驱动）：JTAG_Sequence /
  JTAG_Configure / JTAG_IDCode / JTAG_Transfer / JTAG_TransferBlock，
  在 TCK(PA4)/TMS(PA2)/TDI(PA9)/TDO(PA10) 上软件位操作。
- 已对 Blue Pill（STM32F103C8T6）端到端验证：CPUID 0x411FC231、
  目标复位、halt/resume、内存读取（SWD）。
- 经 pyocd pack 算法（Keil.STM32F1xx_DFP）对目标 flash 编程：
  `pyocd flash -t stm32f103c8 -M halt ...` -> 芯片擦除 + 页编程 + 校验，
  目标随后运行烧写的镜像。

### USB 复合设备
- 单设备、三接口：HID（DAP）、CDC ACM 通知、CDC 数据。
  端点：EP0 控制、EP1 CDC 通知 IN、EP2 CDC 数据 IN/OUT、EP3 HID。
- 厂商 "ARM"，产品 "CMSIS-DAP-C6"。
- 序列号基于 96 位芯片唯一 ID 在运行时生成，形如 "C6" + 8 位十六进制
  （如 C64691CB05），写入 USB 序列号描述符并由
  DAP_Info/DAP_SER_NUM 返回。
- 无软件 USB 连接引脚：板卡始终保持 USB 连接。

### CDC 虚拟串口（USART1 桥接）
- USART1 位于 PA9（TX）/ PA10（RX），115200 8-N-1，RX 中断驱动。
- SPSC 环形缓冲（各 512 B），仅中断写 USART，RX ISR 清除
  ORE/错误标志，EP2 OUT 上自然 NAK 流控。
- 在线速率下逐字节正确（已通过 PA9/PA10 回环验证）。
- USART1 IRQ 与 USB 同级 NVIC 优先级，RX 不会被饿死。

### 运行时 CDC <-> JTAG 引脚共享（PA9/PA10）
- TDI（PA9）与 TDO（PA10）复用 USART1 的 TX/RX 引脚。默认探针以
  CDC 模式启动（USART1 工作）。
- 当主机在 CMSIS-DAP 命令中选择了 JTAG 端口（`DAP_Connect` 端口 2，
  或处于 JTAG 会话中）时，USART1 被关闭，PA9 重新配置为推挽输出
  TDI，PA10 为浮空输入 TDO。
- 在 JTAG 静默超过 `CDC_JTAG_TIMEOUT_MS`（默认 2000）后，探针切回
  CDC（USART1 以 115200 重新初始化）。
- 编译期总开关：`CDC_JTAG_SWITCH = 1`（默认，两种模式）或 `0`
  （强制仅 CDC，JTAG 编译移除）。见 `cdc` CMake 预设。

### 指示灯
- PB8 LED_RUNNING：主循环约 1 Hz 闪烁。
- PB12 LED_CONNECTED：USB 设备进入 CONFIGURED 状态后点亮。

## 未实现 / 限制

- **SWO / ITM 跟踪**：C6 板未引出 SWO/TDO 跟踪引脚（JTAG TDO 在 PA10
  上与 USART1 RX 共享，不用于跟踪）。
- **JTAG 引脚与 CDC 物理共享**：PA9/PA10 仅在 JTAG 会话期间充当
  TDI/TDO；期间 CDC 串口桥被关闭。空闲 2 s（见上文切换节）后引脚
  回到 USART1。
- **VTref / 目标电压检测**：SWD 线为推挽输出；无电平检测或电平转换。
  目标逻辑必须为 3.3 V，并与探针共地。
- **CDC 波特率选择**：主机波特率请求被忽略；USART1 固定 115200
  （72 MHz PCLK2 下一次性计算 BRR）。
- **CDC 硬件流控**：未引出 RTS/CTS；仅桥接 TX/RX。环形缓冲流控保护
  USART 侧。
- **under-reset 连接**：`vResetTarget` 将 `value bit=1` 解释为
  "先拉低 nRESET 再释放"（与 CMSIS-DAP 惯例 bit=1 表示保持高的
  约定相反）。pyocd stm32 pack 目标的默认 under-reset 连接返回
  No ACK，因此使用 `-M halt`，可靠。
- **主机洪水 / 超线速**：未节流的主机以远超 UART 线速写入时，可能
  丢失约 1% 的字节（某些 64 字节 USB 批次的首字节）。这是单缓冲
  EP2 RX 的特性；双缓冲批量端点才是稳健的修复。线速率流量无损。
- **非 USB 高速**：由 STM32F103 片上 USB FS 外设提供全速 12 Mbps
  （无 USB 2.0 HS PHY）。
- **无 option-byte / RDP / WRP 工具**：探针不实现目标内存保护管理，
  不触碰目标的 option 字节。
- **pyocd "Board ID C646 is not recognized" 警告** 无害：序列号前缀被
  解析为 board ID 并在 mbed 板卡数据库中查询。
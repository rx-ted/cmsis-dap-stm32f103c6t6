# 烧录与设备操作

## 固件大小

| 区域  | 已用    | 总量  | 占比  |
|-------|---------|-------|-------|
| FLASH | 10368 B | 32 KB | 31.6% |
| RAM   | 2588 B  | 10 KB | 25.3% |

产物文件：`build/Release/nanoDAP-C6.hex` 与 `build/Release/nanoDAP-C6.bin`。

## 烧录 C6 板本身（本地）

C6 mini 板有 SWD 排针（PA2 SWDIO / PA4 SWCLK / GND / 3.3V）。将
ST-Link V2 接到这些引脚，外加 NRST（PA6，可选但推荐）。

接线（ST-Link → C6）：

| ST-Link | C6             |
|---------|----------------|
| SWDIO   | PA2            |
| SWCLK   | PA4            |
| GND     | GND            |
| 3.3V    | 3V3            |
| NRST    | PA6（可选）    |

命令（在已配置的 build 目录）：

```sh
cmake --preset Release
cmake --build --preset Release
make -C build/Release flash       # 编程 + 校验 + 复位
```

或使用 CMake 原生驱动：

```sh
cmake --build --preset Release --target flash
```

其它本地目标：

```sh
make -C build/Release check       # 读 IDCODE，验证连通性
make -C build/Release erase       # 全片擦除
make -C build/Release lock        # 使能读保护（RDP level 1）
make -C build/Release unlock      # 清除读保护（会擦除整片）
make -C build/Release reset       # 软件复位 halt
make -C build/Release gdbserver   # 启动 GDB server 于 :3333（前台）
```

## 经 C6 烧录目标板（CMSIS-DAP）

当 C6 作为 CMSIS-DAP 调试探针时，可烧录另一块 STM32 板（如
Blue Pill）。将 C6 SWD 排针接到目标：

C6 → 目标：

| C6  | 目标   |
|-----|--------|
| PA2 | SWDIO  |
| PA4 | SWCLK  |
| PA6 | NRST   |
| GND | GND    |

以 CMSIS-DAP 接口配置并重新构建：

```sh
cmake --preset Release \
  -DOPENOCD_INTERFACE=interface/cmsis-dap.cfg \
  -DOPENOCD_TRANSPORT=swd
cmake --build --preset Release
make -C build/Release check       # 验证目标连通性
make -C build/Release flash       # 烧录目标
```

若要烧录自定义 hex/bin（而非探针固件），直接使用 openocd：

```sh
openocd -f interface/cmsis-dap.cfg \
  -c "transport select swd" \
  -f target/stm32f1x.cfg \
  -c "adapter speed 10000" \
  -c "program /path/to/firmware.hex verify reset exit"
```

## 锁定 / 解锁（读保护）

```sh
make -C build/Release lock        # RDP level 1：flash 内容不可读
make -C build/Release unlock      # 清除 RDP：警告，会擦除整片 flash
```

解锁后芯片为空白，需重新烧录。

## 故障排查

**"Error: connect failed" / "SWD/JTAG-DP but no APs"**

- 检查接线：SWDIO、SWCLK、GND 必须连接。
- 确认目标已供电（3.3V）。
- 尝试更低适配器速度：`-DOPENOCD_SPEED=1000`。

**"Error: couldn't open device"**

- openocd 无法找到调试探针。检查 USB 连接。
- 运行 `lsusb`（Linux）或查看系统信息（macOS）确认探针已枚举。
- 对 ST-Link：确保没有其它 GDB/OpenOCD 会话占用探针。

**"Error: read protect is enabled"（RDP level 1）**

- 芯片已锁定。先解锁（会擦除 flash）：
  `make -C build/Release unlock`
- 再重新烧录。

**未找到 openocd**

- 安装：`brew install openocd`
- 或在配置时设置 `OPENOCD_EXECUTABLE`：
  `cmake -DOPENOCD_EXECUTABLE=/path/to/openocd ...`

**pyocd 与 openocd 对比**

之前的设置使用 pyocd（经 `/tmp/daptest` venv）烧录目标。OpenOCD 无需
Python venv 即可提供相同功能，是今后推荐路径。两者可共存。
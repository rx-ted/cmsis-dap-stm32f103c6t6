# 用 VS Code + Cortex-Debug 调试

本指南介绍如何使用微软 VS Code 配合 **Cortex-Debug** 扩展，以及项目
`gdbserver` make 目标启动的 GDB server，对 C6 探针（以及经它烧录的
目标板）进行图形化调试。跨平台（Windows / macOS / Linux）。

**调试请用 Debug 构建。** `Release` 以 `-g0` 编译（无调试信息）；
`Debug` 使用 `-O0 -g3`，符号、断点与源码单步才可用。`Release` 保留
用于烧录设备。

另见：[为什么用 VS Code 而非 STM32CubeIDE？](#vs-code-与-stm32cubeide-对比)

## 依赖

- [VS Code](https://code.visualstudio.com/)
- 扩展 **Cortex-Debug**（作者 marus25）
- `arm-none-eabi-gdb`（已就绪：`/opt/gcc/arm-none-eabi-gcc/bin/`）
- `openocd`（已安装：`/opt/homebrew/bin/openocd`）

## VS Code 与 STM32CubeIDE 对比

| | VS Code + Cortex-Debug | STM32CubeIDE（基于 Eclipse） |
|---|---|---|
| 许可 | 免费、开源扩展 | 免费、闭源 |
| 安装体积 | 小（按需装扩展） | 超过 1 GB 的完整 IDE |
| 配置驱动 | `.vscode/` 下的 JSON 文件 | GUI 向导 + 生成工程 |
| 跨平台 | Windows / macOS / Linux | Windows / macOS / Linux |
| 与本项目 CMake+OpenOCD | 原生契合 | 导入 CMake，重建费劲 |
| GDB server | 复用 `make gdbserver` | 自带 ST-Link GDB server |
| 脚本/CI 集成 | 文件式配置，易脚本化 | 更重 |

针对本项目基于 `CMakePresets.json` 的工作流，VS Code + Cortex-Debug
更轻量：它复用 `make gdbserver` 和同一套工具链，无需 GUI 导入步骤。

## 安装

1. 安装 VS Code，在扩展标签安装 `Cortex-Debug`。
2. 确保 `arm-none-eabi-gdb` 与 `openocd` 在 PATH 上。

## 工作区文件

Cortex-Debug 读取 `.vscode/launch.json`。创建目录与文件：

```jsonc
// .vscode/launch.json
{
  "version": "0.2.0",
  "configurations": [
    {
      "name": "C6 (OpenOCD gdbserver)",
      "type": "cortex-debug",
      "servertype": "openocd",
      "request": "launch",
      "cwd": "${workspaceFolder}",
      "executable": "${workspaceFolder}/build/Debug/nanoDAP-C6.elf",
      "device": "STM32F103C6T6",
      "configFiles": [
        "interface/stlink.cfg",
        "target/stm32f1x.cfg"
      ],
      "svdFile": "${workspaceFolder}/build/Debug/nanoDAP-C6.svd", // 可选
      "gdbPath": "arm-none-eabi-gdb",
      "preLaunchTask": "Build & start gdbserver"
    }
  ]
}
```

## 启动 GDB server

在终端中：

```sh
cmake --preset Debug
cmake --build --preset Debug
cd build/Debug && make gdbserver
```

`make gdbserver` 在前台于 3333 端口运行 OpenOCD。保持该终端打开，
Cortex-Debug 会连接它。

或者由上面的 `preLaunchTask` 为你启动 OpenOCD；见下文
[tasks 示例](#tasksjson)。

## 调试会话

1. 保持 GDB server 终端运行。
2. 在 VS Code 中按 `F5`（或运行 运行与调试 → **C6 (OpenOCD gdbserver)**）。
3. Cortex-Debug 附加目标、停机，并显示：
   - 调用栈、线程、寄存器视图
   - 内存浏览器
   - 外设视图（提供了 `.svd` 文件时）
   - 源码单步
4. 在 `src/main.c` / 中断处理函数中设置断点、单步、观察变量。

## 经 C6（CMSIS-DAP）调试目标板

将 OpenOCD 指向 CMSIS-DAP 接口而非 ST-Link。两种方式：

- 手动启动 server：

  ```sh
  openocd -f interface/cmsis-dap.cfg \
    -c "transport select swd" \
    -f target/stm32f1x.cfg \
    -c "adapter speed 10000"
  ```

  然后在 VS Code 中用 `"servertype": "external"` 的 launch 配置；或

- 配置一个直接以 CMSIS-DAP 配置启动 OpenOCD 的 launch 配置
  （同上，但 `configFiles` 设为 `interface/cmsis-dap.cfg`，
  `executable` 指向目标 ELF）。

## tasks.json

让 VS Code 调试前自动执行构建 + gdbserver：

```jsonc
// .vscode/tasks.json
{
  "version": "2.0.0",
  "tasks": [
    {
      "label": "Build",
      "type": "shell",
      "command": "cmake --build --preset Release",
      "problemMatcher": ["$gcc"]
    },
    {
      "label": "Start gdbserver",
      "type": "shell",
      "command": "cd build/Release && make gdbserver",
      "isBackground": true,
      "problemMatcher": []
    },
    {
      "label": "Build & start gdbserver",
      "dependsOn": ["Build", "Start gdbserver"]
    }
  ]
}
```

## 外设视图

提供 `.svd` 文件时，Cortex-Debug 可显示外设。ST 的 SVD 文件不随本
仓库分发；请下载 `STM32F103C6.svd`（或 F1 pack 中的 SVD）并在
launch.json 中设置 `svdFile`。

## 故障排查

- **Error: connection refused / server not running** — 先启动
  `make gdbserver`，或让 launch 配置自启 OpenOCD。
- **No symbol for current PC** — 使用了错误的 ELF。请用 **Debug**
  构建（`build/Debug/nanoDAP-C6.elf`）；`Release` 无调试信息（`-g0`）。
- **外设缺失** — 提供 `svdFile`。
- **无法连接设备** — 见 `doc/zh/flash.md` 的故障排查一节。
# CMSIS-DAP-C6

Standalone CMSIS-DAP v2 debug probe firmware for the STM32F103C6T6
"mini" board, derived from the STM32F103C8T6 CMSIS-DAP_SWO variant of
the nanoDAP project.

目前唯一的 CMSIS-DAP v2 调试探针固件，面向 STM32F103C6T6 "mini" 板，
源自 nanoDAP 项目的 STM32F103C8T6 CMSIS-DAP_SWO 变体。

## Languages / 语言

- [English](doc/en/README.md)
- [中文](doc/zh/README.md)

## Layout / 目录结构

- `doc/en/`   English documentation
- `doc/zh/`   中文文档
- `test/`      CDC loopback and SWD regression scripts
- `src/`       sources (`dap/`, `usb/`, `config/`, `cmsis/`)
- `startup/`   vector startup and linker script
- `cmake/`     arm-none-eabi toolchain, CubeMX-style source lists, OpenOCD targets
- `.vscode/`   (optional) VS Code Cortex-Debug launch/tasks config

## Quick build / 快速构建

```sh
cmake --preset Release
cmake --build --preset Release
```

See / 见 `doc/en/README.md` or `doc/zh/README.md` for details.
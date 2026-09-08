# Debugging with VS Code + Cortex-Debug

This guide covers graphical debugging of the C6 probe (and of a target
flashed through it) using Microsoft VS Code with the **Cortex-Debug**
extension and the GDB server started by the project's `gdbserver`
make target. It is cross-platform (Windows / macOS / Linux).

**Use the Debug build for debugging.** `Release` is built with `-g0` (no
debug info); `Debug` uses `-O0 -g3` so symbols, breakpoints and source
stepping work. Keep `Release` for programming the device.

See also: [Why VS Code instead of STM32CubeIDE?](#vs-code-vs-stm32cubeide)

## Requirements

- [VS Code](https://code.visualstudio.com/)
- Extension **Cortex-Debug** (by marus25)
- `arm-none-eabi-gdb` (already available: `/opt/gcc/arm-none-eabi-gcc/bin/`)
- `openocd` (installed: `/opt/homebrew/bin/openocd`)

## vs STM32CubeIDE

| | VS Code + Cortex-Debug | STM32CubeIDE (Eclipse-based) |
|---|---|---|
| License | free, open | free, closed |
| Install size | small (extensions on demand) | > 1 GB full IDE |
| Config driven | JSON files in `.vscode/` | GUI wizards + project generated |
| Cross-platform | Windows / macOS / Linux | Windows / macOS / Linux |
| Works with our CMake+OpenOCD | yes, native | import CMake, rebuild pain |
| GDB server | reuse `make gdbserver` | own ST-Link GDB server |
| Gradle/CI integration | file-based config, easy script | heavier |

For this project's existing `CMakePresets.json`-driven workflow, VS Code +
Cortex-Debug is the lighter fit: it reuses `make gdbserver` and the exact
same toolchain, no GUI import step.

## Install

1. Install VS Code, then open the Extensions tab and install `Cortex-Debug`.
2. Make sure `arm-none-eabi-gdb` and `openocd` are on PATH.

## Workspace files

Cortex-Debug reads a `.vscode/launch.json`. Create the directory and file:

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
      "svdFile": "${workspaceFolder}/build/Debug/nanoDAP-C6.svd", // optional
      "gdbPath": "arm-none-eabi-gdb",
      "preLaunchTask": "Build & start gdbserver"
    }
  ]
}
```

## Start the GDB server

In a terminal:

```sh
cmake --preset Debug
cmake --build --preset Debug
cd build/Debug && make gdbserver
```

`make gdbserver` runs OpenOCD in the foreground on port 3333. Keep that
terminal open. Cortex-Debug connects to it.

Alternatively the `preLaunchTask` above can start OpenOCD for you; see the
[tasks example](#tasksjson) below.

## Debug session

1. Keep the OpenOCD/GDB server terminal running.
2. In VS Code press `F5` (or run **Run and Debug** → **C6 (OpenOCD gdbserver)**).
3. Cortex-Debug attaches, halts the target, and shows:
   - Call stack, threads, register view
   - Memory browser
   - Peripheral view (if a `.svd` file is provided)
   - Source stepping
4. Set breakpoints in `src/main.c` / interrupt handlers, step, watch
   variables.

## Debugging a target board via C6 (CMSIS-DAP)

Point OpenOCD at the CMSIS-DAP interface instead of ST-Link. Either:

- Start the server manually with:

  ```sh
  openocd -f interface/cmsis-dap.cfg \
    -c "transport select swd" \
    -f target/stm32f1x.cfg \
    -c "adapter speed 10000"
  ```

  then use a VS Code launch config with `"servertype": "external"` — or

- Configure a launch config that starts OpenOCD with the CMSIS-DAP config
  directly (as above but with `configFiles` set to
  `interface/cmsis-dap.cfg` and `executable` pointing to the target ELF).

## tasks.json

To let VS Code run the build + gdbserver automatically before debug:

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

## Peripheral view

Cortex-Debug shows peripherals when a `.svd` file is given. ST's SVD files
are not bundled with this repo; download `STM32F103C6.svd` (or the F1 pack's
SVD) and set `svdFile` in launch.json.

## Troubleshooting

- **Error: connection refused / server not running** — start
  `make gdbserver` first, or update your launch config to start OpenOCD
  itself.
- **No symbol for current PC** — wrong ELF. Use the **Debug** build
  (`build/Debug/nanoDAP-C6.elf`); `Release` has no debug info (`-g0`).
- **Peripherals missing** — provide `svdFile`.
- **Can't connect to device** — see the troubleshooting section in
  `doc/en/flash.md`.
cmake_minimum_required(VERSION 3.16)
project(nanoDAP-C6 C ASM)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --specs=nano.specs")

# C6: 32 KB Flash / 10 KB SRAM, Cortex-M3, -Os and gc-sections to fit.
add_compile_options(
  -mcpu=cortex-m3
  -mthumb
  -Os
  -ffunction-sections
  -fdata-sections
  -fno-builtin
)

add_link_options(
  -mcpu=cortex-m3
  -mthumb
  -Wl,--gc-sections
  -Wl,--print-memory-usage
  -Wl,-Map=${CMAKE_CURRENT_BINARY_DIR}/nanoDAP-C6.map
  -nostartfiles
  -T${CMAKE_CURRENT_SOURCE_DIR}/startup/stm32f103c6_flash.ld
)

add_compile_definitions(
  BOARD_C6
  STM32F103xB
  __STM32F10x_H
  USE_STDPERIPH_DRIVER
)

set(CMSIS_SRC
  src/cmsis/system_stm32f1xx.c
)

set(DAP_SRC
  src/dap/DAP.c
  src/dap/SW_DP.c
)

set(USB_CORE_SRC
  src/usb/core/src/usb_core.c
  src/usb/core/src/usb_init.c
  src/usb/core/src/usb_int.c
  src/usb/core/src/usb_mem.c
  src/usb/core/src/usb_regs.c
  src/usb/core/src/usb_sil.c
)

set(USB_APP_SRC
  src/usb/hw_config.c
  src/usb/usb_desc.c
  src/usb/usb_endp.c
  src/usb/usb_istr.c
  src/usb/usb_prop.c
  src/usb/usb_pwr.c
)

add_executable(nanoDAP-C6
  src/main.c
  src/stm32_it.c
  startup/startup_stm32f103xb.s
  ${CMSIS_SRC}
  ${DAP_SRC}
  ${USB_CORE_SRC}
  ${USB_APP_SRC}
)

target_include_directories(nanoDAP-C6 PRIVATE
  src
  src/cmsis
  src/config
  src/dap
  src/usb
  src/usb/core/inc
)

add_custom_command(TARGET nanoDAP-C6 POST_BUILD
  COMMAND ${CMAKE_OBJCOPY} -O binary $<TARGET_FILE:nanoDAP-C6> ${CMAKE_CURRENT_BINARY_DIR}/nanoDAP-C6.bin
  COMMAND ${CMAKE_OBJCOPY} -O ihex   $<TARGET_FILE:nanoDAP-C6> ${CMAKE_CURRENT_BINARY_DIR}/nanoDAP-C6.hex
  COMMENT "Generate binary and hex firmware files"
)
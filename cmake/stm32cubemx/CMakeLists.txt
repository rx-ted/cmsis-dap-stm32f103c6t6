cmake_minimum_required(VERSION 3.22)
# Enable CMake support for ASM and C languages
enable_language(C ASM)
# nanoDAP-C6 build symbols (macros), mirrored from the LED/CubeMX layout
set(MX_Defines_Syms 
	BOARD_C6 
	STM32F103xB
	__STM32F10x_H
	USE_STDPERIPH_DRIVER
    $<$<CONFIG:Debug>:DEBUG>
)

# Include paths
set(MX_Include_Dirs
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src/cmsis
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src/config
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src/dap
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src/usb
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src/usb/core/inc
)

# Application sources (main + vectors/startup)
set(MX_Application_Src
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src/main.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src/stm32_it.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../startup/startup_stm32f103xb.s
)

# Library sources: CMSIS system init, CMSIS-DAP SWD core, USB FS stack,
# CDC/HID endpoint and descriptor glue
set(STM32_Drivers_Src
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src/cmsis/system_stm32f1xx.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src/dap/DAP.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src/dap/SW_DP.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src/dap/JTAG_DP.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src/usb/core/src/usb_core.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src/usb/core/src/usb_init.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src/usb/core/src/usb_int.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src/usb/core/src/usb_mem.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src/usb/core/src/usb_regs.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src/usb/core/src/usb_sil.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src/usb/hw_config.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src/usb/usb_desc.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src/usb/usb_endp.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src/usb/usb_istr.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src/usb/usb_prop.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src/usb/usb_pwr.c
)

# Link directories setup
set(MX_LINK_DIRS

)
# Project static libraries
set(MX_LINK_LIBS 
    STM32_Drivers
    ${TOOLCHAIN_LINK_LIBRARIES}
    
)
# Interface library for includes and symbols
add_library(stm32cubemx INTERFACE)
target_include_directories(stm32cubemx INTERFACE ${MX_Include_Dirs})
target_compile_definitions(stm32cubemx INTERFACE ${MX_Defines_Syms})

# Create STM32_Drivers static library
add_library(STM32_Drivers OBJECT)
target_sources(STM32_Drivers PRIVATE ${STM32_Drivers_Src})
target_link_libraries(STM32_Drivers PUBLIC stm32cubemx)


# Add application sources to the project
target_sources(${CMAKE_PROJECT_NAME} PRIVATE ${MX_Application_Src})

# Link directories setup
target_link_directories(${CMAKE_PROJECT_NAME} PRIVATE ${MX_LINK_DIRS})

# Add libraries to the project
target_link_libraries(${CMAKE_PROJECT_NAME} ${MX_LINK_LIBS})
# cmake/openocd.cmake — OpenOCD flash / erase / lock / unlock targets
#
# Cache variables (override at configure time):
#   OPENOCD_INTERFACE  openocd interface config file path
#   OPENOCD_TRANSPORT  transport (hla_swd for ST-Link, swd for CMSIS-DAP)
#   OPENOCD_TARGET     openocd target config file path
#   OPENOCD_SPEED      SWD adapter clock in kHz (default 10000)
#
# Default: ST-Link V2 (local C6 board).  For target via CMSIS-DAP:
#   cmake -DOPENOCD_INTERFACE=interface/cmsis-dap.cfg -DOPENOCD_TRANSPORT=swd ..

find_program(OPENOCD_EXECUTABLE openocd)
if(NOT OPENOCD_EXECUTABLE)
    message(WARNING "openocd not found — flash/check/erase/lock/unlock targets will be unavailable. Install with: brew install openocd")
    return()
endif()

set(OPENOCD_INTERFACE "interface/stlink.cfg" CACHE STRING "OpenOCD interface config")
set(OPENOCD_TRANSPORT "hla_swd"              CACHE STRING "OpenOCD transport")
set(OPENOCD_TARGET    "target/stm32f1x.cfg"  CACHE STRING "OpenOCD target config")
set(OPENOCD_SPEED     10000                  CACHE STRING "SWD adapter clock (kHz)")

# DAP software reset only (reset_config none). This avoids any dependency on
# the ST-Link NRST wire: the C6 SWD header exposes SWDIO/SWCLK/GND/3V3, and
# NRST (PA6) is optional. Set OPENOCD_RESET_CONFIG=srst_only if you wire
# the ST-Link NRST pin and prefer a hardware reset.
set(OPENOCD_RESET_CONFIG "none" CACHE STRING "reset_config (none or srst_only)")

set(_OC_COMMON
    -f ${OPENOCD_INTERFACE}
    -c "transport select ${OPENOCD_TRANSPORT}"
    -f ${OPENOCD_TARGET}
    -c "adapter speed ${OPENOCD_SPEED}"
    -c "reset_config ${OPENOCD_RESET_CONFIG}"
)

# ---- check: read IDCODE, verify connectivity ----
add_custom_target(check
    COMMAND ${OPENOCD_EXECUTABLE} ${_OC_COMMON}
        -c "init"
        -c "targets"
        -c "shutdown"
    COMMENT "OpenOCD: check connectivity (read IDCODE)"
    VERBATIM
)

# ---- flash: program + verify + reset ----
add_custom_target(flash
    COMMAND ${OPENOCD_EXECUTABLE} ${_OC_COMMON}
        -c "program ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_PROJECT_NAME}.hex verify reset exit"
    COMMENT "OpenOCD: flash ${CMAKE_PROJECT_NAME}.hex + verify + reset"
    DEPENDS ${CMAKE_PROJECT_NAME}.elf
    VERBATIM
)

# ---- erase: full chip erase ----
add_custom_target(erase
    COMMAND ${OPENOCD_EXECUTABLE} ${_OC_COMMON}
        -c "init"
        -c "halt"
        -c "flash erase_sector 0 0 last"
        -c "reset halt"
        -c "shutdown"
    COMMENT "OpenOCD: erase all flash sectors"
    VERBATIM
)

# ---- lock: read-protect (RDP level 1) ----
add_custom_target(lock
    COMMAND ${OPENOCD_EXECUTABLE} ${_OC_COMMON}
        -c "init"
        -c "halt"
        -c "stm32f1x lock 0"
        -c "shutdown"
    COMMENT "OpenOCD: enable read-protection (RDP level 1)"
    VERBATIM
)

# ---- unlock: clear read-protection (ERASES ENTIRE CHIP) ----
add_custom_target(unlock
    COMMAND ${OPENOCD_EXECUTABLE} ${_OC_COMMON}
        -c "init"
        -c "halt"
        -c "stm32f1x unlock 0"
        -c "shutdown"
    COMMENT "OpenOCD: clear read-protection (WARNING: erases flash)"
    VERBATIM
)

# ---- reset: software reset ----
add_custom_target(reset
    COMMAND ${OPENOCD_EXECUTABLE} ${_OC_COMMON}
        -c "init"
        -c "reset halt"
        -c "shutdown"
    COMMENT "OpenOCD: reset halt"
    VERBATIM
)

# ---- gdbserver: start GDB server (foreground, blocking) ----
add_custom_target(gdbserver
    COMMAND ${OPENOCD_EXECUTABLE} ${_OC_COMMON}
        -c "gdb_port 3333"
        -c "tcl_port 6666"
        -c "tpiu port 6667"
        -c "init"
    COMMENT "OpenOCD: GDB server on :3333 (Ctrl-C to stop)"
    VERBATIM
)
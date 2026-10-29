# ─── openocd.cmake — Flash and debug targets via OpenOCD ─────────────────────

#
# add_flash_target(TARGET_NAME BOARD_NAME)
#
# Creates a custom target `flash_<TARGET_NAME>` that programs the firmware.
#
function(add_flash_target TARGET_NAME BOARD_NAME)
    set(OPENOCD_CFG "${CMAKE_SOURCE_DIR}/boards/${BOARD_NAME}/openocd.cfg")

    add_custom_target(flash_${TARGET_NAME}
        COMMAND openocd
            -f "${OPENOCD_CFG}"
            -c "program $<TARGET_FILE:${TARGET_NAME}> verify reset exit"
        DEPENDS ${TARGET_NAME}
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        COMMENT "Flashing ${TARGET_NAME} to ${BOARD_NAME}..."
        USES_TERMINAL
    )
endfunction()

#
# add_debug_target(TARGET_NAME BOARD_NAME)
#
# Creates a custom target `debug_<TARGET_NAME>` that starts an OpenOCD GDB server.
#
function(add_debug_target TARGET_NAME BOARD_NAME)
    set(OPENOCD_CFG "${CMAKE_SOURCE_DIR}/boards/${BOARD_NAME}/openocd.cfg")

    add_custom_target(debug_${TARGET_NAME}
        COMMAND openocd -f "${OPENOCD_CFG}"
        DEPENDS ${TARGET_NAME}
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        COMMENT "Starting OpenOCD for ${TARGET_NAME} (connect GDB to :3333)..."
        USES_TERMINAL
    )
endfunction()

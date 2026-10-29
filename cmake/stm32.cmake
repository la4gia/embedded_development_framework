# ─── stm32.cmake — Helper functions for firmware targets ─────────────────────

#
# add_firmware(NAME <name> BOARD <board> SOURCES <src...> [DRIVERS <drv...>])
#
# Creates an executable target and generates .bin, .hex, and size report.
#
function(add_firmware)
    cmake_parse_arguments(FW "" "NAME;BOARD" "SOURCES;DRIVERS" ${ARGN})

    if(NOT FW_NAME)
        message(FATAL_ERROR "add_firmware: NAME is required")
    endif()
    if(NOT FW_BOARD)
        message(FATAL_ERROR "add_firmware: BOARD is required")
    endif()
    if(NOT FW_SOURCES)
        message(FATAL_ERROR "add_firmware: SOURCES is required")
    endif()

    # Convert board name to CMake target (hyphens → underscores)
    string(REPLACE "-" "_" BOARD_TARGET "board_${FW_BOARD}")

    # Create executable
    add_executable(${FW_NAME} ${FW_SOURCES})

    # Link board support (provides startup, system, linker script, includes)
    target_link_libraries(${FW_NAME} PRIVATE ${BOARD_TARGET})

    # Link requested drivers
    if(FW_DRIVERS)
        target_link_libraries(${FW_NAME} PRIVATE ${FW_DRIVERS})
    endif()

    # Link common library
    target_link_libraries(${FW_NAME} PRIVATE lib_common)

    # Generate .bin
    add_custom_command(TARGET ${FW_NAME} POST_BUILD
        COMMAND ${CMAKE_OBJCOPY} -O binary $<TARGET_FILE:${FW_NAME}>
                ${CMAKE_CURRENT_BINARY_DIR}/${FW_NAME}.bin
        COMMENT "Generating ${FW_NAME}.bin"
    )

    # Generate .hex
    add_custom_command(TARGET ${FW_NAME} POST_BUILD
        COMMAND ${CMAKE_OBJCOPY} -O ihex $<TARGET_FILE:${FW_NAME}>
                ${CMAKE_CURRENT_BINARY_DIR}/${FW_NAME}.hex
        COMMENT "Generating ${FW_NAME}.hex"
    )

    # Print size
    add_custom_command(TARGET ${FW_NAME} POST_BUILD
        COMMAND ${CMAKE_SIZE} $<TARGET_FILE:${FW_NAME}>
        COMMENT "Firmware size:"
    )

    # Generate map file
    target_link_options(${FW_NAME} PRIVATE
        "LINKER:-Map=${CMAKE_CURRENT_BINARY_DIR}/${FW_NAME}.map"
    )
endfunction()

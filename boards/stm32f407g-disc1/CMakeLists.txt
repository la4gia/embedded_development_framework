# ─── STM32F407G-DISC1 Board Support Package ──────────────────────────────────

set(BOARD_NAME stm32f407g_disc1)

# External dependency paths
set(CMSIS_CORE_DIR    ${CMAKE_SOURCE_DIR}/external/cmsis-core)
set(CMSIS_DEVICE_DIR  ${CMAKE_SOURCE_DIR}/external/cmsis-device-f4)
set(STARTUP_FILE      ${CMSIS_DEVICE_DIR}/Source/Templates/gcc/startup_stm32f407xx.s)
set(LINKER_SCRIPT     ${CMAKE_CURRENT_SOURCE_DIR}/stm32f407vg.ld)

# Board support object library (startup + system init)
add_library(${BOARD_NAME}_obj OBJECT
    ${STARTUP_FILE}
    ${CMAKE_CURRENT_SOURCE_DIR}/src/system.c
)

target_include_directories(${BOARD_NAME}_obj PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMSIS_CORE_DIR}
    ${CMSIS_DEVICE_DIR}/Include
)

target_compile_definitions(${BOARD_NAME}_obj PRIVATE
    STM32F407xx
)

target_compile_options(${BOARD_NAME}_obj PRIVATE
    -mcpu=cortex-m4
    -mfpu=fpv4-sp-d16
    -mfloat-abi=hard
)

# INTERFACE library for consumers (apps)
add_library(board_${BOARD_NAME} INTERFACE)

target_include_directories(board_${BOARD_NAME} INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMSIS_CORE_DIR}
    ${CMSIS_DEVICE_DIR}/Include
)

target_compile_definitions(board_${BOARD_NAME} INTERFACE
    STM32F407xx
)

target_compile_options(board_${BOARD_NAME} INTERFACE
    -mcpu=cortex-m4
    -mfpu=fpv4-sp-d16
    -mfloat-abi=hard
)

target_link_options(board_${BOARD_NAME} INTERFACE
    -mcpu=cortex-m4
    -mfpu=fpv4-sp-d16
    -mfloat-abi=hard
    -T${LINKER_SCRIPT}
)

target_link_libraries(board_${BOARD_NAME} INTERFACE
    $<TARGET_OBJECTS:${BOARD_NAME}_obj>
)

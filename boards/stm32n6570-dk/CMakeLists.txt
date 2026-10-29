# ─── STM32N6570-DK Board Support Package ─────────────────────────────────────

set(BOARD_NAME stm32n6570_dk)

set(CMSIS_CORE_DIR   ${CMAKE_SOURCE_DIR}/external/cmsis-core)
set(CMSIS_DEVICE_DIR ${CMAKE_SOURCE_DIR}/external/cmsis-device-n6)
set(STARTUP_FILE     ${CMSIS_DEVICE_DIR}/Source/Templates/gcc/startup_stm32n657xx.s)
set(LINKER_SCRIPT    ${CMAKE_CURRENT_SOURCE_DIR}/STM32N657XX_LRUN.ld)

# Board support object library
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
    STM32N657xx
    CPU_IN_SECURE_STATE
)

target_compile_options(${BOARD_NAME}_obj PRIVATE
    -mcpu=cortex-m55
    -mfloat-abi=hard
    -mfpu=fpv5-d16
)

# INTERFACE library for consumers
add_library(board_${BOARD_NAME} INTERFACE)

target_include_directories(board_${BOARD_NAME} INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMSIS_CORE_DIR}
    ${CMSIS_DEVICE_DIR}/Include
)

target_compile_definitions(board_${BOARD_NAME} INTERFACE
    STM32N657xx
    CPU_IN_SECURE_STATE
)

target_compile_options(board_${BOARD_NAME} INTERFACE
    -mcpu=cortex-m55
    -mfloat-abi=hard
    -mfpu=fpv5-d16
)

target_link_options(board_${BOARD_NAME} INTERFACE
    -mcpu=cortex-m55
    -mfloat-abi=hard
    -mfpu=fpv5-d16
    -T${LINKER_SCRIPT}
)

target_link_libraries(board_${BOARD_NAME} INTERFACE
    $<TARGET_OBJECTS:${BOARD_NAME}_obj>
)

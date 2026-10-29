# ─── NUCLEO-C562RE Board Support Package ─────────────────────────────────────
#
# NOTE: CMSIS device headers for STM32C5 are not yet publicly available.
# This BSP is a placeholder. Once the cmsis-device-c5 pack is released:
#   1. Update scripts/fetch-deps.sh to clone it
#   2. Update paths below to point to external/cmsis-device-c5/
#   3. Add the correct startup .s file
#

set(BOARD_NAME nucleo_c562re)

set(CMSIS_CORE_DIR   ${CMAKE_SOURCE_DIR}/external/cmsis-core)
set(LINKER_SCRIPT    ${CMAKE_CURRENT_SOURCE_DIR}/stm32c562re.ld)

# TODO: Uncomment when CMSIS device headers are available
# set(CMSIS_DEVICE_DIR ${CMAKE_SOURCE_DIR}/external/cmsis-device-c5)
# set(STARTUP_FILE     ${CMSIS_DEVICE_DIR}/Source/Templates/gcc/startup_stm32c562xx.s)

# Board support object library
add_library(${BOARD_NAME}_obj OBJECT
    # ${STARTUP_FILE}
    ${CMAKE_CURRENT_SOURCE_DIR}/src/system.c
)

target_include_directories(${BOARD_NAME}_obj PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMSIS_CORE_DIR}
    # ${CMSIS_DEVICE_DIR}/Include
)

target_compile_definitions(${BOARD_NAME}_obj PRIVATE
    STM32C562xx
)

target_compile_options(${BOARD_NAME}_obj PRIVATE
    -mcpu=cortex-m33
    -mfpu=fpv5-sp-d16
    -mfloat-abi=hard
)

# INTERFACE library for consumers
add_library(board_${BOARD_NAME} INTERFACE)

target_include_directories(board_${BOARD_NAME} INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMSIS_CORE_DIR}
    # ${CMSIS_DEVICE_DIR}/Include
)

target_compile_definitions(board_${BOARD_NAME} INTERFACE
    STM32C562xx
)

target_compile_options(board_${BOARD_NAME} INTERFACE
    -mcpu=cortex-m33
    -mfpu=fpv5-sp-d16
    -mfloat-abi=hard
)

target_link_options(board_${BOARD_NAME} INTERFACE
    -mcpu=cortex-m33
    -mfpu=fpv5-sp-d16
    -mfloat-abi=hard
    -T${LINKER_SCRIPT}
)

target_link_libraries(board_${BOARD_NAME} INTERFACE
    $<TARGET_OBJECTS:${BOARD_NAME}_obj>
)

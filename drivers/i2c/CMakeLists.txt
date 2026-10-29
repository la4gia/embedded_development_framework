string(REPLACE "-" "_" BOARD_TARGET "board_${BOARD}")

# I2C register layout differs by family (unlike GPIO), so pick the source
# file that matches the current BOARD's peripheral register map.
if(BOARD STREQUAL "stm32f407g-disc1")
    # No F4 I2C implementation yet — skip the driver for this board.
    return()
elseif(BOARD STREQUAL "stm32n6570-dk")
    set(I2C_SRC src/i2c_n6.c)
elseif(BOARD STREQUAL "nucleo-c562re")
    set(I2C_SRC src/i2c_c5.c)
else()
    message(FATAL_ERROR "driver_i2c: no I2C register layout mapped for BOARD '${BOARD}'")
endif()

add_library(driver_i2c STATIC
    ${I2C_SRC}
)

target_include_directories(driver_i2c PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(driver_i2c PUBLIC ${BOARD_TARGET})

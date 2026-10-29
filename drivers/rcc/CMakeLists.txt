string(REPLACE "-" "_" BOARD_TARGET "board_${BOARD}")

# RCC register layout differs by family (more so than GPIO), so pick the
# source file that matches the current BOARD's peripheral register map.
if(BOARD STREQUAL "stm32f407g-disc1")
    set(RCC_SRC src/rcc_f4.c)
elseif(BOARD STREQUAL "stm32n6570-dk")
    set(RCC_SRC src/rcc_n6.c)
elseif(BOARD STREQUAL "nucleo-c562re")
    set(I2C_SRC src/rcc_c5.c)
else()
    message(FATAL_ERROR "driver_rcc: no RCC register layout mapped for BOARD '${BOARD}'")
endif()

add_library(driver_rcc STATIC
    ${RCC_SRC}
)

target_include_directories(driver_rcc PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(driver_rcc PUBLIC ${BOARD_TARGET})

# UART register layout differs by family (unlike GPIO), so pick the source
# file that matches the current BOARD's peripheral register map.
if(BOARD STREQUAL "stm32f407g-disc1")
    set(UART_SRC src/uart_f4.c)
elseif(BOARD STREQUAL "stm32n6570-dk")
    set(UART_SRC src/uart_n6.c)
elseif(BOARD STREQUAL "nucleo-c562re")
    set(UART_SRC src/uart_c5.c)
else()
    message(FATAL_ERROR "driver_uart: no UART register layout mapped for BOARD '${BOARD}'")
endif()

add_library(driver_uart STATIC
    ${UART_SRC}
)

target_include_directories(driver_uart PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

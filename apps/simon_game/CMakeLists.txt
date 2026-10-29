add_firmware(
    NAME simon_game
    BOARD ${BOARD}
    SOURCES src/main.c
    DRIVERS driver_gpio driver_rcc driver_exti driver_uart
)

add_flash_target(simon_game ${BOARD})
add_debug_target(simon_game ${BOARD})

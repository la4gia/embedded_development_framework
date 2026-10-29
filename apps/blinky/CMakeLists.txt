add_firmware(
    NAME blinky
    BOARD ${BOARD}
    SOURCES src/main.c
    DRIVERS driver_gpio driver_rcc
)

add_flash_target(blinky ${BOARD})
add_debug_target(blinky ${BOARD})

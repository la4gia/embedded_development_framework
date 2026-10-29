add_firmware(
    NAME traffic_light
    BOARD ${BOARD}
    SOURCES src/main.c
    DRIVERS driver_gpio driver_rcc
)

add_flash_target(traffic_light ${BOARD})
add_debug_target(traffic_light ${BOARD})

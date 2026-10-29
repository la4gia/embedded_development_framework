add_firmware(
    NAME i2c_camera_test
    BOARD ${BOARD}
    SOURCES src/main.c
    DRIVERS driver_gpio driver_rcc driver_i2c
)
add_library(driver_gpio STATIC
    src/gpio.c
)

target_include_directories(driver_gpio PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

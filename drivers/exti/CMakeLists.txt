string(REPLACE "-" "_" BOARD_TARGET "board_${BOARD}")

add_library(driver_exti STATIC
    src/exti.c
)

target_include_directories(driver_exti PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(driver_exti PUBLIC ${BOARD_TARGET})

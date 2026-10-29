# Only build the selected board
if(IS_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/${BOARD}")
    add_subdirectory(${BOARD})
else()
    message(FATAL_ERROR "Board directory not found: ${CMAKE_CURRENT_SOURCE_DIR}/${BOARD}")
endif()

add_library(lib_common INTERFACE)

target_include_directories(lib_common INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

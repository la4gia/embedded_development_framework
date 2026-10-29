cmake_minimum_required(VERSION 3.20)

# ─── Validate board selection ─────────────────────────────────────────────────
if(NOT DEFINED BOARD)
    message(FATAL_ERROR
        "BOARD not specified. Use: cmake -B build/<board> -DBOARD=<board> -G Ninja\n"
        "Available boards:\n"
        "  stm32f407g-disc1\n"
        "  nucleo-c562re\n"
        "  stm32n6570-dk"
    )
endif()

if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/boards/${BOARD}/CMakeLists.txt")
    message(FATAL_ERROR "Board '${BOARD}' not found in boards/ directory")
endif()

# ─── Default build type ───────────────────────────────────────────────────────
# Without this, CMAKE_BUILD_TYPE is blank and none of the per-config C flags
# (including -g) get applied, so binaries build with zero debug info.
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type" FORCE)
endif()

# ─── Project ──────────────────────────────────────────────────────────────────
project(embedded-development LANGUAGES C ASM)

# ─── Global settings ──────────────────────────────────────────────────────────
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# ─── Board-specific CPU/FPU flags (applied globally for ABI consistency) ───────
if(BOARD STREQUAL "stm32f407g-disc1")
    add_compile_options(-mcpu=cortex-m4 -mfpu=fpv4-sp-d16 -mfloat-abi=hard)
    add_link_options(-mcpu=cortex-m4 -mfpu=fpv4-sp-d16 -mfloat-abi=hard)
elseif(BOARD STREQUAL "stm32n6570-dk")
    add_compile_options(-mcpu=cortex-m55 -mfpu=fpv5-sp-d16 -mfloat-abi=hard)
    add_link_options(-mcpu=cortex-m55 -mfpu=fpv5-sp-d16 -mfloat-abi=hard)
elseif(BOARD STREQUAL "nucleo-c562re")
    add_compile_options(-mcpu=cortex-m33 -mfpu=fpv5-sp-d16 -mfloat-abi=hard)
    add_link_options(-mcpu=cortex-m33 -mfpu=fpv5-sp-d16 -mfloat-abi=hard)
endif()

# ─── Include helper modules ───────────────────────────────────────────────────
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/stm32.cmake)
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/openocd.cmake)

# ─── Subdirectories ───────────────────────────────────────────────────────────
add_subdirectory(boards)
add_subdirectory(drivers)
add_subdirectory(lib)
add_subdirectory(apps)

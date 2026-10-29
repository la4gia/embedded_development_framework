# ─── ARM GCC Bare-Metal Toolchain File ────────────────────────────────────────
#
# Usage: cmake -B build/<board> -DBOARD=<board> \
#              -DCMAKE_TOOLCHAIN_FILE=cmake/toolchains/arm-none-eabi.cmake -G Ninja
#

set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR arm)

# ─── Toolchain binaries ───────────────────────────────────────────────────────
set(CMAKE_C_COMPILER   arm-none-eabi-gcc)
set(CMAKE_ASM_COMPILER arm-none-eabi-gcc)
set(CMAKE_OBJCOPY      arm-none-eabi-objcopy)
set(CMAKE_OBJDUMP      arm-none-eabi-objdump)
set(CMAKE_SIZE         arm-none-eabi-size)
set(CMAKE_AR           arm-none-eabi-ar)
set(CMAKE_RANLIB       arm-none-eabi-ranlib)

# ─── Compiler flags ──────────────────────────────────────────────────────────
set(CMAKE_C_FLAGS_INIT
    "-mthumb -Wall -Wextra -Werror=return-type -fno-common -ffunction-sections -fdata-sections -fno-builtin"
)
set(CMAKE_ASM_FLAGS_INIT
    "-mthumb -x assembler-with-cpp"
)

# ─── Linker flags ────────────────────────────────────────────────────────────
set(CMAKE_EXE_LINKER_FLAGS_INIT
    "--specs=nosys.specs --specs=nano.specs -Wl,--gc-sections -Wl,--print-memory-usage"
)

# ─── Build type defaults ─────────────────────────────────────────────────────
set(CMAKE_C_FLAGS_DEBUG   "-Og -g3 -gdwarf-4" CACHE STRING "")
set(CMAKE_C_FLAGS_RELEASE "-O2 -DNDEBUG" CACHE STRING "")
set(CMAKE_C_FLAGS_MINSIZEREL "-Os -DNDEBUG" CACHE STRING "")

# ─── Cross-compilation search paths ─────────────────────────────────────────
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

# ─── Skip compiler test (can't run ARM binary on host) ───────────────────────
set(CMAKE_C_COMPILER_WORKS TRUE)
set(CMAKE_ASM_COMPILER_WORKS TRUE)
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

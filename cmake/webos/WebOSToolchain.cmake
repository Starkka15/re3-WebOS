# WebOS TouchPad CMake Toolchain File
# For cross-compiling re3 to ARM using HP webOS PDK

# Skip compiler tests (we're cross-compiling with known working toolchain)
set(CMAKE_C_COMPILER_WORKS 1)
set(CMAKE_CXX_COMPILER_WORKS 1)
set(CMAKE_C_ABI_COMPILED 1)
set(CMAKE_CXX_ABI_COMPILED 1)

# Target system information
set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR arm)
set(CMAKE_SYSTEM_VERSION 1)

# WebOS PDK root path (WSL paths - /mnt/c/ prefix)
set(WEBOS_PDK_ROOT "/mnt/c/RE3 to WebOS Project Files/HP webOS/PDK")

# Compiler paths (Linaro 4.8 ARM Toolchain - GCC 4.8 for GLIBC 2.17 compatibility)
# Using Linaro 4.8 (2015) toolchain for webOS compatibility
set(LINARO_PATH "$ENV{HOME}/toolchains/gcc-linaro-4.8-2015.06-x86_64_arm-linux-gnueabi")
set(CMAKE_C_COMPILER "${LINARO_PATH}/bin/arm-linux-gnueabi-gcc")
set(CMAKE_CXX_COMPILER "${LINARO_PATH}/bin/arm-linux-gnueabi-g++")
set(CMAKE_AR "${LINARO_PATH}/bin/arm-linux-gnueabi-ar" CACHE FILEPATH "Archiver")
set(CMAKE_RANLIB "${LINARO_PATH}/bin/arm-linux-gnueabi-ranlib" CACHE FILEPATH "Ranlib")
set(CMAKE_STRIP "${LINARO_PATH}/bin/arm-linux-gnueabi-strip" CACHE FILEPATH "Strip")

# Compiler flags for ARM Cortex-A8 (TouchPad processor)
# Using softfp for compatibility with PDK libraries
# Optimization flags inspired by PS Vita port (Cortex-A9) adapted for Cortex-A8
set(ARM_CPU_FLAGS "-mcpu=cortex-a8 -mfpu=neon -mfloat-abi=softfp -marm")
# Removed -ffast-math as it can cause runtime issues
# Removed -ftree-vectorize as it may cause issues with this GCC version
set(ARM_OPT_FLAGS "")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${ARM_CPU_FLAGS} ${ARM_OPT_FLAGS}" CACHE STRING "C flags")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${ARM_CPU_FLAGS} ${ARM_OPT_FLAGS}" CACHE STRING "CXX flags")

# Additional linking flags for embedded target
# Static link libstdc++ and libgcc for compatibility with webOS GLIBC 2.11
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,--allow-shlib-undefined -pthread -static-libstdc++ -static-libgcc" CACHE STRING "Linker flags")

# Threading configuration for cross-compilation
set(CMAKE_THREAD_LIBS_INIT "-pthread")
set(CMAKE_HAVE_THREADS_LIBRARY 1)
set(CMAKE_USE_WIN32_THREADS_INIT 0)
set(CMAKE_USE_PTHREADS_INIT 1)
set(THREADS_PREFER_PTHREAD_FLAG ON)

# Search paths for libraries and includes
set(CMAKE_FIND_ROOT_PATH "${WEBOS_PDK_ROOT}/device")
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

# Include directories
include_directories(SYSTEM
    "${WEBOS_PDK_ROOT}/include"
    "${WEBOS_PDK_ROOT}/include/SDL"
)

# OpenAL configuration for webOS PDK
set(OPENAL_INCLUDE_DIR "${WEBOS_PDK_ROOT}/include" CACHE PATH "OpenAL include directory")
set(OPENAL_LIBRARY "${WEBOS_PDK_ROOT}/device/lib/libopenal.so" CACHE FILEPATH "OpenAL library")

# Library directories
link_directories(
    "${WEBOS_PDK_ROOT}/device/lib"
    "${WEBOS_PDK_ROOT}/device/usr/lib"
)

# WebOS-specific libraries
set(WEBOS_LIBRARIES SDL GLESv2 EGL openal pdl pthread)

# Platform-specific definitions
add_definitions(-DWEBOS_TOUCHPAD)
add_definitions(-DGTA_HANDHELD)
add_definitions(-D__ARM_NEON__)
add_definitions(-DFINAL)  # Disable debug menu to avoid font crashes

message(STATUS "WebOS PDK toolchain configured")
message(STATUS "  PDK Root: ${WEBOS_PDK_ROOT}")
message(STATUS "  C Compiler: ${CMAKE_C_COMPILER}")
message(STATUS "  CXX Compiler: ${CMAKE_CXX_COMPILER}")

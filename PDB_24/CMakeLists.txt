cmake_minimum_required(VERSION 3.22)

#
# This file is generated only once,
# and is not re-generated if converter is called multiple times.
#
# User is free to modify the file as much as necessary
#

# Setup compiler settings
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)


# Define the build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug")
endif()

# Set the project name
set(CMAKE_PROJECT_NAME PDB_24V)

# Enable compile command to ease indexing with e.g. clangd
set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

set(FIRMFLAKE_ROOT   ${CMAKE_SOURCE_DIR}/..)
set(COMMON_LIBS_PATH ${FIRMFLAKE_ROOT}/common_libraries)
set(PLATFORM_PATH    ${FIRMFLAKE_ROOT}/platform/stm32g4xx)
set(HAL_DRIVERS_PATH ${PLATFORM_PATH}/HAL_Drivers)

# Core project settings
project(${CMAKE_PROJECT_NAME})
message("Build type: " ${CMAKE_BUILD_TYPE})

# Enable CMake support for ASM and C languages
enable_language(C CXX ASM)

# Create an executable object type
add_executable(${CMAKE_PROJECT_NAME})

# Add STM32CubeMX generated sources
add_subdirectory(cmake/stm32cubemx)

# Link directories setup
target_link_directories(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined library search paths
)

# Add sources to executable
target_sources(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user sources here
    Core/Src/app/app_main.cpp
    # --- Common libraries (platform agnostic) ---
    ${COMMON_LIBS_PATH}/usr_leds/usr_led.cpp
    ${COMMON_LIBS_PATH}/clock/stopwatch.cpp
    ${COMMON_LIBS_PATH}/clock/timer.cpp
    ${COMMON_LIBS_PATH}/comms/comms.c
    ${PLATFORM_PATH}/Modules/can/can_stm32g4xx.cpp

    # --- Platform modules (STM32G4xx implementations of the common interfaces) ---
    ${PLATFORM_PATH}/Modules/clock/clock_stm32g4xx.cpp
    ${PLATFORM_PATH}/Modules/primitive_io/primitive_io_stm32g4xx.cpp
)

# Add include paths
target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE
    ${COMMON_LIBS_PATH}/clock
    ${COMMON_LIBS_PATH}/can
    ${COMMON_LIBS_PATH}/comms
    ${COMMON_LIBS_PATH}/primitive_io
    ${COMMON_LIBS_PATH}/servo
    ${COMMON_LIBS_PATH}/usr_leds
    ${COMMON_LIBS_PATH}/sensors
    ${PLATFORM_PATH}/Modules/can
    ${PLATFORM_PATH}/Modules/clock
    ${PLATFORM_PATH}/Modules/primitive_io
    ${PLATFORM_PATH}/Modules/servo
)

# Add project symbols (macros)
target_compile_definitions(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined symbols
)

# Remove wrong libob.a library dependency when using cpp files
list(REMOVE_ITEM CMAKE_C_IMPLICIT_LINK_LIBRARIES ob)

# Add linked libraries
target_link_libraries(${CMAKE_PROJECT_NAME}
    stm32cubemx

    # Add user defined libraries
)

# Toolchain cmake file
set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR arm)
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

# Use whatever arm-none-eabi compilers are on path
# (Ubuntu 24: `sudo apt install gcc-arm-none-eabi` gives GCC 13, which supports C++20)
set(CMAKE_C_COMPILER arm-none-eabi-gcc)
set(CMAKE_CXX_COMPILER arm-none-eabi-g++)
set(CMAKE_ASM_COMPILER arm-none-eabi-gcc)
set(CMAKE_OBJCOPY arm-none-eabi-objcopy)
set(CMAKE_SIZE arm-none-eabi-size)

# To pin a specific version instead (e.g. on Ubuntu 22, whose apt GCC is too old for C++20),
# download and extract from https://developer.arm.com/downloads/-/arm-gnu-toolchain-downloads
# and replace the block above with:
# set(TOOLCHAIN_DIR /opt/arm-gnu-toolchain-15.2.rel1-x86_64-arm-none-eabi/bin)
# set(CMAKE_C_COMPILER ${TOOLCHAIN_DIR}/arm-none-eabi-gcc)
# set(CMAKE_CXX_COMPILER ${TOOLCHAIN_DIR}/arm-none-eabi-g++)
# set(CMAKE_ASM_COMPILER ${TOOLCHAIN_DIR}/arm-none-eabi-gcc)
# set(CMAKE_OBJCOPY ${TOOLCHAIN_DIR}/arm-none-eabi-objcopy)
# set(CMAKE_SIZE ${TOOLCHAIN_DIR}/arm-none-eabi-size)

# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "/home/rowan/zephyrproject/modules/hal/espressif/components/bootloader/subproject"
  "/home/rowan/FirmFlake/samples-and-testing/echo_bot/build/esp-idf/build/bootloader"
  "/home/rowan/FirmFlake/samples-and-testing/echo_bot/build/esp-idf"
  "/home/rowan/FirmFlake/samples-and-testing/echo_bot/build/esp-idf/tmp"
  "/home/rowan/FirmFlake/samples-and-testing/echo_bot/build/esp-idf/src/EspIdfBootloader-stamp"
  "/home/rowan/FirmFlake/samples-and-testing/echo_bot/build/esp-idf/src"
  "/home/rowan/FirmFlake/samples-and-testing/echo_bot/build/esp-idf/src/EspIdfBootloader-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "/home/rowan/FirmFlake/samples-and-testing/echo_bot/build/esp-idf/src/EspIdfBootloader-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "/home/rowan/FirmFlake/samples-and-testing/echo_bot/build/esp-idf/src/EspIdfBootloader-stamp${cfgdir}") # cfgdir has leading slash
endif()

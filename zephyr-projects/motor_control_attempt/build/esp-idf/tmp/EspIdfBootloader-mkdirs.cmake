# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "/home/rog/zephyrproject/modules/hal/espressif/components/bootloader/subproject"
  "/home/rog/zephyrproject/zephyr/tutorials/motor_control_prac/build/esp-idf/build/bootloader"
  "/home/rog/zephyrproject/zephyr/tutorials/motor_control_prac/build/esp-idf"
  "/home/rog/zephyrproject/zephyr/tutorials/motor_control_prac/build/esp-idf/tmp"
  "/home/rog/zephyrproject/zephyr/tutorials/motor_control_prac/build/esp-idf/src/EspIdfBootloader-stamp"
  "/home/rog/zephyrproject/zephyr/tutorials/motor_control_prac/build/esp-idf/src"
  "/home/rog/zephyrproject/zephyr/tutorials/motor_control_prac/build/esp-idf/src/EspIdfBootloader-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "/home/rog/zephyrproject/zephyr/tutorials/motor_control_prac/build/esp-idf/src/EspIdfBootloader-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "/home/rog/zephyrproject/zephyr/tutorials/motor_control_prac/build/esp-idf/src/EspIdfBootloader-stamp${cfgdir}") # cfgdir has leading slash
endif()

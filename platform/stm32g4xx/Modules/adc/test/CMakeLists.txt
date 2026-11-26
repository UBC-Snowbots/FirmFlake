cmake_minimum_required(VERSION 3.20)
project(stm32g4_adc_host_tests LANGUAGES CXX)

if(CMAKE_CROSSCOMPILING OR NOT CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    message(FATAL_ERROR "The ADC host tests require native GCC.")
endif()

set(HAL_DRIVERS "${CMAKE_CURRENT_SOURCE_DIR}/../../../HAL_Drivers")

add_executable(adc_host_tests
    ../adc_stm32g4xx.cpp
    fake_hal/fake_hal.cpp
    test_adc_controller.cpp
)
target_compile_features(adc_host_tests PRIVATE cxx_std_20)
set_target_properties(adc_host_tests PROPERTIES CXX_EXTENSIONS NO)
target_compile_definitions(adc_host_tests PRIVATE STM32G491xx)
target_include_directories(adc_host_tests PRIVATE .. fake_hal)
target_include_directories(adc_host_tests SYSTEM PRIVATE
    "${HAL_DRIVERS}/STM32G4xx_HAL_Driver/Inc"
    "${HAL_DRIVERS}/CMSIS/Device/ST/STM32G4xx/Include"
    "${HAL_DRIVERS}/CMSIS/Include"
)
target_compile_options(adc_host_tests PRIVATE -Wall -Wextra)
if(CMAKE_SIZEOF_VOID_P GREATER 4)
    # Unused vendor LL inline functions cast pointers to 32-bit integers.
    # The controller and the test stubs do not use these helpers.
    target_compile_options(adc_host_tests PRIVATE -fpermissive)
endif()

enable_testing()
add_test(NAME adc_controller COMMAND adc_host_tests)

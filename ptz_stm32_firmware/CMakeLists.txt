cmake_minimum_required(VERSION 3.22)
project(ptz_stm32_firmware C CXX ASM)

set(CMAKE_C_STANDARD 11)
#use modern CPP std 20
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)  # disables -std=gnu++20, forces -std=c++20
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "" FORCE)
endif()

set(COMMON_LIBS_PATH ../common_libraries)

set(MCU_FLAGS "-mcpu=cortex-m4 -mthumb -mfpu=fpv4-sp-d16 -mfloat-abi=hard")
set(LINKER_SCRIPT ${CMAKE_SOURCE_DIR}/STM32G474RETx_FLASH.ld)
# set(LINKER_SCRIPT ${CMAKE_SOURCE_DIR}/STM32G431CBUx_FLASH.ld)

set(CMAKE_C_FLAGS   "${MCU_FLAGS} -Wall -fdata-sections -ffunction-sections")
set(CMAKE_CXX_FLAGS "${MCU_FLAGS} -Wall -fdata-sections -ffunction-sections -fno-exceptions -fno-rtti")
set(CMAKE_ASM_FLAGS "${MCU_FLAGS} -Wall -fdata-sections -ffunction-sections")
set(CMAKE_EXE_LINKER_FLAGS
    "${MCU_FLAGS} -specs=nano.specs -T${LINKER_SCRIPT} -Wl,-Map=output.map,--cref -Wl,--gc-sections -lc -lm -lnosys"
)

#TODO glob sources / platform specific
add_executable(${PROJECT_NAME}
    Core/Src/main.cpp
    Core/Src/app/app.cpp
    Core/Inc/usr_leds/usr_led.cpp
    Core/Inc/clock/stopwatch.cpp
    Core/Inc/clock/timer.cpp
    
    # Platform Specific
    Core/Inc/clock/clock_stm32g4xx.cpp
    Core/Inc/primitive_io/primitive_io_stm32g4xx.cpp
    Core/Inc/servo/servo_stm32g4xx.cpp
    Core/Inc/can_comms/can_stm32g4xx.cpp
    
    # Common Lib
    ${COMMON_LIBS_PATH}/comms/comms.c
    
    
    # Core/Src/app_freertos.c
    Core/Src/stm32g4xx_it.c
    Core/Src/stm32g4xx_hal_msp.cpp
    Core/Src/stm32g4xx_hal_timebase_tim.c
    Core/Src/system_stm32g4xx.c

    Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_fdcan.c
    Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal.c
    Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_rcc.c
    Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_rcc_ex.c
    Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_flash.c
    Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_flash_ex.c
    Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_flash_ramfunc.c
    Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_gpio.c
    Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_exti.c
    Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_dma.c
    Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_dma_ex.c
    Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_pwr.c
    Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_pwr_ex.c
    Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_cortex.c
    Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_tim.c
    Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_tim_ex.c
    Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_adc.c
    Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_adc_ex.c

    #! Not using freertos currently
    # Middlewares/Third_Party/FreeRTOS/Source/croutine.c
    # Middlewares/Third_Party/FreeRTOS/Source/event_groups.c
    # Middlewares/Third_Party/FreeRTOS/Source/list.c
    # Middlewares/Third_Party/FreeRTOS/Source/queue.c
    # Middlewares/Third_Party/FreeRTOS/Source/stream_buffer.c
    # Middlewares/Third_Party/FreeRTOS/Source/tasks.c
    # Middlewares/Third_Party/FreeRTOS/Source/timers.c
    # Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2/cmsis_os2.c
    # Middlewares/Third_Party/FreeRTOS/Source/portable/MemMang/heap_4.c
    # Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F/port.c

    startup_stm32g474xx.s
    # startup_stm32g431xx.s
)

target_compile_definitions(${PROJECT_NAME} PRIVATE USE_HAL_DRIVER STM32G474xx)
# target_compile_definitions(${PROJECT_NAME} PRIVATE USE_HAL_DRIVER STM32G431xx)

#TODO seperate platform includes
target_include_directories(${PROJECT_NAME} PRIVATE
    Core/Inc
    Core/Inc/app
    Core/Inc/pinout_defs
    Core/Inc/primitive_io 
    Core/Inc/usr_leds
    Core/Inc/clock
    Core/Inc/servo
    Core/Inc/can_comms

    #common
    ${COMMON_LIBS_PATH}/comms

    Drivers/STM32G4xx_HAL_Driver/Inc
    Drivers/STM32G4xx_HAL_Driver/Inc/Legacy
    # No RTOS yet
    # Middlewares/Third_Party/FreeRTOS/Source/include
    # Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2
    # Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F
    Drivers/CMSIS/Device/ST/STM32G4xx/Include
    Drivers/CMSIS/Include
)
#-gdwarf-2
target_compile_options(${PROJECT_NAME} PRIVATE
    $<$<CONFIG:Debug>:-g  -ggdb3 -Og>
    $<$<CONFIG:Release>:-g -ggdb3 -Og>
)

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_SIZE} $<TARGET_FILE:${PROJECT_NAME}>
    COMMAND ${CMAKE_OBJCOPY} -O ihex   $<TARGET_FILE:${PROJECT_NAME}> ${PROJECT_NAME}.hex
    COMMAND ${CMAKE_OBJCOPY} -O binary $<TARGET_FILE:${PROJECT_NAME}> ${PROJECT_NAME}.bin
)
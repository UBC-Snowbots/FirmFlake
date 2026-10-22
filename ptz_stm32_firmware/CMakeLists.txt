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

# ---------------------------------------------------------------------------------------------
# Where code lives
#   This project (ptz_stm32_firmware/)  -> board specific: main/app, CubeMX board files,
#                                          platform_defs.h, pinout_defs, startup + linker script
#   COMMON_LIBS_PATH                    -> platform agnostic: interfaces + generic code built on them
#   PLATFORM_PATH                       -> STM32G4xx specific: implementations of those interfaces
#   HAL_DRIVERS_PATH                    -> ST HAL + CMSIS, one shared copy for every G4 board
# ---------------------------------------------------------------------------------------------
set(FIRMFLAKE_ROOT   ${CMAKE_SOURCE_DIR}/..)
set(COMMON_LIBS_PATH ${FIRMFLAKE_ROOT}/common_libraries)
set(PLATFORM_PATH    ${FIRMFLAKE_ROOT}/platform/stm32g4xx)
set(HAL_DRIVERS_PATH ${PLATFORM_PATH}/HAL_Drivers)

set(MCU_FLAGS "-mcpu=cortex-m4 -mthumb -mfpu=fpv4-sp-d16 -mfloat-abi=hard")
set(LINKER_SCRIPT ${CMAKE_SOURCE_DIR}/STM32G474RETx_FLASH.ld)
# set(LINKER_SCRIPT ${CMAKE_SOURCE_DIR}/STM32G431CBUx_FLASH.ld)

set(CMAKE_C_FLAGS   "${MCU_FLAGS} -Wall -fdata-sections -ffunction-sections")
set(CMAKE_CXX_FLAGS "${MCU_FLAGS} -Wall -fdata-sections -ffunction-sections -fno-exceptions -fno-rtti")
set(CMAKE_ASM_FLAGS "${MCU_FLAGS} -Wall -fdata-sections -ffunction-sections")
set(CMAKE_EXE_LINKER_FLAGS
    "${MCU_FLAGS} -specs=nano.specs -T${LINKER_SCRIPT} -Wl,-Map=output.map,--cref -Wl,--gc-sections -lc -lm -lnosys"
)

add_executable(${PROJECT_NAME}
    # --- Project / application (board specific) ---
    Core/Src/main.cpp
    Core/Src/app/app.cpp

    # --- Common libraries (platform agnostic) ---
    ${COMMON_LIBS_PATH}/usr_leds/usr_led.cpp
    ${COMMON_LIBS_PATH}/clock/stopwatch.cpp
    ${COMMON_LIBS_PATH}/clock/timer.cpp
    ${COMMON_LIBS_PATH}/comms/comms.c

    # --- Platform modules (STM32G4xx implementations of the common interfaces) ---
    ${PLATFORM_PATH}/Modules/clock/clock_stm32g4xx.cpp
    ${PLATFORM_PATH}/Modules/primitive_io/primitive_io_stm32g4xx.cpp
    ${PLATFORM_PATH}/Modules/servo/servo_stm32g4xx.cpp
    ${PLATFORM_PATH}/Modules/can/can_stm32g4xx.cpp

    # --- CubeMX generated board files (interrupts, clocks, msp, syscalls) ---
    # Core/Src/app_freertos.c
    Core/Src/stm32g4xx_it.c
    Core/Src/stm32g4xx_hal_msp.cpp
    Core/Src/stm32g4xx_hal_timebase_tim.c
    Core/Src/system_stm32g4xx.c
    Core/Src/sysmem.c
    Core/Src/syscalls.c

    # --- ST HAL drivers (shared copy). Only list the peripherals this board uses ---
    ${HAL_DRIVERS_PATH}/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_fdcan.c
    ${HAL_DRIVERS_PATH}/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal.c
    ${HAL_DRIVERS_PATH}/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_rcc.c
    ${HAL_DRIVERS_PATH}/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_rcc_ex.c
    ${HAL_DRIVERS_PATH}/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_flash.c
    ${HAL_DRIVERS_PATH}/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_flash_ex.c
    ${HAL_DRIVERS_PATH}/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_flash_ramfunc.c
    ${HAL_DRIVERS_PATH}/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_gpio.c
    ${HAL_DRIVERS_PATH}/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_exti.c
    ${HAL_DRIVERS_PATH}/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_dma.c
    ${HAL_DRIVERS_PATH}/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_dma_ex.c
    ${HAL_DRIVERS_PATH}/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_pwr.c
    ${HAL_DRIVERS_PATH}/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_pwr_ex.c
    ${HAL_DRIVERS_PATH}/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_cortex.c
    ${HAL_DRIVERS_PATH}/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_tim.c
    ${HAL_DRIVERS_PATH}/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_tim_ex.c
    ${HAL_DRIVERS_PATH}/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_adc.c
    ${HAL_DRIVERS_PATH}/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_adc_ex.c

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

    # --- Startup (chip specific, stays with the project) ---
    startup_stm32g474xx.s
)

# Selects the chip variant inside the shared HAL/CMSIS headers
target_compile_definitions(${PROJECT_NAME} PRIVATE USE_HAL_DRIVER STM32G474xx)
# target_compile_definitions(${PROJECT_NAME} PRIVATE USE_HAL_DRIVER STM32G431xx)

# Sources include headers by bare filename (eg. #include <timer.hpp>), so every folder below must be listed.
target_include_directories(${PROJECT_NAME} PRIVATE
    # --- Project (board specific). Core/Inc holds platform_defs.h + stm32g4xx_hal_conf.h ---
    Core/Inc
    Core/Inc/app
    Core/Inc/pinout_defs

    # --- Common libraries (platform agnostic) ---
    ${COMMON_LIBS_PATH}/comms
    ${COMMON_LIBS_PATH}/can
    ${COMMON_LIBS_PATH}/clock
    ${COMMON_LIBS_PATH}/primitive_io
    ${COMMON_LIBS_PATH}/servo
    ${COMMON_LIBS_PATH}/usr_leds
    ${COMMON_LIBS_PATH}/sensors

    # --- Platform (STM32G4xx). Root holds main_datatypes_stm32g4xx.h ---
    ${PLATFORM_PATH}
    ${PLATFORM_PATH}/Modules/clock
    ${PLATFORM_PATH}/Modules/primitive_io
    ${PLATFORM_PATH}/Modules/servo
    ${PLATFORM_PATH}/Modules/can

    # --- ST HAL + CMSIS (shared copy) ---
    ${HAL_DRIVERS_PATH}/STM32G4xx_HAL_Driver/Inc
    ${HAL_DRIVERS_PATH}/STM32G4xx_HAL_Driver/Inc/Legacy
    ${HAL_DRIVERS_PATH}/CMSIS/Device/ST/STM32G4xx/Include
    ${HAL_DRIVERS_PATH}/CMSIS/Include
    # No RTOS yet
    # Middlewares/Third_Party/FreeRTOS/Source/include
    # Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2
    # Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F
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

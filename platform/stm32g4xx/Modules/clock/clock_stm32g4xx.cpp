// clock_stm32g4xx.cpp
#include "clock_stm32g4xx.hpp"

ClockStatus ClockSTM32G4XX::init() {
    // HAL_Init() must have been called before this
    // SysTick is configured by HAL to tick every 1ms by default
    ready = true;
    return ClockStatus::OK;
}

uint32_t ClockSTM32G4XX::get_ms() {
    if (!ready) return 0;
    return HAL_GetTick();
}

void ClockSTM32G4XX::blocking_delay(uint32_t ms_to_block)
{
    HAL_Delay(ms_to_block);
    return;
}

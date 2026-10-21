// clock_stm32g4xx.hpp
#pragma once
#include "clock_interface.hpp"
#include "stm32g4xx_hal.h"

class ClockSTM32G4XX : public ClockInterface {
public:
    ClockStatus init() override;
    uint32_t get_ms() override;
    void blocking_delay(uint32_t ms_to_block) override;
private:
    bool ready = false;
};
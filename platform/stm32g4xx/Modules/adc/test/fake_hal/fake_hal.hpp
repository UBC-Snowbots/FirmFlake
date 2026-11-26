#pragma once

#include "stm32g4xx_hal.h"
#include <vector>

namespace fake_hal {

struct StatusCall {
    unsigned count = 0;
    HAL_StatusTypeDef result = HAL_OK;
    uint32_t error = HAL_ADC_ERROR_NONE;
};

enum class Call { Init, DeInit, Calibrate, ConfigureChannel, Start, Poll, GetValue, Stop };

struct AdcCalls {
    StatusCall calibrate;
    StatusCall configure_channel;
    StatusCall start;
    StatusCall poll;
    StatusCall stop;
    uint32_t value = 0;
    uint32_t calibration_mode = 0;
    ADC_ChannelConfTypeDef channel{};
    uint32_t timeout_ms = 0;
    const ADC_HandleTypeDef* last_handle = nullptr;
    std::vector<Call> log;
};

extern AdcCalls calls;

} // namespace fake_hal

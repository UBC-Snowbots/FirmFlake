#pragma once

#include "stm32g4xx_hal_def.h"

// Keep vendor instance/channel validation while using host register storage.
extern "C++" {
namespace fake_hal {
extern ADC_TypeDef adc1;
extern ADC_TypeDef adc2;
extern ADC_TypeDef adc3;
extern ADC_Common_TypeDef adc12_common;
extern ADC_Common_TypeDef adc345_common;
} // namespace fake_hal
}

#undef ADC1
#undef ADC2
#undef ADC3
#undef ADC12_COMMON
#undef ADC345_COMMON

#define ADC1 (&::fake_hal::adc1)
#define ADC2 (&::fake_hal::adc2)
#define ADC3 (&::fake_hal::adc3)
#define ADC12_COMMON (&::fake_hal::adc12_common)
#define ADC345_COMMON (&::fake_hal::adc345_common)

#define USE_HAL_ADC_REGISTER_CALLBACKS 0U
#include "stm32g4xx_hal_dma.h"
#include "stm32g4xx_hal_adc.h"

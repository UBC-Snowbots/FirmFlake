#pragma once
#include <stm32g4xx_hal.h>
#include "primitive_io_base.hpp"

// Ensure RCC clock is initiated for this port

class PrimitiveDigitalOutputSTM32G4XX : public PrimitiveDigitalOutput {
public:
    PrimitiveDigitalOutputSTM32G4XX(GPIO_TypeDef* port, uint16_t pin, uint32_t mode, uint32_t pull)
        : port(port), pin(pin), mode(mode), pull(pull), ready(false) {};

    bool init() override;
    bool write(bool state) override;

private:
    GPIO_TypeDef* port;
    uint16_t pin;
    uint32_t mode;
    uint32_t pull;

    bool ready;
};


// Ensure RCC clocks are enabled for the GPIO port and the ADC instance, and that
// the ADC kernel clock source is configured (HAL_RCCEx_PeriphCLKConfig), before init().
//
// Reference config for a basic single-ended, software-triggered, 12-bit read:
//
//   ADC_InitTypeDef init{};
//   init.ClockPrescaler        = ADC_CLOCK_SYNC_PCLK_DIV4;
//   init.Resolution            = ADC_RESOLUTION_12B;
//   init.DataAlign             = ADC_DATAALIGN_RIGHT;
//   init.ScanConvMode          = ADC_SCAN_DISABLE;
//   init.EOCSelection          = ADC_EOC_SINGLE_CONV;
//   init.LowPowerAutoWait      = DISABLE;
//   init.ContinuousConvMode    = DISABLE;
//   init.NbrOfConversion       = 1;
//   init.DiscontinuousConvMode = DISABLE;
//   init.ExternalTrigConv      = ADC_SOFTWARE_START;
//   init.ExternalTrigConvEdge  = ADC_EXTERNALTRIGCONVEDGE_NONE;
//   init.DMAContinuousRequests = DISABLE;
//   init.Overrun               = ADC_OVR_DATA_OVERWRITTEN;
//   init.OversamplingMode      = DISABLE;
//
//   ADC_ChannelConfTypeDef chan{};
//   chan.Channel      = ADC_CHANNEL_1;
//   chan.Rank         = ADC_REGULAR_RANK_1;
//   chan.SamplingTime = ADC_SAMPLETIME_47CYCLES_5;
//   chan.SingleDiff   = ADC_SINGLE_ENDED;
//   chan.OffsetNumber = ADC_OFFSET_NONE;
//   chan.Offset       = 0;

class PrimitiveAnalogInputSTM32G4XX : public PrimitiveAnalogInput {
public:
    // channel: ADC_CHANNEL_x for this pin (always authoritative — see note below).
    // adc_init / channel_config: optional overrides; nullptr uses the static
    // defaults. A supplied channel_config's Channel is overwritten by `channel`.
    PrimitiveAnalogInputSTM32G4XX(GPIO_TypeDef* port, uint16_t pin,
                                  ADC_TypeDef* adc_instance, uint32_t channel,
                                  const ADC_InitTypeDef* adc_init = nullptr,
                                  const ADC_ChannelConfTypeDef* channel_config = nullptr);

    bool init() override;
    bool read(uint32_t& raw_value) override;

    // Defaults: single-ended, software-triggered, 12-bit single conversion.
    static ADC_InitTypeDef default_init();
    static ADC_ChannelConfTypeDef default_channel_config(uint32_t channel);

    void set_index(uint32_t index) override {
        this->index = index;
    }
private:
    GPIO_TypeDef* port;
    uint16_t pin;

    ADC_HandleTypeDef hadc{};
    ADC_ChannelConfTypeDef channel_config;
    bool ready;

    uint32_t index = 99;
};
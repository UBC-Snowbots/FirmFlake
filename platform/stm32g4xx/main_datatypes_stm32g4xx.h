#pragma once
#include <stm32g4xx_hal.h>

// Silicone specific datatypes (eg, stuff used for hardware inits)

struct LedPanelHardware_t {
    GPIO_TypeDef* port;              // GPIOA/GPIOB/... (pointer, see note below)
    uint16_t      pin;               // GPIO_PIN_x bitmask
    uint32_t      alternate_function;// GPIO_AFy_TIMz selecting the timer channel
    TIM_TypeDef*  timer_instance;
    uint32_t      channel;
};

struct AnalogInputHardware_t {
    GPIO_TypeDef*   port;
    uint16_t        pin;
    ADC_TypeDef*    adc_instance;
    uint32_t        channel;
};

struct ServoHardware_t
{
    GPIO_TypeDef *port;
    uint16_t      pin;
    uint32_t      alternate_function;
    TIM_TypeDef  *timer_instance;
    uint32_t      channel;
};



struct ServoPwmConfig
{
    // Kernel clock actually feeding this timer's prescaler, in Hz. Needed because a servo
    // cares about absolute pulse *time*, not duty percent, so the driver has to know how
    // long a tick is. Watch out: on G4 the APB timer clock is often 2x the APB bus clock.
    uint32_t timer_clock_hz;

    uint32_t pwm_clock_prescaler;  // eg 170 -> 1MHz tick off a 170MHz timer. (-1 handled by class)
    uint32_t ticks_per_pwm_period; // eg 20000 ticks @ 1MHz -> 20ms -> 50Hz. (-1 handled by class)
                                   // Must fit the timer's ARR (16-bit on the general purpose ones).
};
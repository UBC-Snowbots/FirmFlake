#pragma once

#include "servo_interface.hpp"
#include <main_datatypes_stm32g4xx.h>
#include <platform_defs.h>
#include <stm32g4xx_hal.h>
#include <primitive_io_base.hpp>

// Mirrors LedPanelHardware_t -- this probably belongs in main_datatypes_stm32g4xx.h next to
// it rather than here, but it's kept local for now so this drops in without touching shared
// headers. If you'd rather not have two identical structs, rename that one to PwmHardware_t
// and share it.


// 1MHz tick / 50Hz frame off a 170MHz timer. 1us of resolution over a 1000us range is
// ~1000 steps, ie ~0.18 deg on a 180 deg servo -- finer than the servo's own deadband.
static constexpr ServoPwmConfig SERVO_PWM_50HZ_170MHZ = {170000000u, 170u, 20000u};
static constexpr ServoPwmConfig SERVO_PWM_50HZ_16MHZ = {16000000u, 16u, 20000u};
class ServoSTM32G4XX : public ServoInterface
{
public:
    // feedback_sensor is optional -- most servos are open loop. Pass nullptr (default) and
    // read_sensor() will return NO_SENSOR.
    ServoSTM32G4XX(ServoHardware_t hw,
                   TIM_HandleTypeDef &htim,
                   const ServoPwmConfig pwm_config,
                   const ServoConfig servo_config,
                   uint8_t index,
                   PrimitiveAnalogInput *feedback_sensor = nullptr)
        : port(hw.port),
          pin(hw.pin),
          af(hw.alternate_function),
          instance(*hw.timer_instance),
          channel(hw.channel),
          htim(htim),
          pwm_clock_prescaler(pwm_config.pwm_clock_prescaler - 1u),
          ticks_per_pwm_period(pwm_config.ticks_per_pwm_period - 1u),
          full_duty(pwm_config.ticks_per_pwm_period),
          ticks_per_us(static_cast<float>(pwm_config.timer_clock_hz /
                                          (pwm_config.pwm_clock_prescaler == 0u
                                               ? 1u
                                               : pwm_config.pwm_clock_prescaler)) /
                       1000000.0f),
          cfg(servo_config),
          current_pulse_us(servo_config.neutral_pulse_us),
          sensor(feedback_sensor),
          index(index) {};

    ServoStatus init(void) override;

    ServoType get_type(void) const override { return cfg.type; }

    ServoStatus set_angle(float angle_deg) override;
    ServoStatus set_speed(float speed_percent) override;
    ServoStatus set_pulse_us(uint32_t pulse_us) override;

    uint32_t get_pulse_us(void) const override { return current_pulse_us; }
    float    get_setpoint(void) const override { return setpoint; }

    ServoStatus enable(void) override;
    ServoStatus disable(void) override;
    bool        is_enabled(void) const override { return enabled; }

    bool        has_sensor(void) const override { return sensor != nullptr; }
    ServoStatus read_sensor(uint32_t &raw_value) override;

    uint8_t get_index(void) const override { return this->index; }

private:
    uint32_t pulse_us_to_ticks(uint32_t pulse_us) const;
    void     apply_pulse(void); // pushes current_pulse_us (or 0 when disabled) to the CCR

    // Hardware
    GPIO_TypeDef      *port;
    uint16_t           pin;
    uint32_t           af;
    TIM_TypeDef       &instance;
    uint32_t           channel;
    TIM_HandleTypeDef &htim;

    // PWM timebase
    uint32_t pwm_clock_prescaler;
    uint32_t ticks_per_pwm_period;
    uint32_t full_duty;
    float    ticks_per_us;

    // Servo behaviour
    ServoConfig cfg;
    uint32_t    current_pulse_us;
    float       setpoint = 0.0f; // degrees (positional) or percent (continuous)

    // Optional feedback
    PrimitiveAnalogInput *sensor;

    uint8_t index = 99;
    bool    enabled = false;
    bool    ready   = false;
};
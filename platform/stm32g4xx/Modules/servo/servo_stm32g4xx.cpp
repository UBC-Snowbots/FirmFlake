#include "servo_stm32g4xx.hpp"

using enum ServoStatus;

ServoStatus ServoSTM32G4XX::init()
{
    // ---- config sanity. Cheap to check here, expensive to debug on the bench ----
    if (ticks_per_us <= 0.0f)
    {
        return BAD_CONFIG;
    }
    if (cfg.min_pulse_us >= cfg.max_pulse_us)
    {
        return BAD_CONFIG;
    }
    if (cfg.neutral_pulse_us < cfg.min_pulse_us || cfg.neutral_pulse_us > cfg.max_pulse_us)
    {
        return BAD_CONFIG;
    }
    // The pulse has to fit inside one frame with room to spare. If max_pulse >= period the
    // servo just sees a permanently high line and does something exciting.
    const float period_us = static_cast<float>(full_duty) / ticks_per_us;
    if (static_cast<float>(cfg.max_pulse_us) >= period_us)
    {
        return BAD_CONFIG;
    }

    // ---- GPIO ----
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull      = GPIO_NOPULL;
    GPIO_InitStruct.Speed     = GPIO_SPEED_FREQ_LOW; // 50Hz, edges do not matter at all here
    GPIO_InitStruct.Pin       = pin;
    GPIO_InitStruct.Alternate = af;
    HAL_GPIO_Init(port, &GPIO_InitStruct);

    // ---- timer, once per timer ----
    // Several servos will share a timer on different channels, so only the first one through
    // configures the timebase. Relies on HAL setting State to READY after a successful init.
    if (htim.State != HAL_TIM_StateTypeDef::HAL_TIM_STATE_READY)
    {
        htim.Instance               = &instance;
        htim.Init.Prescaler         = pwm_clock_prescaler;
        htim.Init.CounterMode       = TIM_COUNTERMODE_UP;
        htim.Init.Period            = ticks_per_pwm_period;
        htim.Init.ClockDivision     = TIM_CLOCKDIVISION_DIV1;
        htim.Init.RepetitionCounter = 0;
        htim.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
        if (HAL_TIM_Base_Init(&htim) != HAL_OK)
        {
            return NOT_OK;
        }
        if (HAL_TIM_PWM_Init(&htim) != HAL_OK)
        {
            return NOT_OK;
        }
    }

    // ---- channel ----
    TIM_OC_InitTypeDef sConfigOC = {0};
    sConfigOC.OCMode       = TIM_OCMODE_PWM1;
    sConfigOC.Pulse        = 0; // start with no pulse -> servo stays limp until we say otherwise
    sConfigOC.OCPolarity   = TIM_OCPOLARITY_HIGH;
    sConfigOC.OCNPolarity  = TIM_OCNPOLARITY_HIGH;
    sConfigOC.OCFastMode   = TIM_OCFAST_DISABLE;
    sConfigOC.OCIdleState  = TIM_OCIDLESTATE_RESET;
    sConfigOC.OCNIdleState = TIM_OCNIDLESTATE_RESET;

    if (HAL_TIM_PWM_ConfigChannel(&htim, &sConfigOC, channel) != HAL_OK)
    {
        return NOT_OK;
    }
    if (HAL_TIM_PWM_Start(&htim, channel) != HAL_OK)
    {
        return NOT_OK;
    }

    if (sensor != nullptr)
    {
        sensor->set_index(this->index);
        if (sensor->init() == false)
        {
            return NOT_OK;
        }
    }

    this->ready = true;

    // Come up at neutral: stopped for a continuous servo, a known pose for a positional one.
    current_pulse_us = cfg.neutral_pulse_us;
    setpoint = (cfg.type == ServoType::CONTINUOUS)
                   ? 0.0f
                   : (cfg.min_angle_deg + cfg.max_angle_deg) * 0.5f;
    enabled = cfg.start_enabled;
    apply_pulse();

    return OK;
}

ServoStatus ServoSTM32G4XX::set_angle(float angle_deg)
{
    if (this->ready == false)
    {
        return NOT_INITIALIZED;
    }
    if (cfg.type != ServoType::POSITIONAL)
    {
        return WRONG_SERVO_TYPE;
    }

    setpoint         = servo_clampf(angle_deg, cfg.min_angle_deg, cfg.max_angle_deg);
    current_pulse_us = servo_angle_to_pulse_us(cfg, angle_deg);
    apply_pulse();
    return OK;
}

ServoStatus ServoSTM32G4XX::set_speed(float speed_percent)
{
    if (this->ready == false)
    {
        return NOT_INITIALIZED;
    }
    if (cfg.type != ServoType::CONTINUOUS)
    {
        return WRONG_SERVO_TYPE;
    }

    setpoint         = servo_clampf(speed_percent, -100.0f, 100.0f);
    current_pulse_us = servo_speed_to_pulse_us(cfg, speed_percent);
    apply_pulse();
    return OK;
}

ServoStatus ServoSTM32G4XX::set_pulse_us(uint32_t pulse_us)
{
    if (this->ready == false)
    {
        return NOT_INITIALIZED;
    }

    // Clamped, not rejected -- same spirit as set_duty() clamping to 0/100. If you're
    // calibrating and need to go outside these, widen the ServoConfig rather than the clamp.
    if (pulse_us < cfg.min_pulse_us)
    {
        pulse_us = cfg.min_pulse_us;
    }
    if (pulse_us > cfg.max_pulse_us)
    {
        pulse_us = cfg.max_pulse_us;
    }

    current_pulse_us = pulse_us;
    apply_pulse();
    return OK;
}

ServoStatus ServoSTM32G4XX::enable()
{
    if (this->ready == false)
    {
        return NOT_INITIALIZED;
    }
    enabled = true;
    apply_pulse();
    return OK;
}

ServoStatus ServoSTM32G4XX::disable()
{
    if (this->ready == false)
    {
        return NOT_INITIALIZED;
    }
    enabled = false;
    apply_pulse(); // CCR -> 0, ie no pulse at all
    return OK;
}

ServoStatus ServoSTM32G4XX::read_sensor(uint32_t &raw_value)
{
    if (this->ready == false)
    {
        return NOT_INITIALIZED;
    }
    if (sensor == nullptr)
    {
        return NO_SENSOR;
    }

    uint32_t temp_raw_value = 0;
    if (sensor->read(temp_raw_value) == false)
    {
        return NOT_OK;
    }

    raw_value = temp_raw_value;
    return OK;
}

// ---- private ---------------------------------------------------------------

uint32_t ServoSTM32G4XX::pulse_us_to_ticks(uint32_t pulse_us) const
{
    float ticks = static_cast<float>(pulse_us) * ticks_per_us;
    if (ticks < 0.0f)
    {
        ticks = 0.0f;
    }

    uint32_t t = static_cast<uint32_t>(ticks + 0.5f);
    if (t > full_duty)
    {
        t = full_duty;
    }
    return t;
}

void ServoSTM32G4XX::apply_pulse()
{
    // Disabling is done by writing a zero-width pulse rather than HAL_TIM_PWM_Stop(), because
    // the timer is shared between channels and stopping it (or dropping MOE on an advanced
    // timer) would take the other servos down with it. No pulse == limp servo, which is what
    // "disabled" means here anyway.
    const uint32_t compare = enabled ? pulse_us_to_ticks(current_pulse_us) : 0u;
    __HAL_TIM_SET_COMPARE(&htim, channel, compare);
}
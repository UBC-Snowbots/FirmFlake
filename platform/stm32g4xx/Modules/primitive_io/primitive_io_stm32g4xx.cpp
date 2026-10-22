#include "primitive_io_stm32g4xx.hpp"

bool PrimitiveDigitalOutputSTM32G4XX::init(void)
{
    // Early return if already initiated
    if(this->ready)
    {
        return true;
    }
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin   = this->pin;
    GPIO_InitStruct.Mode  = this->mode;
    GPIO_InitStruct.Pull  = this->pull;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(this->port, &GPIO_InitStruct);

    this->ready = true;

    return true;
}

bool PrimitiveDigitalOutputSTM32G4XX::write(bool state)
{
    if(this->ready != true)
    {
        return false;
    }
    //TODO find better ways of detecting success/failure
    GPIO_PinState stm_pin_state = state ? GPIO_PIN_SET : GPIO_PIN_RESET;
    HAL_GPIO_WritePin(this->port, this->pin, stm_pin_state); 


    return true; 
}



// Timeout for a single blocking conversion poll.
static constexpr uint32_t ADC_POLL_TIMEOUT_MS = 10;

bool PrimitiveAnalogInputSTM32G4XX::init(void)
{
    // Early return if already initiated
    if(this->ready)
    {
        return true;
    }

    // GPIO: analog mode, no pull.
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin  = this->pin;
    GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(this->port, &GPIO_InitStruct);

    // ADC peripheral (Instance and Init set in constructor).
    if(HAL_ADC_Init(&this->hadc) != HAL_OK)
    {
        return false;
    }

    // Calibrate against the same single/differential mode the channel uses.
    if(HAL_ADCEx_Calibration_Start(&this->hadc, this->channel_config.SingleDiff) != HAL_OK)
    {
        return false;
    }

    this->ready = true;
    return true;
}

bool PrimitiveAnalogInputSTM32G4XX::read(uint32_t& raw_value)
{
    if(this->ready != true)
    {
        return false;
    }
    // Channel must be configured every read
    if(HAL_ADC_ConfigChannel(&this->hadc, &this->channel_config) != HAL_OK)
    {
        return false;
    }

    if(HAL_ADC_Start(&this->hadc) != HAL_OK)
    {
        return false;
    }

    if(HAL_ADC_PollForConversion(&this->hadc, ADC_POLL_TIMEOUT_MS) != HAL_OK)
    {
        HAL_ADC_Stop(&this->hadc);
        return false;
    }

    raw_value = HAL_ADC_GetValue(&this->hadc);

    HAL_ADC_Stop(&this->hadc);
    return true;
}

ADC_InitTypeDef PrimitiveAnalogInputSTM32G4XX::default_init()
{
    ADC_InitTypeDef init = {0};
    init.ClockPrescaler        = ADC_CLOCK_SYNC_PCLK_DIV4;
    init.Resolution            = ADC_RESOLUTION_12B;
    init.DataAlign             = ADC_DATAALIGN_RIGHT;
    init.ScanConvMode          = ADC_SCAN_DISABLE;
    init.EOCSelection          = ADC_EOC_SINGLE_CONV;
    init.LowPowerAutoWait      = DISABLE;
    init.ContinuousConvMode    = DISABLE;
    init.NbrOfConversion       = 1;
    init.DiscontinuousConvMode = DISABLE;
    init.ExternalTrigConv      = ADC_SOFTWARE_START;
    init.ExternalTrigConvEdge  = ADC_EXTERNALTRIGCONVEDGE_NONE;
    init.DMAContinuousRequests = DISABLE;
    init.Overrun               = ADC_OVR_DATA_OVERWRITTEN;

    //Oversample 32 times, then divide by 32
    init.OversamplingMode                   = ENABLE;
    init.Oversampling.Ratio                 = ADC_OVERSAMPLING_RATIO_32;
    init.Oversampling.RightBitShift         = ADC_RIGHTBITSHIFT_5;
    init.Oversampling.TriggeredMode         = ADC_TRIGGEREDMODE_SINGLE_TRIGGER;
    init.Oversampling.OversamplingStopReset = ADC_REGOVERSAMPLING_CONTINUED_MODE;    return init;
}

ADC_ChannelConfTypeDef PrimitiveAnalogInputSTM32G4XX::default_channel_config(uint32_t channel)
{
    ADC_ChannelConfTypeDef chan = {0};
    chan.Channel      = channel;
    chan.Rank         = ADC_REGULAR_RANK_1;
    chan.SamplingTime = ADC_SAMPLETIME_47CYCLES_5;
    chan.SingleDiff   = ADC_SINGLE_ENDED;
    chan.OffsetNumber = ADC_OFFSET_NONE;
    chan.Offset       = 0;
    return chan;
}

PrimitiveAnalogInputSTM32G4XX::PrimitiveAnalogInputSTM32G4XX(
        GPIO_TypeDef* port, uint16_t pin,
        ADC_TypeDef* adc_instance, uint32_t channel,
        const ADC_InitTypeDef* adc_init,
        const ADC_ChannelConfTypeDef* channel_config)
    : port(port), pin(pin), ready(false)
{
    this->hadc.Instance = adc_instance;
    this->hadc.Init     = adc_init ? *adc_init : default_init();

    this->channel_config = channel_config ? *channel_config : default_channel_config(channel);
    this->channel_config.Channel = channel;   // channel arg is authoritative
}
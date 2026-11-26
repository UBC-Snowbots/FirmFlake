#include "fake_hal.hpp"

namespace fake_hal {

ADC_TypeDef adc1{};
ADC_TypeDef adc2{};
ADC_TypeDef adc3{};
ADC_Common_TypeDef adc12_common{};
ADC_Common_TypeDef adc345_common{};
AdcCalls calls{};

} // namespace fake_hal

namespace {

void record(fake_hal::Call call, const ADC_HandleTypeDef* handle)
{
    fake_hal::calls.log.push_back(call);
    fake_hal::calls.last_handle = handle;
}

HAL_StatusTypeDef respond(fake_hal::StatusCall& call, ADC_HandleTypeDef* handle)
{
    ++call.count;
    // Simulate HAL calls overwriting previous error codes.
    handle->ErrorCode = call.error;
    return call.result;
}

} // namespace

// Detect unexpected initialization or deinitialization by the controller.
HAL_StatusTypeDef HAL_ADC_Init(ADC_HandleTypeDef* handle)
{
    record(fake_hal::Call::Init, handle);
    return HAL_ERROR;
}

HAL_StatusTypeDef HAL_ADC_DeInit(ADC_HandleTypeDef* handle)
{
    record(fake_hal::Call::DeInit, handle);
    return HAL_ERROR;
}

HAL_StatusTypeDef HAL_ADCEx_Calibration_Start(ADC_HandleTypeDef* handle, uint32_t mode)
{
    record(fake_hal::Call::Calibrate, handle);
    fake_hal::calls.calibration_mode = mode;
    return respond(fake_hal::calls.calibrate, handle);
}

HAL_StatusTypeDef HAL_ADC_ConfigChannel(ADC_HandleTypeDef* handle,
                                      const ADC_ChannelConfTypeDef* channel)
{
    record(fake_hal::Call::ConfigureChannel, handle);
    fake_hal::calls.channel = *channel;
    return respond(fake_hal::calls.configure_channel, handle);
}

HAL_StatusTypeDef HAL_ADC_Start(ADC_HandleTypeDef* handle)
{
    record(fake_hal::Call::Start, handle);
    return respond(fake_hal::calls.start, handle);
}

HAL_StatusTypeDef HAL_ADC_PollForConversion(ADC_HandleTypeDef* handle, uint32_t timeout)
{
    record(fake_hal::Call::Poll, handle);
    fake_hal::calls.timeout_ms = timeout;
    return respond(fake_hal::calls.poll, handle);
}

uint32_t HAL_ADC_GetValue(const ADC_HandleTypeDef* handle)
{
    record(fake_hal::Call::GetValue, handle);
    return fake_hal::calls.value;
}

HAL_StatusTypeDef HAL_ADC_Stop(ADC_HandleTypeDef* handle)
{
    record(fake_hal::Call::Stop, handle);
    return respond(fake_hal::calls.stop, handle);
}

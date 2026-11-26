#include "adc_stm32g4xx.hpp"


namespace {

AdcResult validation_failure(AdcStatus status) noexcept
{
    AdcResult result{};
    result.status = status;
    result.operation = AdcOperation::Validate;

    return result;
}

AdcResult hal_failure(
    const ADC_HandleTypeDef& handle,
    AdcOperation operation,
    HAL_StatusTypeDef status) noexcept
{
    AdcResult result{};

    result.operation = operation;
    result.hal_status = status;
    result.hal_error = handle.ErrorCode;

    switch (status) {
        case HAL_TIMEOUT:
            result.status = AdcStatus::Timeout;
            break;

        case HAL_BUSY:
            result.status = AdcStatus::Busy;
            break;

        default:
            result.status = AdcStatus::HalFailure;
            break;
    }

    return result;
}

} // namespace


AdcControllerSTM32G4XX::AdcControllerSTM32G4XX(
    ADC_HandleTypeDef& handle) noexcept
    : handle_(handle)
{
}


AdcResult AdcControllerSTM32G4XX::prepare() noexcept
{
    if (state_ != AdcState::Unprepared) {
        return AdcResult{ready_status()};
    }

    auto result = validate_configuration();

    if (result.status != AdcStatus::Ok) {
        return result;
    }

    AdcRawFormat format{};

    result = calculate_raw_format(format);

    if (result.status != AdcStatus::Ok) {
        return result;
    }

    result = calibrate();

    if (result.status == AdcStatus::Ok) {
        format_ = format;
    }

    return result;
}


AdcResult AdcControllerSTM32G4XX::recalibrate() noexcept
{
    const auto status = ready_status();

    if (status != AdcStatus::Ok) {
        return AdcResult{status};
    }

    return calibrate();
}


AdcResult AdcControllerSTM32G4XX::validate_channel(
    const ADC_ChannelConfTypeDef& channel) const noexcept
{
    if (channel.Rank != ADC_REGULAR_RANK_1 ||
        channel.SingleDiff != ADC_SINGLE_ENDED ||
        channel.OffsetNumber != ADC_OFFSET_NONE) {
        return validation_failure(
            AdcStatus::UnsupportedConfiguration);
    }

    if (!IS_ADC_ALL_INSTANCE(handle_.Instance) ||
        !IS_ADC_RESOLUTION(handle_.Init.Resolution) ||
        !IS_ADC_CHANNEL(&handle_, channel.Channel) ||
        !IS_ADC_SAMPLE_TIME(channel.SamplingTime) ||
        !IS_ADC_RANGE(handle_.Init.Resolution, channel.Offset)) {
        return validation_failure(
            AdcStatus::InvalidConfiguration);
    }

    return {};
}


AdcResult AdcControllerSTM32G4XX::read(
    const ADC_ChannelConfTypeDef& channel,
    uint32_t& raw_value,
    uint32_t timeout_ms) noexcept
{
    const auto ready = ready_status();

    if (ready != AdcStatus::Ok) {
        return AdcResult{ready};
    }

    auto result = validate_channel(channel);

    if (result.status != AdcStatus::Ok) {
        return result;
    }

    result = validate_hardware_idle();

    if (result.status != AdcStatus::Ok) {
        return result;
    }

    state_ = AdcState::Busy;

    auto config = channel;

    auto status = HAL_ADC_ConfigChannel(&handle_, &config);

    if (status != HAL_OK) {
        state_ = AdcState::Faulted;

        return hal_failure(
            handle_, AdcOperation::ConfigureChannel, status);
    }

    status = HAL_ADC_Start(&handle_);

    if (status != HAL_OK) {
        result = hal_failure(
            handle_, AdcOperation::Start, status);

        if (status != HAL_BUSY) {
            result.cleanup_status = HAL_ADC_Stop(&handle_);
        }

        state_ = AdcState::Faulted;

        return result;
    }

    status = HAL_ADC_PollForConversion(&handle_, timeout_ms);

    if (status != HAL_OK) {
        result = hal_failure(
            handle_, AdcOperation::Poll, status);

        result.cleanup_status = HAL_ADC_Stop(&handle_);

        // A timeout is recoverable if the ADC stops successfully.
        state_ = (status == HAL_TIMEOUT &&
                  result.cleanup_status == HAL_OK)
            ? AdcState::Ready
            : AdcState::Faulted;

        return result;
    }

    const uint32_t value = HAL_ADC_GetValue(&handle_);

    status = HAL_ADC_Stop(&handle_);

    if (status != HAL_OK) {
        state_ = AdcState::Faulted;

        return hal_failure(
            handle_, AdcOperation::Stop, status);
    }

    // Publish the value only after the conversion has fully completed.
    raw_value = value;
    state_ = AdcState::Ready;

    return {};
}


AdcStatus AdcControllerSTM32G4XX::raw_format(
    AdcRawFormat& format) const noexcept
{
    const auto status = ready_status();

    if (status == AdcStatus::Ok) {
        format = format_;
    }

    return status;
}


AdcState AdcControllerSTM32G4XX::state() const noexcept
{
    return state_;
}


AdcResult AdcControllerSTM32G4XX::validate_configuration() const noexcept
{
    if (!IS_ADC_ALL_INSTANCE(handle_.Instance)) {
        return validation_failure(
            AdcStatus::InvalidConfiguration);
    }

    if ((handle_.State & HAL_ADC_STATE_READY) == 0U) {
        return validation_failure(
            AdcStatus::InvalidConfiguration);
    }

    const auto& init = handle_.Init;

    // Only single-shot, software-triggered polling is supported.
    if (init.DataAlign != ADC_DATAALIGN_RIGHT ||
        init.EOCSelection != ADC_EOC_SINGLE_CONV ||
        init.LowPowerAutoWait != DISABLE ||
        init.ContinuousConvMode != DISABLE ||
        init.DiscontinuousConvMode != DISABLE ||
        init.ExternalTrigConv != ADC_SOFTWARE_START ||
        init.ExternalTrigConvEdge != ADC_EXTERNALTRIGCONVEDGE_NONE ||
        init.DMAContinuousRequests != DISABLE ||
        init.SamplingMode != ADC_SAMPLING_MODE_NORMAL ||
        init.GainCompensation != 0U) {
        return validation_failure(
            AdcStatus::UnsupportedConfiguration);
    }

    if (init.ScanConvMode == ADC_SCAN_ENABLE &&
        init.NbrOfConversion != 1U) {
        return validation_failure(
            AdcStatus::UnsupportedConfiguration);
    }

    if (init.OversamplingMode == ENABLE &&
        init.Oversampling.TriggeredMode !=
            ADC_TRIGGEREDMODE_SINGLE_TRIGGER) {
        return validation_failure(
            AdcStatus::UnsupportedConfiguration);
    }

    // DMA and automatic injected conversions are outside this module's scope.
    if ((handle_.Instance->CFGR &
         (ADC_CFGR_DMAEN | ADC_CFGR_JAUTO)) != 0U) {
        return validation_failure(
            AdcStatus::UnsupportedConfiguration);
    }

#if defined(ADC_MULTIMODE_SUPPORT)
    if ((__HAL_ADC_COMMON_INSTANCE(handle_.Instance)->CCR &
         ADC_CCR_DUAL) != 0U) {
        return validation_failure(
            AdcStatus::UnsupportedConfiguration);
    }
#endif

    return {};
}


AdcResult AdcControllerSTM32G4XX::validate_hardware_idle() const noexcept
{
    constexpr uint32_t active_mask =
        ADC_CR_ADSTART |
        ADC_CR_JADSTART |
        ADC_CR_ADSTP |
        ADC_CR_JADSTP |
        ADC_CR_ADDIS |
        ADC_CR_ADCAL;

    if (handle_.Lock == HAL_LOCKED ||
        (handle_.Instance->CR & active_mask) != 0U) {
        return validation_failure(AdcStatus::Busy);
    }

    return {};
}


AdcResult AdcControllerSTM32G4XX::calculate_raw_format(
    AdcRawFormat& format) const noexcept
{
    const auto& init = handle_.Init;

    uint8_t resolution_bits = 0;
    uint16_t ratio = 1;
    uint8_t right_shift = 0;

    switch (init.Resolution) {
        case ADC_RESOLUTION_12B: resolution_bits = 12; break;
        case ADC_RESOLUTION_10B: resolution_bits = 10; break;
        case ADC_RESOLUTION_8B:  resolution_bits = 8;  break;
        case ADC_RESOLUTION_6B:  resolution_bits = 6;  break;

        default:
            return validation_failure(
                AdcStatus::InvalidConfiguration);
    }

    if (init.OversamplingMode == ENABLE) {
        switch (init.Oversampling.Ratio) {
            case ADC_OVERSAMPLING_RATIO_2:   ratio = 2;   break;
            case ADC_OVERSAMPLING_RATIO_4:   ratio = 4;   break;
            case ADC_OVERSAMPLING_RATIO_8:   ratio = 8;   break;
            case ADC_OVERSAMPLING_RATIO_16:  ratio = 16;  break;
            case ADC_OVERSAMPLING_RATIO_32:  ratio = 32;  break;
            case ADC_OVERSAMPLING_RATIO_64:  ratio = 64;  break;
            case ADC_OVERSAMPLING_RATIO_128: ratio = 128; break;
            case ADC_OVERSAMPLING_RATIO_256: ratio = 256; break;

            default:
                return validation_failure(
                    AdcStatus::InvalidConfiguration);
        }

        switch (init.Oversampling.RightBitShift) {
            case ADC_RIGHTBITSHIFT_NONE: right_shift = 0; break;
            case ADC_RIGHTBITSHIFT_1:    right_shift = 1; break;
            case ADC_RIGHTBITSHIFT_2:    right_shift = 2; break;
            case ADC_RIGHTBITSHIFT_3:    right_shift = 3; break;
            case ADC_RIGHTBITSHIFT_4:    right_shift = 4; break;
            case ADC_RIGHTBITSHIFT_5:    right_shift = 5; break;
            case ADC_RIGHTBITSHIFT_6:    right_shift = 6; break;
            case ADC_RIGHTBITSHIFT_7:    right_shift = 7; break;
            case ADC_RIGHTBITSHIFT_8:    right_shift = 8; break;

            default:
                return validation_failure(
                    AdcStatus::InvalidConfiguration);
        }
    } else if (init.OversamplingMode != DISABLE) {
        return validation_failure(
            AdcStatus::InvalidConfiguration);
    }

    const uint32_t base_max = (1UL << resolution_bits) - 1UL;
    uint32_t max_count = base_max * ratio;

    // Account for hardware rounding during oversampling.
    if (right_shift > 0U) {
        max_count += 1UL << (right_shift - 1U);
        max_count >>= right_shift;
    }

    // The ADC data register holds a 16-bit conversion result.
    if (max_count > 0xFFFFU) {
        return validation_failure(
            AdcStatus::UnsupportedConfiguration);
    }

    format = {
        resolution_bits,
        ratio,
        right_shift,
        max_count
    };

    return {};
}


AdcStatus AdcControllerSTM32G4XX::ready_status() const noexcept
{
    switch (state_) {
        case AdcState::Unprepared:
            return AdcStatus::NotPrepared;

        case AdcState::Ready:
            return AdcStatus::Ok;

        case AdcState::Busy:
            return AdcStatus::Busy;

        case AdcState::Faulted:
            return AdcStatus::Faulted;
    }

    return AdcStatus::HalFailure;
}


AdcResult AdcControllerSTM32G4XX::calibrate() noexcept
{
    auto result = validate_hardware_idle();

    if (result.status != AdcStatus::Ok) {
        return result;
    }

    state_ = AdcState::Busy;

    const auto status =
        HAL_ADCEx_Calibration_Start(&handle_, ADC_SINGLE_ENDED);

    if (status != HAL_OK) {
        state_ = AdcState::Faulted;

        return hal_failure(
            handle_, AdcOperation::Calibrate, status);
    }

    state_ = AdcState::Ready;

    return {};
}
#pragma once

#include <cstdint>
#include "stm32g4xx_hal.h"


enum class AdcStatus : uint8_t {
    Ok,
    NotPrepared,
    Busy,
    InvalidConfiguration,
    UnsupportedConfiguration,
    Timeout,
    HalFailure,
    Faulted
};

enum class AdcOperation : uint8_t {
    None,
    Validate,
    Calibrate,
    ConfigureChannel,
    Start,
    Poll,
    Stop
};

enum class AdcState : uint8_t {
    Unprepared,
    Ready,
    Busy,
    Faulted
};

struct AdcResult {
    AdcStatus status = AdcStatus::Ok;
    AdcOperation operation = AdcOperation::None;
    HAL_StatusTypeDef hal_status = HAL_OK;
    uint32_t hal_error = HAL_ADC_ERROR_NONE;

    // Result of cleanup after a failed conversion.
    HAL_StatusTypeDef cleanup_status = HAL_OK;
};

struct AdcRawFormat {
    uint8_t resolution_bits = 0;
    uint16_t oversampling_ratio = 1;
    uint8_t right_shift = 0;
    uint32_t max_count = 0;
};


// Manages calibration and conversions for one physical ADC peripheral.
// Multiple analog input channels can share the same controller.
//
// The board owns and initializes the HAL handle, which must outlive
// this controller. Access must be serialized by the caller.
class AdcControllerSTM32G4XX final {
public:
    explicit AdcControllerSTM32G4XX(
        ADC_HandleTypeDef& handle) noexcept;

    AdcControllerSTM32G4XX(
        const AdcControllerSTM32G4XX&) = delete;

    AdcControllerSTM32G4XX& operator=(
        const AdcControllerSTM32G4XX&) = delete;

    AdcControllerSTM32G4XX(
        AdcControllerSTM32G4XX&&) = delete;

    AdcControllerSTM32G4XX& operator=(
        AdcControllerSTM32G4XX&&) = delete;

    // Call after board initialization to validate and calibrate the ADC.
    [[nodiscard]] AdcResult prepare() noexcept;

    [[nodiscard]] AdcResult recalibrate() noexcept;

    [[nodiscard]] AdcResult validate_channel(
        const ADC_ChannelConfTypeDef& channel) const noexcept;

    // Blocking conversion. raw_value is unchanged on failure.
    [[nodiscard]] AdcResult read(
        const ADC_ChannelConfTypeDef& channel,
        uint32_t& raw_value,
        uint32_t timeout_ms = 10) noexcept;

    [[nodiscard]] AdcStatus raw_format(
        AdcRawFormat& format) const noexcept;

    [[nodiscard]] AdcState state() const noexcept;

private:
    [[nodiscard]] AdcResult validate_configuration() const noexcept;

    [[nodiscard]] AdcResult validate_hardware_idle() const noexcept;

    [[nodiscard]] AdcResult calculate_raw_format(
        AdcRawFormat& format) const noexcept;

    [[nodiscard]] AdcStatus ready_status() const noexcept;

    [[nodiscard]] AdcResult calibrate() noexcept;

    ADC_HandleTypeDef& handle_;
    AdcState state_ = AdcState::Unprepared;
    AdcRawFormat format_{};
};

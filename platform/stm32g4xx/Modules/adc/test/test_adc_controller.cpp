#include "adc_stm32g4xx.hpp"
#include "fake_hal.hpp"

#include <cstdio>
#include <source_location>
#include <stdexcept>

namespace {

using fake_hal::Call;
using fake_hal::calls;

void check(bool condition,
           const std::source_location where = std::source_location::current())
{
    if (!condition) {
        std::fprintf(stderr, "%s:%u: check failed\n", where.file_name(), where.line());
        throw std::runtime_error("check failed");
    }
}

ADC_HandleTypeDef board_initialized_handle()
{
    fake_hal::adc1 = {};
    fake_hal::adc2 = {};
    fake_hal::adc3 = {};
    fake_hal::adc12_common = {};
    fake_hal::adc345_common = {};
    calls = {};

    ADC_HandleTypeDef handle{};
    handle.Instance = ADC1;
    handle.Lock = HAL_UNLOCKED;
    handle.State = HAL_ADC_STATE_READY;
    handle.Init.Resolution = ADC_RESOLUTION_12B;
    handle.Init.DataAlign = ADC_DATAALIGN_RIGHT;
    handle.Init.ScanConvMode = ADC_SCAN_DISABLE;
    handle.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
    handle.Init.NbrOfConversion = 1;
    handle.Init.ExternalTrigConv = ADC_SOFTWARE_START;
    handle.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
    handle.Init.SamplingMode = ADC_SAMPLING_MODE_NORMAL;
    handle.Init.Overrun = ADC_OVR_DATA_PRESERVED;
    return handle;
}

ADC_ChannelConfTypeDef input_channel(uint32_t number = ADC_CHANNEL_1)
{
    ADC_ChannelConfTypeDef channel{};
    channel.Channel = number;
    channel.Rank = ADC_REGULAR_RANK_1;
    channel.SamplingTime = ADC_SAMPLETIME_47CYCLES_5;
    channel.SingleDiff = ADC_SINGLE_ENDED;
    channel.OffsetNumber = ADC_OFFSET_NONE;
    return channel;
}

void prepare_calibrates_once()
{
    auto handle = board_initialized_handle();
    handle.Init.ClockPrescaler = ADC_CLOCK_ASYNC_DIV4;
    AdcControllerSTM32G4XX controller(handle);
    check(controller.state() == AdcState::Unprepared);
    check(calls.log.empty());

    check(controller.prepare().status == AdcStatus::Ok);
    check(controller.state() == AdcState::Ready);
    check(calls.log == std::vector{Call::Calibrate});
    check(calls.calibration_mode == ADC_SINGLE_ENDED);
    check(calls.last_handle == &handle);
    check(handle.Init.ClockPrescaler == ADC_CLOCK_ASYNC_DIV4);

    check(controller.prepare().status == AdcStatus::Ok);
    check(calls.calibrate.count == 1);
    check(controller.recalibrate().status == AdcStatus::Ok);
    check(calls.log == std::vector{Call::Calibrate, Call::Calibrate});
    check(controller.state() == AdcState::Ready);
}

void unprepared_operations()
{
    auto handle = board_initialized_handle();
    AdcControllerSTM32G4XX controller(handle);
    uint32_t raw = 123;
    check(controller.read(input_channel(), raw).status == AdcStatus::NotPrepared);
    check(raw == 123);
    check(controller.recalibrate().status == AdcStatus::NotPrepared);
    AdcRawFormat format{7, 3, 2, 42};
    check(controller.raw_format(format) == AdcStatus::NotPrepared);
    check(format.resolution_bits == 7 && format.oversampling_ratio == 3 &&
          format.right_shift == 2 && format.max_count == 42);
    check(calls.log.empty());
}

void calibration_failure()
{
    auto handle = board_initialized_handle();
    calls.calibrate.result = HAL_ERROR;
    calls.calibrate.error = HAL_ADC_ERROR_INTERNAL;
    AdcControllerSTM32G4XX controller(handle);
    const auto result = controller.prepare();
    check(result.status == AdcStatus::HalFailure);
    check(result.operation == AdcOperation::Calibrate);
    check(result.hal_status == HAL_ERROR);
    check(result.hal_error == HAL_ADC_ERROR_INTERNAL);
    check(controller.state() == AdcState::Faulted);

    uint32_t raw = 123;
    check(controller.read(input_channel(), raw).status == AdcStatus::Faulted);
    check(raw == 123);
    check(controller.prepare().status == AdcStatus::Faulted);
    check(controller.recalibrate().status == AdcStatus::Faulted);
    AdcRawFormat format{7, 3, 2, 42};
    check(controller.raw_format(format) == AdcStatus::Faulted);
    check(format.resolution_bits == 7 && format.oversampling_ratio == 3 &&
          format.right_shift == 2 && format.max_count == 42);
    check(calls.log == std::vector{Call::Calibrate});
}

void reject_unsupported_configuration()
{
    const struct {
        void (*configure)(ADC_HandleTypeDef&);
        AdcStatus status;
    } cases[] = {
        {[](ADC_HandleTypeDef& h) { h.Instance = nullptr; }, AdcStatus::InvalidConfiguration},
        {[](ADC_HandleTypeDef& h) { h.State = HAL_ADC_STATE_RESET; }, AdcStatus::InvalidConfiguration},
        {[](ADC_HandleTypeDef& h) { h.Init.ContinuousConvMode = ENABLE; }, AdcStatus::UnsupportedConfiguration},
        {[](ADC_HandleTypeDef& h) { h.Init.ExternalTrigConv = ADC_EXTERNALTRIG_T1_TRGO; },
         AdcStatus::UnsupportedConfiguration},
        {[](ADC_HandleTypeDef& h) { h.Init.ScanConvMode = ADC_SCAN_ENABLE; h.Init.NbrOfConversion = 2; },
         AdcStatus::UnsupportedConfiguration},
        {[](ADC_HandleTypeDef& h) {
            h.Init.OversamplingMode = ENABLE;
            h.Init.Oversampling.Ratio = ADC_OVERSAMPLING_RATIO_2;
            h.Init.Oversampling.RightBitShift = ADC_RIGHTBITSHIFT_NONE;
            h.Init.Oversampling.TriggeredMode = ADC_TRIGGEREDMODE_MULTI_TRIGGER;
            h.Init.Oversampling.OversamplingStopReset = ADC_REGOVERSAMPLING_CONTINUED_MODE;
        }, AdcStatus::UnsupportedConfiguration},
        {[](ADC_HandleTypeDef& h) { h.Instance->CFGR = ADC_CFGR_DMAEN; }, AdcStatus::UnsupportedConfiguration},
        {[](ADC_HandleTypeDef& h) { h.Instance->CFGR = ADC_CFGR_JAUTO; }, AdcStatus::UnsupportedConfiguration},
        {[](ADC_HandleTypeDef&) { ADC12_COMMON->CCR = ADC_DUALMODE_REGSIMULT; }, AdcStatus::UnsupportedConfiguration},
    };
    for (const auto& test : cases) {
        auto handle = board_initialized_handle();
        test.configure(handle);
        AdcControllerSTM32G4XX controller(handle);
        const auto result = controller.prepare();
        check(result.status == test.status);
        check(result.operation == AdcOperation::Validate);
        check(controller.state() == AdcState::Unprepared);
        check(calls.log.empty());
    }
}

void reject_invalid_channels()
{
    const struct {
        void (*configure)(ADC_ChannelConfTypeDef&);
        AdcStatus status;
    } cases[] = {
        {[](ADC_ChannelConfTypeDef& c) { c.Channel = 0xFFFFFFFFU; }, AdcStatus::InvalidConfiguration},
        {[](ADC_ChannelConfTypeDef& c) { c.SamplingTime = 0xFFFFFFFFU; }, AdcStatus::InvalidConfiguration},
        {[](ADC_ChannelConfTypeDef& c) { c.SingleDiff = ADC_DIFFERENTIAL_ENDED; }, AdcStatus::UnsupportedConfiguration},
        {[](ADC_ChannelConfTypeDef& c) { c.OffsetNumber = ADC_OFFSET_1; c.Offset = 10; },
         AdcStatus::UnsupportedConfiguration},
        {[](ADC_ChannelConfTypeDef& c) { c.Rank = ADC_REGULAR_RANK_2; }, AdcStatus::UnsupportedConfiguration},
    };
    for (const auto& test : cases) {
        auto handle = board_initialized_handle();
        AdcControllerSTM32G4XX controller(handle);
        check(controller.prepare().status == AdcStatus::Ok);
        calls.log.clear();
        auto channel = input_channel();
        test.configure(channel);
        check(controller.validate_channel(channel).status == test.status);
        uint32_t raw = 123;
        const auto result = controller.read(channel, raw);
        check(result.status == test.status);
        check(result.operation == AdcOperation::Validate);
        check(raw == 123);
        check(calls.log.empty());
        check(controller.state() == AdcState::Ready);
    }
}

void two_channels_share_one_controller()
{
    auto handle = board_initialized_handle();
    handle.Instance = ADC2;
    AdcControllerSTM32G4XX controller(handle);
    check(controller.prepare().status == AdcStatus::Ok);
    for (const auto number : {ADC_CHANNEL_11, ADC_CHANNEL_15}) {
        calls.log.clear();
        const auto channel = input_channel(number);
        check(controller.validate_channel(channel).status == AdcStatus::Ok);
        calls.value = (number == ADC_CHANNEL_11) ? 345 : 678;
        uint32_t raw = 0;
        check(controller.read(channel, raw, 37).status == AdcStatus::Ok);
        check(raw == calls.value);
        check(controller.state() == AdcState::Ready);
        check(calls.log == std::vector{Call::ConfigureChannel, Call::Start, Call::Poll,
                                       Call::GetValue, Call::Stop});
        check(calls.last_handle == &handle);
        check(calls.channel.Channel == channel.Channel);
        check(calls.channel.Rank == channel.Rank);
        check(calls.channel.SamplingTime == channel.SamplingTime);
        check(calls.channel.SingleDiff == channel.SingleDiff);
        check(calls.channel.OffsetNumber == channel.OffsetNumber);
        check(calls.channel.Offset == channel.Offset);
        check(calls.timeout_ms == 37);
    }
    check(calls.calibrate.count == 1);
}

void conversion_failures_handling()
{
    const struct {
        fake_hal::StatusCall* failure;
        AdcOperation operation;
        HAL_StatusTypeDef hal;
        AdcStatus status;
        std::vector<Call> sequence;
    } cases[] = {
        {&calls.configure_channel, AdcOperation::ConfigureChannel, HAL_ERROR, AdcStatus::HalFailure,
         {Call::ConfigureChannel}},
        {&calls.start, AdcOperation::Start, HAL_ERROR, AdcStatus::HalFailure,
         {Call::ConfigureChannel, Call::Start, Call::Stop}},
        {&calls.start, AdcOperation::Start, HAL_BUSY, AdcStatus::Busy,
         {Call::ConfigureChannel, Call::Start}},
        {&calls.poll, AdcOperation::Poll, HAL_ERROR, AdcStatus::HalFailure,
         {Call::ConfigureChannel, Call::Start, Call::Poll, Call::Stop}},
        {&calls.stop, AdcOperation::Stop, HAL_ERROR, AdcStatus::HalFailure,
         {Call::ConfigureChannel, Call::Start, Call::Poll, Call::GetValue, Call::Stop}},
    };
    for (const auto& test : cases) {
        auto handle = board_initialized_handle();
        AdcControllerSTM32G4XX controller(handle);
        check(controller.prepare().status == AdcStatus::Ok);
        calls.log.clear();
        test.failure->result = test.hal;
        test.failure->error = HAL_ADC_ERROR_INTERNAL;
        calls.value = 999;
        uint32_t raw = 123;
        const auto result = controller.read(input_channel(), raw);
        check(result.status == test.status);
        check(result.operation == test.operation);
        check(result.hal_status == test.hal);
        check(result.hal_error == HAL_ADC_ERROR_INTERNAL);
        check(result.cleanup_status == HAL_OK);
        check(raw == 123);
        check(calls.log == test.sequence);
        check(controller.state() == AdcState::Faulted);
        if (test.operation == AdcOperation::Poll ||
            (test.operation == AdcOperation::Start && test.hal != HAL_BUSY)) {
            check(handle.ErrorCode == HAL_ADC_ERROR_NONE);
        }
    }
}

void timeout_recovery()
{
    for (const auto stop_status : {HAL_OK, HAL_ERROR}) {
        auto handle = board_initialized_handle();
        AdcControllerSTM32G4XX controller(handle);
        check(controller.prepare().status == AdcStatus::Ok);
        calls.log.clear();
        calls.poll.result = HAL_TIMEOUT;
        calls.poll.error = HAL_ADC_ERROR_OVR;
        calls.stop.result = stop_status;
        calls.stop.error = stop_status == HAL_OK ? HAL_ADC_ERROR_NONE : HAL_ADC_ERROR_INTERNAL;
        uint32_t raw = 123;
        const auto result = controller.read(input_channel(), raw);
        check(result.status == AdcStatus::Timeout);
        check(result.operation == AdcOperation::Poll);
        check(result.hal_status == HAL_TIMEOUT);
        check(result.hal_error == HAL_ADC_ERROR_OVR);
        check(result.cleanup_status == stop_status);
        check(handle.ErrorCode == calls.stop.error);
        check(raw == 123);
        check(calls.log == std::vector{Call::ConfigureChannel, Call::Start, Call::Poll, Call::Stop});
        check(calls.timeout_ms == 10);
        check(controller.state() == (stop_status == HAL_OK ? AdcState::Ready : AdcState::Faulted));
        if (stop_status == HAL_OK) {
            calls.poll.result = HAL_OK;
            calls.poll.error = HAL_ADC_ERROR_NONE;
            calls.value = 456;
            check(controller.read(input_channel(ADC_CHANNEL_2), raw).status == AdcStatus::Ok);
            check(raw == 456);
            check(controller.state() == AdcState::Ready);
        } else {
            calls.log.clear();
            check(controller.read(input_channel(), raw).status == AdcStatus::Faulted);
            check(raw == 123);
            check(calls.log.empty());
        }
    }
}

void hardware_busy()
{
    for (const auto locked : {false, true}) {
        auto handle = board_initialized_handle();
        AdcControllerSTM32G4XX controller(handle);
        check(controller.prepare().status == AdcStatus::Ok);
        calls.log.clear();
        handle.Lock = locked ? HAL_LOCKED : HAL_UNLOCKED;
        handle.Instance->CR = locked ? 0U : ADC_CR_ADSTART;
        uint32_t raw = 123;
        const auto result = controller.read(input_channel(), raw);
        check(result.status == AdcStatus::Busy);
        check(result.operation == AdcOperation::Validate);
        check(raw == 123);
        check(calls.log.empty());
        check(controller.state() == AdcState::Ready);
    }
}

void raw_format_calculation()
{
    const struct {
        uint32_t resolution;
        FunctionalState oversampling;
        uint32_t ratio;
        uint32_t shift;
        AdcRawFormat expected;
    } cases[] = {
        {ADC_RESOLUTION_12B, DISABLE, 0, 0, {12, 1, 0, 4095}},
        {ADC_RESOLUTION_10B, DISABLE, 0, 0, {10, 1, 0, 1023}},
        {ADC_RESOLUTION_12B, ENABLE, ADC_OVERSAMPLING_RATIO_32, ADC_RIGHTBITSHIFT_5, {12, 32, 5, 4095}},
        {ADC_RESOLUTION_12B, ENABLE, ADC_OVERSAMPLING_RATIO_16, ADC_RIGHTBITSHIFT_NONE, {12, 16, 0, 65520}},
        {ADC_RESOLUTION_12B, ENABLE, ADC_OVERSAMPLING_RATIO_2, ADC_RIGHTBITSHIFT_2, {12, 2, 2, 2048}},
    };
    for (const auto& test : cases) {
        auto handle = board_initialized_handle();
        handle.Init.Resolution = test.resolution;
        handle.Init.OversamplingMode = test.oversampling;
        handle.Init.Oversampling.Ratio = test.ratio;
        handle.Init.Oversampling.RightBitShift = test.shift;
        handle.Init.Oversampling.TriggeredMode = ADC_TRIGGEREDMODE_SINGLE_TRIGGER;
        handle.Init.Oversampling.OversamplingStopReset = ADC_REGOVERSAMPLING_CONTINUED_MODE;
        AdcControllerSTM32G4XX controller(handle);
        check(controller.prepare().status == AdcStatus::Ok);
        AdcRawFormat format{};
        check(controller.raw_format(format) == AdcStatus::Ok);
        check(format.resolution_bits == test.expected.resolution_bits);
        check(format.oversampling_ratio == test.expected.oversampling_ratio);
        check(format.right_shift == test.expected.right_shift);
        check(format.max_count == test.expected.max_count);
    }
}

void reject_raw_format_overflow()
{
    auto handle = board_initialized_handle();
    handle.Init.OversamplingMode = ENABLE;
    handle.Init.Oversampling.Ratio = ADC_OVERSAMPLING_RATIO_32;
    handle.Init.Oversampling.RightBitShift = ADC_RIGHTBITSHIFT_NONE;
    handle.Init.Oversampling.TriggeredMode = ADC_TRIGGEREDMODE_SINGLE_TRIGGER;
    handle.Init.Oversampling.OversamplingStopReset = ADC_REGOVERSAMPLING_CONTINUED_MODE;
    AdcControllerSTM32G4XX controller(handle);
    const auto result = controller.prepare();
    check(result.status == AdcStatus::UnsupportedConfiguration);
    check(result.operation == AdcOperation::Validate);
    check(controller.state() == AdcState::Unprepared);
    check(calls.log.empty());
}

} // namespace

int main()
{
    const struct {
        const char* name;
        void (*run)();
    } tests[] = {
        {"preparation calibrates once without board initialization", prepare_calibrates_once},
        {"unprepared operations preserve outputs", unprepared_operations},
        {"calibration failure faults and rejects operations", calibration_failure},
        {"unsupported configuration is rejected before calibration", reject_unsupported_configuration},
        {"invalid channels do not start conversion", reject_invalid_channels},
        {"two channels share one controller", two_channels_share_one_controller},
        {"conversion failures preserve diagnostics and output", conversion_failures_handling},
        {"timeout cleanup allows retry or faults on stop failure", timeout_recovery},
        {"hardware busy rejects read without HAL calls", hardware_busy},
        {"raw format reports resolution accumulation and rounding", raw_format_calculation},
        {"raw format exceeding 16 bits is rejected", reject_raw_format_overflow},
    };
    unsigned failures = 0;
    for (const auto& test : tests) {
        try {
            test.run();
            std::printf("PASS: %s\n", test.name);
        } catch (const std::exception& error) {
            ++failures;
            std::fprintf(stderr, "FAIL: %s (%s)\n", test.name, error.what());
        }
    }
    std::printf("%zu tests, %u failures\n", sizeof(tests) / sizeof(tests[0]), failures);
    return failures == 0 ? 0 : 1;
}

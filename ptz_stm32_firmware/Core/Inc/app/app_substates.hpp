#pragma once

#define NULL_STATE 9999


enum class FancyStartupSubstate
{
    START_FIRST_FLASH,
    RUN_FIRST_FLASH,
    SECOND_FLASH,
    COMPLETE
};
#define FIRST_FLASH_TIME_MS 500


enum class TestPanelsSubstate : uint32_t
{
    START_SEQUENTIAL_ON,
    RUN_SEQUENTIAL_ON,
    ALL_OFF,
    RUN_RAMP,
    COMPLETE
};

// Tunables for the panel test
static constexpr uint32_t TEST_STEP_ON_MS   = 300u;   // gap between each panel lighting up
static constexpr float    TEST_ON_DUTY_PCT  = 20.0f;
static constexpr uint32_t TEST_RAMP_STEP_MS = 20u;    // time per duty step in the ramp
static constexpr float    TEST_RAMP_STEP    = 5.0f;   // % per step → 0..100 in 20 steps

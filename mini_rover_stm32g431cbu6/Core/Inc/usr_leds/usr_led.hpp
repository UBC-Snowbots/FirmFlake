// Dependencies (Not platform specific)
#pragma once
#include <primitive_io_base.hpp>
#include <timer.hpp>
#include <stdint.h>
#include <stddef.h>

static constexpr size_t NUM_USR_LEDS = 4;

enum class UsrLedStatus {
    LED_OK,
    LED_ERROR
};

enum class UsrLedIndex {
    LED_0 = 0,
    LED_1 = 1,
    LED_2 = 2,
    LED_3 = 3
};

// Single LED
class UsrLed {
public:
    UsrLed(PrimitiveDigitalOutput& led_output, bool active_low = true)
    : led_output(led_output), active_low(active_low){};

    UsrLedStatus init();
    UsrLedStatus turn_off();
    UsrLedStatus turn_on();
    UsrLedStatus toggle();
    bool get_state();

    static constexpr bool ON = true; 
    static constexpr bool OFF  = false; 
private:

    PrimitiveDigitalOutput& led_output;
    bool active_low;


    bool ready = false;
    bool state = OFF;

    UsrLedStatus set(bool state);
};

// All usr leds
//TODO FUTURE: Template number of leds
class UsrLedControl {
public:
    UsrLedControl(ClockInterface& clock, UsrLed& led0, UsrLed& led1, UsrLed& led2, UsrLed& led3)
    : clock(clock), leds{led0, led1, led2, led3}, blinking_timers{Timer(clock, true), Timer(clock, true), Timer(clock, true), Timer(clock, true)}    {};

    UsrLedStatus init();
    UsrLedStatus turn_all_off();
    UsrLedStatus turn_all_on();

    UsrLedStatus turn_on(UsrLedIndex index);
    UsrLedStatus turn_off(UsrLedIndex index);

    UsrLedStatus blink_led(UsrLedIndex index, uint32_t period_ms);
    UsrLedStatus stop_blinking_led(UsrLedIndex index);

    UsrLedStatus flash(UsrLedIndex index, uint32_t times = 1, uint32_t period_ms = 200); // Blink a set number of times then turn off
    // UsrLedStatus blink_all(UsrLedIndex index, uint32_t period_ms);

    // Must be called at regular intervals when blinking / using timers
    void update();

    // Overload [] operator
    //! Not using on purpose
    UsrLed& operator[](int i) { return leds[i]; }
    const UsrLed& operator[](int i) const { return leds[i]; }
    

private:
    ClockInterface& clock;
    //TODO cleanup with controlled led struct
    UsrLed leds[NUM_USR_LEDS];
    Timer blinking_timers[NUM_USR_LEDS];
    bool blinking[NUM_USR_LEDS] = {false};
    uint32_t flashes_remaining[NUM_USR_LEDS] = {0}; //0 -> No flashes / disabled. 
    bool ready = false;

};

//Possible refactoring from here:
// Create base class for "Primitive IO" consisting of:
// low speed digital IO (on/off leds, limit switch and other simple sensors)
// analog input/output - utilizing ADCs and DACs, including PWM timers... 

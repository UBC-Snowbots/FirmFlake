// timer.hpp
#pragma once
#include "clock_interface.hpp"

class Timer {
public:
    Timer(ClockInterface& clock, bool auto_restart = false) : clock(clock), auto_restart(auto_restart) {};
    // Init clock
    bool init();
    // Set timer for ms_from_now milliseconds. Also enables timer.
    void set(uint32_t ms_from_now);
    // Check if timer has expired
    bool expired();
    // Check if timer has expired, and return time since expiry
    // @param elapsed_ms set to 0 if timer has not expired yet
    bool expired(uint32_t& elapsed_ms);

    // Restart timer with last duration used
    void restart();

    // Change auto reset behaviour
    void set_auto_restart(bool auto_restart);

    // Turn off timer. If timer is disabled, expired() will always return false.
    void disable(); 
private:
    ClockInterface& clock;
    bool auto_restart;

    uint32_t start_ms = 0;
    uint32_t duration_ms = 0;
    bool initiated = false;
    bool enabled = false;
};
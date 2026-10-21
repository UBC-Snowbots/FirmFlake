// timer.cpp
#include "timer.hpp"

bool Timer::init() {
    ClockStatus status = clock.init();
    if (status != ClockStatus::OK) return false;
    initiated = true;
    return true;
}

void Timer::set(uint32_t ms_from_now) {
    if (!initiated) return;
    start_ms = clock.get_ms();
    duration_ms = ms_from_now;
    this->enabled = true;
}

void Timer::disable()
{
    this->enabled = false;
}

bool Timer::expired() {
    if (!initiated) return false;
    // return (clock.get_ms() - start_ms) >= duration_ms;
    uint32_t ms_since_expiry = 0;
    return this->expired(ms_since_expiry);
}

bool Timer::expired(uint32_t& ms_since_expiry) {
    if (!initiated) {
        ms_since_expiry = 0;
        return false;
    }
    uint32_t now = clock.get_ms();
    uint32_t delta = now - start_ms;
    if (delta >= duration_ms) {
        ms_since_expiry = delta - duration_ms;
        // Auto restart behaviour, does not require user to restart, and stops drift from update delay
        if(auto_restart)
        {
            // If a full duration was skipped, restart for upcomming expiry.
            start_ms += (delta / duration_ms) * duration_ms;
        }
        return true;
    }
    ms_since_expiry = 0;
    return false;
}

void Timer::set_auto_restart(bool auto_restart)
{
    this->auto_restart = auto_restart;
}
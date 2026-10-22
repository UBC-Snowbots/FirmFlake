// stopwatch.hpp
#pragma once
#include "clock_interface.hpp"

class Stopwatch {
public:
    Stopwatch(ClockInterface& clock) : clock(clock) {};
    // Init clock
    bool init();
    // Start stopwatch
    // @note will fail silently if clock is not inited yet
    void start();
    // Stop stopwatch and return time since start() was called
    uint32_t stop();
    // Get elapsed ms since start() was called without stopping
    uint32_t elapsed_ms();
private:
    ClockInterface& clock;
    uint32_t start_ms = 0;
    bool running = false;
    bool ready = false;
};
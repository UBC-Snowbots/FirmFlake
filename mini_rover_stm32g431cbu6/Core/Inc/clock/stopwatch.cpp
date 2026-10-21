// stopwatch.cpp
#include "stopwatch.hpp"

bool Stopwatch::init() {
    ClockStatus status = clock.init();
    if (status != ClockStatus::OK) return false;
    ready = true;
    return true;
}

void Stopwatch::start() {
    if (!ready) return;
    start_ms = clock.get_ms();
    running = true;
}

uint32_t Stopwatch::stop() {
    if (!ready || !running) return 0;
    running = false;
    return clock.get_ms() - start_ms;
}

uint32_t Stopwatch::elapsed_ms() {
    if (!ready || !running) return 0;
    return clock.get_ms() - start_ms;
}
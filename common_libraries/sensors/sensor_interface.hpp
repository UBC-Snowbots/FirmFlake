#pragma once
#include <cstdint>

class Sensor {
public:
    virtual bool init() = 0;

    // Read (activates hardware)
    virtual bool read_raw(uint32_t& raw_value) = 0;
    virtual bool read(float& value) = 0;

    // Get (gets last read value without touching hardware)
    // virtual bool get_raw(uint32_t& raw_value) = 0;
    // virtual bool get(float& value) = 0;
    virtual ~Sensor() = default;
};
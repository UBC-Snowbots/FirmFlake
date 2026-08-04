//TODO sort out with cmake and platform folder later.
//TODO template to have input / output base classes

#pragma once
#include <cstdint>

    static constexpr bool LOW = false; 
    static constexpr bool HIGH  = true; 

    // static constexpr uint32_t ANALOG_MAX = ;

class PrimitiveDigitalOutput {
public:

    virtual bool init(void) = 0;
    virtual bool write(bool state) = 0;
    virtual ~PrimitiveDigitalOutput() = default;
};


class PrimitiveAnalogInput {
public:

    virtual bool init(void) = 0;
    virtual bool read(uint32_t& raw_value) = 0;
    virtual void set_index(uint32_t index) = 0;
    virtual ~PrimitiveAnalogInput() = default;

};
#pragma once
#include <stdint.h>
// Virtual Clock interface (millisecond level control)
// Get time in ms, thats it. 

enum class ClockStatus {
    OK,
    NOT_OK
};

class ClockInterface {
public:
    virtual ClockStatus init() = 0;

    //! Warning, will fail silently if clock is not inited
    // Returns ms since program start
    virtual uint32_t get_ms() = 0; 

    // Delay
    virtual void blocking_delay(uint32_t ms_to_block) = 0;

};
#include "low_pass_fir_filter.h"
#include <stdexcept>
#include <cstdio>

LowPassFIRFilter::LowPassFIRFilter() : LowPassFIRFilter(1) {}

LowPassFIRFilter::LowPassFIRFilter(unsigned int order)
    : buffer_is_empty(true), buffer(), buffer_index(0), order(order),
    coefficient(1.0f / (order + 1)), output(0) {
    // Valid input check
    if (order < 1 || order > MAX_LP_FIR_ORDER) {
        throw std::invalid_argument("order must be at least 1 and less than the max order");
    }
}

unsigned int LowPassFIRFilter::update(unsigned int input) {
    if (this->buffer_is_empty) {
        // fill entire buffer with the first input
        for (int i = 0; i < this->order + 1; i++) {
            this->buffer[i] = input;
        }
        this->buffer_is_empty = false;
        return this->output = input;
    }

    // `input` is the new data point that is begin added. `buffer[buffer_index]` is the old data point that must be removed. 
    this->output = this->output + input * this->coefficient - this->buffer[this->buffer_index] * this->coefficient;
    this->buffer[this->buffer_index] = input;

    // the index must wrap around once it reaches the right most side of the buffer. Note that buffer size is order + 1.
    this->buffer_index = (this->buffer_index + 1) % (this->order + 1);

    return this->output;
}
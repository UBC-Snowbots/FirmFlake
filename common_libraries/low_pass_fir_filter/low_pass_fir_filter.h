#ifndef FIR_FILTER_HPP
#define FIR_FILTER_HPP
#define MAX_LP_FIR_ORDER (10)

/*
This class is used to apply a low pass FIR filter to a stream of inputs with a specific order. This filter is simply taking S=[ current_input, previous_input_1, previous_input_2, ... , previous_input_{order} ], and returning the average.

The buffer is initially filled with the first input to simplfy the algorithm (this removes the need for taking the average of less than "order" + 1 number of inputs).

The buffer is of size "order" + 1 since we need to store the current input, and also remember the oldest input (to remove from the total output in a subsequent update).
*/

template <int order=1>
class LowPassFIRFilter {
private:
    bool buffer_is_empty = true;
    unsigned int buffer[order+1];
    unsigned int buffer_index = 0;
    float coefficient = 1.0f / (order + 1);
    float output = 0;

public:
    LowPassFIRFilter() {
        // Valid input check
        if (order < 1 || order > MAX_LP_FIR_ORDER) {
            throw std::invalid_argument("order must be at least 1 and less than the max order");
        }
    };

    unsigned int getOrder() const {
        return order;
    }

    // Update filter with new input and return filtered output
    unsigned int update(unsigned int input) {
        if (this->buffer_is_empty) {
            // fill entire buffer with the first input
            for (int i = 0; i < order + 1; i++) {
                this->buffer[i] = input;
            }
            this->buffer_is_empty = false;
            return this->output = input;
        }
    
        // `input` is the new data point that is begin added. `buffer[buffer_index]` is the old data point that must be removed. 
        this->output = this->output + input * this->coefficient - this->buffer[this->buffer_index] * this->coefficient;
        this->buffer[this->buffer_index] = input;
    
        // the index must wrap around once it reaches the right most side of the buffer. Note that buffer size is order + 1.
        this->buffer_index = (this->buffer_index + 1) % (order + 1);
    
        return this->output;
    };
};

#endif
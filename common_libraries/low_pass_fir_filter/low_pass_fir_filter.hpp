#ifndef FIR_FILTER_HPP
#define FIR_FILTER_HPP
#define MAX_LP_FIR_ORDER (10)

/*
This class is used to apply a low pass FIR filter to a stream of inputs with a specific order. This filter is simply taking S=[ current_input, previous_input_1, previous_input_2, ... , previous_input_{order} ], and returning the average.

The buffer is initially filled with the first input to simplfy the algorithm (this removes the need for taking the average of less than "order" + 1 number of inputs).

The buffer is of size "order" + 1 since we need to store the current input, and also remember the oldest input (to remove from the total output in a subsequent update).
*/

class LowPassFIRFilter {
private:
    bool buffer_is_empty;
    unsigned int buffer[MAX_LP_FIR_ORDER];
    unsigned int buffer_index;
    float coefficient;
    float output;

public:
    unsigned int order;
    // Default constructor (order 1, equal weights)
    LowPassFIRFilter();

    // Constructor with filter order (uses equal weights)
    // order = number of previous inputs to use
    LowPassFIRFilter(unsigned int order);

    // Update filter with new input and return filtered output
    unsigned int update(unsigned int input);
};

#endif
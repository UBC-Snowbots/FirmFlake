# Low Pass FIR Filter
This is an FIR filter that can be used on any unsigned 32-bit integers. You can specify the order (AKA. the number of previous data points the algorithm considers) when creating an instance of LowPassFIRFilter (default order of 1). The following example filters incoming data with an order of 1:
```cpp
#include <low_pass_fir_filter.hpp>

LowPassFIRFilter filter;

void process_incoming_data(unsigned int input) {
    unsigned int filtered_input = filter.update(input);
    send_filtered_data_to_arm(filtered_input);
}
```

1. Constructor: defaults to order of 1
2. Buffer: used to store previous data points
3. Update function: 
    - Fills the entire buffer with the first input to simplify the algorithm (otherwise cases for when the buffer is not full yet have to be handled differently).
    - Uses sliding window to calculate the average of the correct set of data points.

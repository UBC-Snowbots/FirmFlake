# FirmFlake
Embedded Systems Repository for UBC Rover

This contains all our firmware and embedded code. From both elec and software.

Some build code is committed, which will likely change soon. But we don't have a setup script for zephyr, and use teensy tools to flash our old arm, so by having build code, we don't need to install zephyr on all our computers.

### CONTRIBUTING
Currently, we protect main branch by PR approval and an automated test of our common libraries.
More automated tests will follow.


### ZephyrRTOS
We have some zephyr rtos projects under zephyr_projects. To build/develop them you will need to install zephyr: https://docs.zephyrproject.org/latest/develop/getting_started/index.html

### Arduino
We have some arduino projects, under arduino_firmware.

### common libraries
Under common_libs/ we have our common libraries. These should be hardware agnostic, modular and testable.
We have an example here, that shows the general workflow for creating a common library module, including Catch2 testing.

#### Low Pass FIR Filter
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

### Testing
We use Catch2 for testing cpp and c code. It's an easy to use, and easy to integrate tool for cpp unit tests.
Ideally, all our code has test coverage. Test driven development (TDD) is a powerful process where if done perfectly, you never push a bug that would impact system operations, because your tests would cover every needed operation of the system.

Embedded systems development's biggest difference to regular software is that hardware is always in the loop, and this makes debugging harder. "is it the code or the hardware? is this my problem or elec's problem?" With really good test code, we can blame more things on elec!!

Catch2 docs are in it's readme: https://github.com/catchorg/Catch2?tab=readme-ov-file

A good book on TDD is Test Driven Development for Embedded Systems: https://pragprog.com/titles/jgade/test-driven-development-for-embedded-c/

### ROS2 Tests
We have some ros2 code in here, specifically for testing firmware. Might be moved to RoverFlake2 in the future.

### End Notes
The filestructure is currently based around development enviroments, we may change this to project based structure in the future.
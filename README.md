# FirmFlake
Embedded Systems Repository for UBC Rover

This contains all our firmware and embedded code. From both elec and software.

Some build code is committed, which will likely change soon. But we don't have a setup script for zephyr, and use teensy tools to flash our old arm, so by having build code, we don't need to install zephyr on all our computers.


## Setup

### Install CMake

This is a tool used to run scripts (e.g. setup scripts).

#### Mac
```bash
brew install cmake
cd common_libs/
make build
```

#### Linux / WSL
```bash
sudo apt-get install cmake
cd common_libs/
make build
```

### Install Zephyr

Follow setup instructions [here](https://docs.zephyrproject.org/latest/develop/getting_started/index.html).

## Contributing
Currently, we protect main branch by PR approval and an automated test of our common libraries.
More automated tests will follow.


## ZephyrRTOS
We have some zephyr rtos projects under zephyr_projects. To build/develop them you will need to install zephyr: https://docs.zephyrproject.org/latest/develop/getting_started/index.html

## Arduino
We have some arduino projects, under arduino_firmware.

### Common Libraries
Under common_libraries/ we have our common libraries. These should be hardware agnostic, modular and testable.
We have an example here, that shows the general workflow for creating a common library module, including Catch2 testing.

1. [Low pass FIR filter](common_libraries/low_pass_fir_filter/README.md)

## Testing
We use Catch2 for testing cpp and c code. It's an easy to use, and easy to integrate tool for cpp unit tests.
Ideally, all our code has test coverage. Test driven development (TDD) is a powerful process where if done perfectly, you never push a bug that would impact system operations, because your tests would cover every needed operation of the system.

Embedded systems development's biggest difference to regular software is that hardware is always in the loop, and this makes debugging harder. "is it the code or the hardware? is this my problem or elec's problem?" With really good test code, we can blame more things on elec!!

Catch2 docs are in it's readme: https://github.com/catchorg/Catch2?tab=readme-ov-file

A good book on TDD is Test Driven Development for Embedded Systems: https://pragprog.com/titles/jgade/test-driven-development-for-embedded-c/

For example, to test the common libraries, do:
```bash
cd common_libraries/
make test
```

### ROS2 Tests
We have some ros2 code in here, specifically for testing firmware. Might be moved to RoverFlake2 in the future.

## End Notes
The filestructure is currently based around development enviroments, we may change this to project based structure in the future.
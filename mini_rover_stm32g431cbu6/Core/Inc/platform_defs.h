#pragma once
// #include <stdint.h>
//These should be injected via cmake...?
// Keeping as #define so its easier to refactor later.


#define CLOCK_FREQ_MHz 170

// FDCAN nominal bit timing, used by platform/stm32g4xx/Modules/can/can_stm32g4xx.cpp
// Bitrate = fdcan_kernel_clock / (PRESCALER * (1 + SEG1 + SEG2)).
// Depends on this board's FDCAN clock source (PCLK1, see Core/Src/fdcan.c), so it can't be shared between boards.
#define CAN_NOMINAL_PRESCALER 1
#define CAN_NOMINAL_SJW       4
#define CAN_NOMINAL_SEG1      27
#define CAN_NOMINAL_SEG2      4

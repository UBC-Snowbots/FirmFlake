#include "main.h"
#include "tim.h"
#include "fdcan.h"
#include "can_stm32g4xx.h"

static CanSTM32G4XX can(hfdcan1);

extern "C" void app_main(void) {
    can.init();
    // start PWM, then loop forever
    for (;;) { }
}
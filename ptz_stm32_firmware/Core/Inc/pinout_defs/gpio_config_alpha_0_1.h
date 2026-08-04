// GPIO Config for alpha v0.1 (the first PCB based on stm32g474ret6)
#include <stdint.h>

#include <stm32g4xx_hal.h>
#include <main_datatypes_stm32g4xx.h>

static constexpr uint32_t NUM_LED_PANELS = 10;

namespace pindefs
{
    // Silicone specific pins and hardware

    // PWM PINS FOR LED PANELS
    static const ServoHardware_t servo_tilt_hw = {GPIOA, GPIO_PIN_6, GPIO_AF2_TIM3, TIM3, TIM_CHANNEL_1};
    static const ServoHardware_t servo_pan_hw = {GPIOA, GPIO_PIN_7, GPIO_AF2_TIM3, TIM3, TIM_CHANNEL_2};
 
    // USR LEDS - Turn on when pulled LOW (Active Low)
    // static constexpr uint16_t       USRLED_3_PIN  = GPIO_PIN_10; //10 // Messed up the silkscreen, USRLED_0 is labeled USRLED_3
    // static GPIO_TypeDef* const      USRLED_3_PORT = GPIOB; //B

    // static constexpr uint16_t       USRLED_2_PIN  = GPIO_PIN_11; // Even when off, its still has a bit of glow. idk... i messed up the usr leds for a led control board....
    // static GPIO_TypeDef* const      USRLED_2_PORT = GPIOB;

    // static constexpr uint16_t       USRLED_1_PIN  = GPIO_PIN_12;
    // static GPIO_TypeDef* const      USRLED_1_PORT = GPIOB;

    // static constexpr uint16_t       USRLED_0_PIN  = GPIO_PIN_13; // This LED is really, really friggin bright. That was an accident. The 5v ok led is also really really bright
    // static GPIO_TypeDef* const      USRLED_0_PORT = GPIOB;

    // CAN (FDcan1)
    static constexpr uint16_t       CANFD_RX_PIN  = GPIO_PIN_11;
    static GPIO_TypeDef* const      CANFD_RX_PORT = GPIOA;

    static constexpr uint16_t       CANFD_TX_PIN  = GPIO_PIN_12;
    static GPIO_TypeDef* const      CANFD_TX_PORT = GPIOA;

    // UART (usart2) (Shared with SWCLK and JTDI) 
    static constexpr uint16_t       UART_RX_PIN  = GPIO_PIN_15;
    static GPIO_TypeDef* const      UART_RX_PORT = GPIOA;

    static constexpr uint16_t       UART_TX_PIN  = GPIO_PIN_14;
    static GPIO_TypeDef* const      UART_TX_PORT = UART_RX_PORT;

    // static const AnalogInputHardware_t ADC_INPUTS[] = {
    // //   index    port   pin         inst   channel
    //     /* 0 */ { GPIOC, GPIO_PIN_0, ADC1,  ADC_CHANNEL_6  },  // PC0  ADC1_IN6  (ADC12_IN6)
    //     /* 1 */ { GPIOC, GPIO_PIN_1, ADC1,  ADC_CHANNEL_7  },  // PC1  ADC1_IN7  (ADC12_IN7)
    //     /* 2 */ { GPIOC, GPIO_PIN_2, ADC1,  ADC_CHANNEL_8  },  // PC2  ADC1_IN8  (ADC12_IN8)
    //     /* 3 */ { GPIOC, GPIO_PIN_3, ADC1,  ADC_CHANNEL_9  },  // PC3  ADC1_IN9  (ADC12_IN9)
    //     /* 4 */ { GPIOC, GPIO_PIN_4, ADC2,  ADC_CHANNEL_5  },  // PC4  ADC2_IN5
    //     /* 5 */ { GPIOC, GPIO_PIN_5, ADC2,  ADC_CHANNEL_11 },  // PC5  ADC2_IN11
    //     /* 6 */ { GPIOB, GPIO_PIN_2, ADC2,  ADC_CHANNEL_12 },  // PB2  ADC2_IN12
    //     /* 7 */ { GPIOB, GPIO_PIN_1, ADC1,  ADC_CHANNEL_12 },  // PB1  ADC1_IN12 (or ADC3_IN1)
    //     /* 8 */ { GPIOB, GPIO_PIN_0, ADC1,  ADC_CHANNEL_15 },  // PB0  ADC1_IN15 (or ADC3_IN12)
    //     /* 9 */ { GPIOA, GPIO_PIN_8, ADC5,  ADC_CHANNEL_1  },  // PA8  ADC5_IN1
    // };
};
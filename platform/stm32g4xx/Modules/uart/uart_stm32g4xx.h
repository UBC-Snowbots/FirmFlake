#pragma once
#include <cstddef>
#include <cstdint>

#include <ring_buffer.h>

#include "uart_interface.h"
#include "stm32g4xx_hal.h"

// Fixed around interrupt-based reading for now 
// TODO: add support for DMA (and maybe polling based)

// size of the UART buffer 
constexpr size_t UART_RING_BUFFER_LENGTH = 512;
using UartMessageHandler =  void (*)(const msg_t& message);


class UartSTM32G4XX : public UartInterface {
    // UartSTM32GXX inherits from UartInterface, where the UartSTM32G4XX is the platform specific implementation 
    public:
        UartSTM32G4XX(UART_HandleTypeDef& handle, UartMessageHandler message_handler) 
            :   handle_(handle), // assigns handle to handle_ 
                message_handler_(message_handler)
        {
        }
        
        UartStatus init() override;
        UartStatus send(const uint8_t* data, size_t length) override;
        UartStatus read(uint8_t& byte) override;
        UartStatus process_rxbuf(); // polls Rx buffer until its empty 

        void dispatch(const msg_t& message);
        void rx_interrupt_callback();
    private:
        UART_HandleTypeDef& handle_;
        uint8_t rx_byte_; 

        // uint8_t rx_dma_buffer_[UART_DMA_RXBUFFER_LENGTH];
        // size_t rx_read_pos_ = 0; 
        UartRxBuffer<UART_RING_BUFFER_LENGTH> rx_ring_;
        RingBuffer<COMMS_UART_MSG_SIZE> FrameBuf; // for frame constructing
        UartMessageHandler message_handler_;
};

bool send_message(UartInterface& uart, const msg_t& message);
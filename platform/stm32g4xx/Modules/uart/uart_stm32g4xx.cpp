#include "uart_stm32g4xx.h"
#include <platform_defs.h> 

// only arms UART IT, actual UART init should be done through cubemx
UartStatus UartSTM32G4XX::init() {
    handle_.Init.BaudRate = 115200; // 115200 is a pre standard speed 
    /*
    // Configures UART for 8N1 operation without hardware flow control. 
    handle_.Init.WordLength = UART_WORDLENGTH_8B; // 
    handle_.Init.StopBits = UART_STOPBITS_1;
    handle_.Init.Parity = UART_PARITY_NONE;
    handle_.Init.Mode = UART_MODE_TX_RX;
    handle_.Init.HwFlowCtl = UART_HWCONTROL_NONE; // if u want 2 extra wires (RTS/CTS) 
    handle_.Init.OverSampling = UART_OVERSAMPLING_16;  
    handle_.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE; // uses 3 bit sampling, more robust
    handle_.Init.ClockPrescaler = UART_PRESCALER_DIV1; // ! might want to look into if locking this in is safe 
    handle_.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT; // random UART features like TX/RX auto baud detection

    // use HAL init
    if (HAL_UART_Init(&handle_) != HAL_OK)
        return UartStatus::NOT_OK;
    */
    
    if (HAL_UART_Receive_IT(&handle_, &rx_byte_, 1) != HAL_OK) 
        return UartStatus::NOT_OK;

    return UartStatus::OK;
}

// ------------ Receive -------------
UartStatus UartSTM32G4XX::read(uint8_t& byte) {
    return rx_ring_.pop(byte);
}


// ! double check this is okay. 
void UartSTM32G4XX::rx_interrupt_callback(uint8_t byte)
{
    rx_ring_.push(byte);
    HAL_UART_Receive_IT(&handle_, &rx_byte_, 1);
}

// Call ts in main
UartStatus UartSTM32G4XX::process_rxbuf() {
    UartStatus status;
    uint8_t byte; 

    for (;;) {
        status = UartSTM32G4XX::read(byte);
        if (status == UartStatus::RX_EMPTY) {
            break;
        }
        if (status != UartStatus::OK)
            return status; // theres prolly something wrong

        FrameBuf.push_overwrite(byte);
        if (byte == 0x00) {
            size_t frame_length = FrameBuf.size();
            if (frame_length > 0 && frame_length <= COMMS_UART_MSG_SIZE) {
                encoded_msg_t encoded{};
                uint8_t raw_frame[COMMS_UART_MSG_SIZE];
                const size_t frame_length = FrameBuf.copy_to(raw_frame, COMMS_UART_MSG_SIZE);

                if (convert_from_uart(raw_frame, frame_length, &encoded)){
                    msg_t msg{};
                    if (decode(&encoded, &msg)) {
                        dispatch(msg);
                    }
                }
            }
            FrameBuf.clear(); // reset FrameBuf for next frame. 
        }
    }
    return UartStatus::OK;
}

void UartSTM32G4XX::dispatch(const msg_t& message) {
    if (message_handler_ != nullptr) {
        message_handler_(message);
    }
}

/*
Dispatch Example:
void dispatch_message(const msg_t& message) {
    switch (message.type) {
        case MSG_TYPE__HEARTBEAT:
            // Handle heartbeat
            break;

        case MSG_TYPE__PTZ_VEL_CMD:
            // Handle PTZ command
            break;

        default:
            break;
    }
}
then construction:
UartSTM32G4XX uart(huart4, dispatch_message);
*/


// ------------- UART Send ----------------
/**
 * @returns true if successful; false otherwise. 
 */
bool send_message(UartInterface& uart, const msg_t& message){
    // encode message 
    encoded_msg_t encoded{};
    if (!encode(&message, &encoded))
        return false;
    
    // convert to UART frame 
    uint8_t uart_frame[COMMS_UART_MSG_SIZE]{}; // empty curly brackets => initialized to 0 
    const size_t frame_length = convert_to_uart(&encoded, sizeof(uart_frame), uart_frame);

    if (frame_length == 0 || frame_length > sizeof(uart_frame)) // ensure expected behaviour
        return false; 

    return uart.send(uart_frame, frame_length) == UartStatus::OK;
}

/*
TODO: implement IT-based transmit 
*/
UartStatus UartSTM32G4XX::send(const uint8_t* data, size_t length) {
    HAL_StatusTypeDef HALStatus; 
    HALStatus = HAL_UART_Transmit(&handle_, data, length, HAL_TIMEOUT);

    if (HALStatus != HAL_OK)
        return UartStatus::NOT_OK;

    return UartStatus::OK;
}

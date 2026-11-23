# UART Library
author: Alan Cheng

## Notes
* ring_buffer stuff needs testing, proper testcases would be nice
* more testing of this would be nice

## For use (application level):
Install the Callback, example:
```cpp
static UartSTM32G4XX* uart4_instance = nullptr;
static UartSTM32G4XX* uart5_instance = nullptr;

extern "C" void HAL_UART_RxCpltCallback(UART_HandleTypeDef* huart)
{
    if (huart->Instance == UART4)
        uart4_instance->rx_interrupt_callback(uart4_instance->rx_byte_);
    else if (huart->Instance == UART5)
        uart5_instance->rx_interrupt_callback(uart5_instance->rx_byte_);
}
```
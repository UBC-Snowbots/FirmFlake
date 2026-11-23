# UART Library
author: Alan Cheng

## Notes
* ring_buffer stuff needs testing, proper testcases would be nice
* more testing of this would be nice

## For use (application level):
Install the Callback, example:
```cpp
void uart_msg_handler(const msg_t&);
UartSTM32G4XX uart4(huart4, uart_msg_handler);

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
  if (huart->Instance == huart4.Instance) {
    uart4.rx_interrupt_callback();
  }
}
```
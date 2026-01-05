#include <end_effector_actions.h>

// Interprets an SPI message and sends the corresponding action to the appropriate sensor
void handle_SPI_msg(uint8_t spi_peripheral_rx_buf[BUF_SIZE]) {
    return void;
}

// Interprets a CAN FD message and sends the corresponding action over SPI if permissible
void handle_CAN_FD_message(uint8_t spi_controller_tx_buf[BUF_SIZE]) {
    return void;
}

// Retrieves data from a requested sensor and sends it over SPI
void send_data_update_SPI_msg(uint8_t spi_peripheral_rx_buf[BUF_SIZE], uint8_t spi_peripheral_tx_buf[BUF_SIZE]) {
    return void;
}
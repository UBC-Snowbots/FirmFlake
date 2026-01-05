#ifndef END_EFFECTOR_ACTIONS_H
#define END_EFFECTOR_ACTIONS_H

#include <end_effector_base.h>

/* SPI message struct
 - destination sensor
 - sensor request
 - sensor value
*/

// Interprets an SPI message and sends the corresponding action to the appropriate sensor or as a message over CAN FD if permissible
void handle_SPI_msg(uint8_t spi_peripheral_rx_buf[BUF_SIZE]);

// Interprets a CAN FD message and sends the corresponding action over SPI if permissible
void handle_CAN_FD_message(uint8_t spi_controller_tx_buf[BUF_SIZE]);

// Retrieves data from a requested sensor and sends it over SPI
void send_data_update_SPI_msg(uint8_t spi_peripheral_rx_buf[BUF_SIZE], uint8_t spi_peripheral_tx_buf[BUF_SIZE]);

#endif END_EFFECTOR_ACTIONS_H
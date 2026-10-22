#ifndef CAN_FD_BRIDGE_H
#define CAN_FD_BRIDGE_H

// Reconstructs CAN FD frame from an SPI message
void SPI_to_CAN_FD();

// Sends a CAN FD frame via an SPI message
void CAN_FD_to_SPI();

#endif CAN_FD_BRIDGE_H
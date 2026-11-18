#pragma once
#include <stdint.h>


#ifdef __cplusplus
extern "C" {
#endif
// static inline uint8_t crc8(uint8_t crc, const uint8_t *data, uint32_t length)
// {
//  //TODO if we need it

// }
// commented above for now, dont need it. 

/**
 * @brief CRC16/CCITT-FALSE
 * 
 * Taken from https://gist.github.com/tijnkooijmans/10981093
 * 
 * @param data pointer to [0] of location of data 
 * @param length length of data
 * @retval returns the crc code (CRC/CCIT-FALSE) based on the data 
 */
uint16_t crc16(const uint8_t *pData, uint32_t length);

#ifdef __cplusplus
}
#endif
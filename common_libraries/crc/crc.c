#include "crc.h"

/**
* @note Implementation of CRC16/CCIT-FALSE based on https://gist.github.com/tijnkooijmans/10981093
*/
uint16_t crc16(const uint8_t *pData, uint32_t length) {
    uint8_t i;
    uint16_t wCrc = 0xffff;
    while (length --) { // terminates when length drops to 0 
        wCrc ^= *(unsigned char*)pData++ << 8; // XOR with current byte, then increment pData AFTER XOR operation
        for (i=0; i < 8; i++)
            wCrc = wCrc & 0x8000 ? (wCrc << 1) ^ 0x1021 : wCrc << 1; // wtf (polynomial thingy)
    }
    return wCrc & 0xffff;
}
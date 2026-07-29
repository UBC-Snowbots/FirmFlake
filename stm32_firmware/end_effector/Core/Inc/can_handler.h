#ifndef CAN_HANDLER_H
#define CAN_HANDLER_H

#include "main.h"
#include <stdint.h>

/* TODO: What IDs are appropriate here? */
#define CAN_ID_LOADCELL_TX   0x100   /* transmit load cell data on this ID  */
#define CAN_ID_CMD_RX        0x200   /* listen for commands on this ID      */

/* Commands*/
#define CAN_CMD_TARE         0x01    /* Zero/tare both sensors                 */
#define CAN_CMD_REQUEST_DATA 0x02    /* Immediately send a reading             */

/* CAN frame layout for CAN_ID_LOADCELL_TX (8 bytes)
 *  Byte 0-3 : int32_t  raw ADC value, sensor 1 (big-endian)
 *  Byte 4-7 : int32_t  raw ADC value, sensor 2 (big-endian)
*/

typedef struct {
    uint8_t  tare_requested;
    uint8_t  data_requested;
} CAN_Flags_t;

extern CAN_Flags_t g_can_flags;
extern FDCAN_HandleTypeDef hfdcan1;

HAL_StatusTypeDef CAN_Handler_Init(void);
HAL_StatusTypeDef CAN_Transmit_LoadCells(int32_t raw1, int32_t raw2);

void CAN_Handler_RxCallback(FDCAN_HandleTypeDef *hfdcan);

#endif /* CAN_HANDLER_H */

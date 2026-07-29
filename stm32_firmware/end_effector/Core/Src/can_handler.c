#include "can_handler.h"
#include <string.h>

CAN_Flags_t g_can_flags = { 0 };

HAL_StatusTypeDef CAN_Handler_Init(void)
{
    HAL_StatusTypeDef st;

    /* Configure filter */
    FDCAN_FilterTypeDef filter = {0};
    filter.IdType       = FDCAN_STANDARD_ID;
    filter.FilterIndex  = 0;
    filter.FilterType   = FDCAN_FILTER_MASK;
    filter.FilterConfig = FDCAN_FILTER_TO_RXFIFO0;
    filter.FilterID1    = CAN_ID_CMD_RX;
    filter.FilterID2    = 0x7FF;

    st = HAL_FDCAN_ConfigFilter(&hfdcan1, &filter);
    if (st != HAL_OK) return st;

    /* Reject all frames that don't pass filter */
    st = HAL_FDCAN_ConfigGlobalFilter(&hfdcan1, FDCAN_REJECT, FDCAN_REJECT, FDCAN_FILTER_REMOTE, FDCAN_FILTER_REMOTE);
    if (st != HAL_OK) return st;

    /* Enable FIFO 0 new-message interrupt */
    st = HAL_FDCAN_ActivateNotification(&hfdcan1, FDCAN_IT_RX_FIFO0_NEW_MESSAGE, 0);
    if (st != HAL_OK) return st;

    /* Start the peripheral */
    return HAL_FDCAN_Start(&hfdcan1);
}

HAL_StatusTypeDef CAN_Transmit_LoadCells(int32_t raw1, int32_t raw2)
{
    FDCAN_TxHeaderTypeDef hdr = {0};
    hdr.Identifier          = CAN_ID_LOADCELL_TX;
    hdr.IdType              = FDCAN_STANDARD_ID;
    hdr.TxFrameType         = FDCAN_DATA_FRAME;
    hdr.DataLength          = FDCAN_DLC_BYTES_8;
    hdr.ErrorStateIndicator = FDCAN_ESI_ACTIVE;
    hdr.BitRateSwitch       = FDCAN_BRS_OFF;
    hdr.FDFormat            = FDCAN_CLASSIC_CAN;
    hdr.TxEventFifoControl  = FDCAN_NO_TX_EVENTS;
    hdr.MessageMarker       = 0;

    uint8_t data[8];
    data[0] = (uint8_t)((raw1 >> 24) & 0xFF);
    data[1] = (uint8_t)((raw1 >> 16) & 0xFF);
    data[2] = (uint8_t)((raw1 >>  8) & 0xFF);
    data[3] = (uint8_t)( raw1        & 0xFF);
    data[4] = (uint8_t)((raw2 >> 24) & 0xFF);
    data[5] = (uint8_t)((raw2 >> 16) & 0xFF);
    data[6] = (uint8_t)((raw2 >>  8) & 0xFF);
    data[7] = (uint8_t)( raw2        & 0xFF);

    return HAL_FDCAN_AddMessageToTxFifoQ(&hfdcan1, &hdr, data);
}

void CAN_Handler_RxCallback(FDCAN_HandleTypeDef *hfdcan)
{
    FDCAN_RxHeaderTypeDef rx_hdr;
    uint8_t               rx_data[8];

    while (HAL_FDCAN_GetRxMessage(hfdcan, FDCAN_RX_FIFO0, &rx_hdr, rx_data) == HAL_OK)
    {
        if (rx_hdr.Identifier == CAN_ID_CMD_RX && rx_hdr.DataLength >= FDCAN_DLC_BYTES_1)
        {
            switch (rx_data[0])
            {
                case CAN_CMD_TARE:
                    g_can_flags.tare_requested = 1;
                    break;
                case CAN_CMD_REQUEST_DATA:
                    g_can_flags.data_requested = 1;
                    break;
                default:
                    break;
            }
        }
    }
}

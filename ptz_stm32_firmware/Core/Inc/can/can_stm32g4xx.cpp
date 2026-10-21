#include "can_stm32g4xx.h"


CanStatus CanSTM32G4XX::init()
{
    // GPIO is initiated in msp file
    
    hfdcan.Instance = FDCAN1;
    hfdcan.Init.ClockDivider = FDCAN_CLOCK_DIV1;
    hfdcan.Init.FrameFormat = FDCAN_FRAME_CLASSIC;
    hfdcan.Init.Mode = FDCAN_MODE_NORMAL;
    hfdcan.Init.AutoRetransmission = DISABLE;
    hfdcan.Init.TransmitPause = DISABLE;
    hfdcan.Init.ProtocolException = DISABLE;
    hfdcan.Init.NominalPrescaler = 1;
    hfdcan.Init.NominalSyncJumpWidth = 6;
    hfdcan.Init.NominalTimeSeg1 = 35;
    hfdcan.Init.NominalTimeSeg2 = 12;
    hfdcan.Init.DataPrescaler = 1;
    hfdcan.Init.DataSyncJumpWidth = 1;
    hfdcan.Init.DataTimeSeg1 = 1;
    hfdcan.Init.DataTimeSeg2 = 1;
    hfdcan.Init.StdFiltersNbr = 0;
    hfdcan.Init.ExtFiltersNbr = 0;
    hfdcan.Init.TxFifoQueueMode = FDCAN_TX_FIFO_OPERATION; //FDCAN_TX_QUEUE_OPERATION -> might be better than FIFO, sends lower id msgs first
    if (HAL_FDCAN_Init(&hfdcan) != HAL_OK) // Also runs msp init. see stm32g4xx_hal_msp.cpp
    {
        return CanStatus::NOT_OK;
    }
    if (HAL_FDCAN_Start(&hfdcan) != HAL_OK)
    {
        return CanStatus::NOT_OK;
    }
    return CanStatus::OK;
}

CanStatus CanSTM32G4XX::send(const can_msg_t& msg)
{
    // Peek if queue is full
    if (HAL_FDCAN_GetTxFifoFreeLevel(&hfdcan) == 0u) {
        return CanStatus::ERR_TX_QUEUE_FULL; // Hardware queue is full
    }

    // Prepare tx message
    //first, check dlc
    uint32_t dlc = 0;
    if(!len_to_dlc(msg.payload_len, dlc))
    {
        return CanStatus::NOT_OK;
    }
 
    FDCAN_TxHeaderTypeDef tx_header{};
    tx_header.Identifier          = msg.id & COMMS_CAN_EXT_ID_MASK;   // 29-bit ext id
    tx_header.IdType              = FDCAN_EXTENDED_ID;
    tx_header.TxFrameType         = FDCAN_DATA_FRAME;
    tx_header.DataLength          = dlc;
    tx_header.ErrorStateIndicator = FDCAN_ESI_ACTIVE;
    tx_header.BitRateSwitch       = FDCAN_BRS_OFF;
    tx_header.FDFormat            = FDCAN_CLASSIC_CAN;
    tx_header.TxEventFifoControl  = FDCAN_NO_TX_EVENTS;
    tx_header.MessageMarker       = 0u;


    if(HAL_FDCAN_AddMessageToTxFifoQ(&hfdcan, &tx_header, msg.payload) != HAL_OK)
    {
        return CanStatus::NOT_OK;
    }
    return CanStatus::OK;
}

CanStatus CanSTM32G4XX::read(can_msg_t& msg)
{
    switch (read_method)
    {
    case STM32CanReadMethod::POLLING_BASED:
        return read_polling(msg);
    
    default:
        break;
    }
    return CanStatus::NOT_OK;
}


CanStatus CanSTM32G4XX::read_polling(can_msg_t& msg)
{
        // Poll RX FIFO 0 — nothing waiting is a normal, non-error condition
    if (HAL_FDCAN_GetRxFifoFillLevel(&hfdcan, FDCAN_RX_FIFO0) == 0u) {
        return CanStatus::RX_QUEUE_EMPTY;
    }

    FDCAN_RxHeaderTypeDef rx_header{};
    uint8_t rx_data[MAX_MESSAGE_PAYLOAD_SIZE_BYTES] = {0};

    // Pops one frame from the FIFO and copies the payload into rx_data
    if (HAL_FDCAN_GetRxMessage(&hfdcan, FDCAN_RX_FIFO0, &rx_header, rx_data) != HAL_OK) {
        return CanStatus::NOT_OK;
    }

    // DLC -> byte count
    uint8_t len = 0;
    if (!dlc_to_len(rx_header.DataLength, len)) {
        return CanStatus::NOT_OK;   
    }
    if (len > MAX_MESSAGE_PAYLOAD_SIZE_BYTES) {
        return CanStatus::NOT_OK;   
    }

    msg.id          = rx_header.Identifier & COMMS_CAN_EXT_ID_MASK;  // 29-bit ext id
    msg.payload_len = len;
    memcpy(msg.payload, rx_data, len);
    return CanStatus::OK;
}


// ---- DLC helpers For CAN ------------------------------------------------------------
// Theres probably a better spot for these
// Return false if len is not an exact CAN-FD data length; caller picks a valid one.
bool CanSTM32G4XX::len_to_dlc(const uint8_t len, uint32_t& dlc_out)
{
    switch (len) {
        case 0:  dlc_out = FDCAN_DLC_BYTES_0;  return true;
        case 1:  dlc_out = FDCAN_DLC_BYTES_1;  return true;
        case 2:  dlc_out = FDCAN_DLC_BYTES_2;  return true;
        case 3:  dlc_out = FDCAN_DLC_BYTES_3;  return true;
        case 4:  dlc_out = FDCAN_DLC_BYTES_4;  return true;
        case 5:  dlc_out = FDCAN_DLC_BYTES_5;  return true;
        case 6:  dlc_out = FDCAN_DLC_BYTES_6;  return true;
        case 7:  dlc_out = FDCAN_DLC_BYTES_7;  return true;
        case 8:  dlc_out = FDCAN_DLC_BYTES_8;  return true;
        case 12: dlc_out = FDCAN_DLC_BYTES_12; return true;
        case 16: dlc_out = FDCAN_DLC_BYTES_16; return true;
        case 20: dlc_out = FDCAN_DLC_BYTES_20; return true;
        case 24: dlc_out = FDCAN_DLC_BYTES_24; return true;
        case 32: dlc_out = FDCAN_DLC_BYTES_32; return true;
        case 48: dlc_out = FDCAN_DLC_BYTES_48; return true;
        case 64: dlc_out = FDCAN_DLC_BYTES_64; return true;
        default: return false;   // not a valid CAN-FD length
    }
}

bool CanSTM32G4XX::dlc_to_len(const uint32_t dlc, uint8_t& len_out)
{
    switch (dlc) {
        case FDCAN_DLC_BYTES_0:  len_out = 0u;  return true;
        case FDCAN_DLC_BYTES_1:  len_out = 1u;  return true;
        case FDCAN_DLC_BYTES_2:  len_out = 2u;  return true;
        case FDCAN_DLC_BYTES_3:  len_out = 3u;  return true;
        case FDCAN_DLC_BYTES_4:  len_out = 4u;  return true;
        case FDCAN_DLC_BYTES_5:  len_out = 5u;  return true;
        case FDCAN_DLC_BYTES_6:  len_out = 6u;  return true;
        case FDCAN_DLC_BYTES_7:  len_out = 7u;  return true;
        case FDCAN_DLC_BYTES_8:  len_out = 8u;  return true;
        case FDCAN_DLC_BYTES_12: len_out = 12u; return true;
        case FDCAN_DLC_BYTES_16: len_out = 16u; return true;
        case FDCAN_DLC_BYTES_20: len_out = 20u; return true;
        case FDCAN_DLC_BYTES_24: len_out = 24u; return true;
        case FDCAN_DLC_BYTES_32: len_out = 32u; return true;
        case FDCAN_DLC_BYTES_48: len_out = 48u; return true;
        case FDCAN_DLC_BYTES_64: len_out = 64u; return true;
        default: return false;   // unknown DLC
    }
}
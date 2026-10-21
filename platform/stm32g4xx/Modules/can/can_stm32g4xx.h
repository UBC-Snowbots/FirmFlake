#pragma once

#include <can_interface.h>
#include <stm32g4xx_hal.h>
#include <gpio_config_base.h>

typedef enum class STM32CanReadMethod
{
    POLLING_BASED,
    INTERRUPT_BASED,
    DMA
};


//TODO make configuring more up to the user
// Currently, this just organizes the code, but is fully based around one use. So configuration is just hardcoded
class CanSTM32G4XX : public CanInterface {
public:
    CanSTM32G4XX(FDCAN_HandleTypeDef& hfdcan, STM32CanReadMethod read_method = STM32CanReadMethod::POLLING_BASED) : 
    hfdcan(hfdcan), read_method(read_method) {};

    CanStatus init() override;
    CanStatus send(const can_msg_t& msg);
    CanStatus read(can_msg_t& msg);

private:
    FDCAN_HandleTypeDef& hfdcan;
    STM32CanReadMethod read_method;

    CanStatus read_polling(can_msg_t& msg);

    bool dlc_to_len(const uint32_t dlc, uint8_t& len_out);
    bool len_to_dlc(const uint8_t len, uint32_t& dlc_out);

};
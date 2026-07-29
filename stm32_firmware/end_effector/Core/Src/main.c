/* USER CODE BEGIN Header */
/*
  * Hardware connections
  *  FSR #1 : 3.3V → FSR → PA0 (ADC1_IN1) → 2kΩ → GND
  *  FSR #2 : 3.3V → FSR → PA2 (ADC1_IN3) → 2kΩ → GND
  *
  *  Needs to be 3.3V because of the limitations of the ADC pins
  *
  *  FDCAN1 via TJA1051T/3 : RX = PA11, TX = PA12
  *  USART3 via USB-TTL    : TX = PB10, RX = PB11  @ 115200 8N1
*/
/* USER CODE END Header */

#include "main.h"
#include "can_handler.h"
#include "fsr406.h"
#include <stdio.h>
#include <string.h>
#include <stdint.h>

ADC_HandleTypeDef   hadc1;
FDCAN_HandleTypeDef hfdcan1;
UART_HandleTypeDef  huart3;

#define SAMPLE_INTERVAL_MS    100    /* 10 Hz                              */
#define ADC_OVERSAMPLE        50     /* Average 50 readings per channel    */
#define ADC_MAX               4095U
#define UART_TX_BUF_SIZE      120

void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_FDCAN1_Init(void);
static void MX_USART3_UART_Init(void);
static void MX_ADC1_Init(void);

static uint16_t ADC_ReadChannel(uint32_t channel);
static uint16_t ADC_ReadOversampled(uint32_t channel, uint8_t n);
static void     UART_Print(const char *msg);
static void     UART_PrintStatus(uint16_t adc1, uint16_t adc2, uint32_t g1,   uint32_t g2);
static uint16_t ApplyTare(uint16_t raw, uint16_t offset);

/* Baseline offsets to subtract from sent force values */
static uint16_t tare_offset1 = 0;
static uint16_t tare_offset2 = 0;


/* Retarget printf to USART3 for debugging */
int __io_putchar(int ch)
{
    HAL_UART_Transmit(&huart3, (uint8_t *)&ch, 1, 10);
    return ch;
}


int main(void)
{
    HAL_Init();
    SystemClock_Config();

    MX_GPIO_Init();
    MX_FDCAN1_Init();
    MX_USART3_UART_Init();
    MX_ADC1_Init();

    /* Calibrate ADC */
    if (HAL_ADCEx_Calibration_Start(&hadc1, ADC_SINGLE_ENDED) != HAL_OK)
        Error_Handler();

    HAL_Delay(50);

    UART_Print("\r\n========================================\r\n");
    UART_Print("  FSR406 Force Sensor — STM32G4 / ADC1  \r\n");
    UART_Print("========================================\r\n");
    UART_Print("  FSR1 = PA0 (ADC1_IN1)  FSR2 = PA2 (ADC1_IN3)\r\n");
    UART_Print("  Pulldown = 2k ohm\r\n\r\n");

    {
        char info[64];
        snprintf(info, sizeof(info), "[INFO] RM = %lu ohm  VCC = %lu mV\r\n\r\n",
        		(unsigned long)FSR_RM_OHMS, (unsigned long)FSR_VCC_MV);
        UART_Print(info);
    }

    UART_Print("[INIT] FDCAN1... ");
    HAL_StatusTypeDef can_st = CAN_Handler_Init();
    UART_Print(can_st == HAL_OK ? "OK\r\n\r\n" : "FAIL\r\n\r\n");

    UART_Print("T(ms)   | FSR1 ADC | FSR1      | FSR2 ADC | FSR2\r\n");
    UART_Print("--------|----------|-----------|----------|----------\r\n");

    uint32_t last_sample = 0;
    uint16_t adc1_val = 0, adc2_val = 0;

    while (1)
    {
        uint32_t now = HAL_GetTick();

        /* If a tare has been requested over CAN, read + save the baseline tare values */
        if (g_can_flags.tare_requested)
        {
            g_can_flags.tare_requested = 0;

            tare_offset1 = ADC_ReadOversampled(ADC_CHANNEL_1, ADC_OVERSAMPLE);
            tare_offset2 = ADC_ReadOversampled(ADC_CHANNEL_3, ADC_OVERSAMPLE);

            char msg[64];
            snprintf(msg, sizeof(msg),
                     "[CAN] Tare set: off1=%u off2=%u\r\n",
                     (unsigned)tare_offset1, (unsigned)tare_offset2);
            UART_Print(msg);
        }

        /* On request, send the most recent saved values */
        if (g_can_flags.data_requested)
        {
            g_can_flags.data_requested = 0;
            if (can_st == HAL_OK)
                CAN_Transmit_LoadCells((int32_t)adc1_val, (int32_t)adc2_val);
            UART_Print("[CAN] Immediate transmit triggered.\r\n");
        }

        /* Do regular sampling */
        if ((now - last_sample) >= SAMPLE_INTERVAL_MS)
        {
            last_sample = now;

            uint16_t raw1 = ADC_ReadOversampled(ADC_CHANNEL_1, ADC_OVERSAMPLE);
            uint16_t raw2 = ADC_ReadOversampled(ADC_CHANNEL_3, ADC_OVERSAMPLE);

            adc1_val = ApplyTare(raw1, tare_offset1);
            adc2_val = ApplyTare(raw2, tare_offset2);

            uint32_t mn1 = FSR406_ADCtoMilliNewtons(adc1_val);
            uint32_t mn2 = FSR406_ADCtoMilliNewtons(adc2_val);
            uint32_t g1  = FSR406_MilliNewtonsToGrams(mn1);
            uint32_t g2  = FSR406_MilliNewtonsToGrams(mn2);

            if (can_st == HAL_OK)
                CAN_Transmit_LoadCells((int32_t)adc1_val, (int32_t)adc2_val);

            UART_PrintStatus(adc1_val, adc2_val, g1, g2);
        }
    }
}

/* Takes the baseline tare value off of a raw reading */
static uint16_t ApplyTare(uint16_t raw, uint16_t offset)
{
    return (raw > offset) ? (uint16_t)(raw - offset) : 0U;
}

/* Read from ADC */
static uint16_t ADC_ReadChannel(uint32_t channel)
{
    ADC_ChannelConfTypeDef cfg = {0};
    cfg.Channel      = channel;
    cfg.Rank         = ADC_REGULAR_RANK_1;
    cfg.SamplingTime = ADC_SAMPLETIME_47CYCLES_5;
    cfg.SingleDiff   = ADC_SINGLE_ENDED;
    cfg.OffsetNumber = ADC_OFFSET_NONE;
    cfg.Offset       = 0;

    if (HAL_ADC_ConfigChannel(&hadc1, &cfg) != HAL_OK) return 0;
    if (HAL_ADC_Start(&hadc1) != HAL_OK)               return 0;
    if (HAL_ADC_PollForConversion(&hadc1, 10) != HAL_OK)
    {
        HAL_ADC_Stop(&hadc1);
        return 0;
    }
    uint16_t val = (uint16_t)HAL_ADC_GetValue(&hadc1);
    HAL_ADC_Stop(&hadc1);
    return val;
}

/* Averages n readings from the ADC */
static uint16_t ADC_ReadOversampled(uint32_t channel, uint8_t n)
{
    uint32_t acc = 0;
    for (uint8_t i = 0; i < n; i++)
        acc += ADC_ReadChannel(channel);
    return (uint16_t)(acc / n);
}

/* Print to UART */
static void UART_Print(const char *msg)
{
    HAL_UART_Transmit(&huart3, (const uint8_t *)msg, (uint16_t)strlen(msg), 100);
}

/* Prints a given reading */
static void UART_PrintStatus(uint16_t adc1, uint16_t adc2,
                              uint32_t g1,   uint32_t g2)
{
    char buf[UART_TX_BUF_SIZE];
    char kg1[12], kg2[12];

    FSR406_FormatKg(g1, kg1, sizeof(kg1));
    FSR406_FormatKg(g2, kg2, sizeof(kg2));

    snprintf(buf, sizeof(buf),
             "%06lu  |     %4u | %10s |     %4u | %10s\r\n",
             (unsigned long)HAL_GetTick(),
             (unsigned)adc1, kg1,
             (unsigned)adc2, kg2);
    UART_Print(buf);
}


/* FDCAN RX callback */
void HAL_FDCAN_RxFifo0MsgPendingCallback(FDCAN_HandleTypeDef *hfdcan)
{
    CAN_Handler_RxCallback(hfdcan);
}

void SystemClock_Config(void)
{
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
    HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1);
    RCC_OscInitStruct.OscillatorType      = RCC_OSCILLATORTYPE_HSI;
    RCC_OscInitStruct.HSIState            = RCC_HSI_ON;
    RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
    RCC_OscInitStruct.PLL.PLLState        = RCC_PLL_NONE;
    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) Error_Handler();
    RCC_ClkInitStruct.ClockType      = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
                                     | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource   = RCC_SYSCLKSOURCE_HSI;
    RCC_ClkInitStruct.AHBCLKDivider  = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK) Error_Handler();
}

static void MX_ADC1_Init(void)
{
    ADC_MultiModeTypeDef multimode = {0};
    hadc1.Instance                   = ADC1;
    hadc1.Init.ClockPrescaler        = ADC_CLOCK_SYNC_PCLK_DIV2;
    hadc1.Init.Resolution            = ADC_RESOLUTION_12B;
    hadc1.Init.DataAlign             = ADC_DATAALIGN_RIGHT;
    hadc1.Init.GainCompensation      = 0;
    hadc1.Init.ScanConvMode          = ADC_SCAN_DISABLE;
    hadc1.Init.EOCSelection          = ADC_EOC_SINGLE_CONV;
    hadc1.Init.LowPowerAutoWait      = DISABLE;
    hadc1.Init.ContinuousConvMode    = DISABLE;
    hadc1.Init.NbrOfConversion       = 1;
    hadc1.Init.DiscontinuousConvMode = DISABLE;
    hadc1.Init.ExternalTrigConv      = ADC_SOFTWARE_START;
    hadc1.Init.ExternalTrigConvEdge  = ADC_EXTERNALTRIGCONVEDGE_NONE;
    hadc1.Init.DMAContinuousRequests = DISABLE;
    hadc1.Init.Overrun               = ADC_OVR_DATA_PRESERVED;
    hadc1.Init.OversamplingMode      = DISABLE;
    if (HAL_ADC_Init(&hadc1) != HAL_OK) Error_Handler();
    multimode.Mode = ADC_MODE_INDEPENDENT;
    if (HAL_ADCEx_MultiModeConfigChannel(&hadc1, &multimode) != HAL_OK) Error_Handler();
}
/* TQ = Prescaler / f_PCLK = 16 / 16 MHz = 1 µs
*  Bit time = TQ × (1 + TimeSeg1 + TimeSeg2) = 1 µs × 3 = 3 µs
*  Baud rate ≈ 333 kbps
*
*  If we need 500 kbps or 1 Mbps, adjust NominalPrescaler / TimeSeg1 / TimeSeg2
 */
static void MX_FDCAN1_Init(void)
{
    hfdcan1.Instance                  = FDCAN1;
    hfdcan1.Init.ClockDivider         = FDCAN_CLOCK_DIV1;
    hfdcan1.Init.FrameFormat          = FDCAN_FRAME_CLASSIC;
    hfdcan1.Init.Mode                 = FDCAN_MODE_NORMAL;
    hfdcan1.Init.AutoRetransmission   = DISABLE;
    hfdcan1.Init.TransmitPause        = DISABLE;
    hfdcan1.Init.ProtocolException    = DISABLE;
    hfdcan1.Init.NominalPrescaler     = 2;
    hfdcan1.Init.NominalSyncJumpWidth = 1;
    hfdcan1.Init.NominalTimeSeg1      = 13;
    hfdcan1.Init.NominalTimeSeg2      = 2;
    hfdcan1.Init.DataPrescaler        = 2;
    hfdcan1.Init.DataSyncJumpWidth    = 1;
    hfdcan1.Init.DataTimeSeg1         = 13;
    hfdcan1.Init.DataTimeSeg2         = 2;
    hfdcan1.Init.StdFiltersNbr        = 1;
    hfdcan1.Init.ExtFiltersNbr        = 0;
    hfdcan1.Init.TxFifoQueueMode      = FDCAN_TX_FIFO_OPERATION;
    if (HAL_FDCAN_Init(&hfdcan1) != HAL_OK) Error_Handler();
}

static void MX_USART3_UART_Init(void)
{
    huart3.Instance            = USART3;
    huart3.Init.BaudRate       = 115200;
    huart3.Init.WordLength     = UART_WORDLENGTH_8B;
    huart3.Init.StopBits       = UART_STOPBITS_1;
    huart3.Init.Parity         = UART_PARITY_NONE;
    huart3.Init.Mode           = UART_MODE_TX_RX;
    huart3.Init.HwFlowCtl      = UART_HWCONTROL_NONE;
    huart3.Init.OverSampling   = UART_OVERSAMPLING_16;
    huart3.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
    huart3.Init.ClockPrescaler = UART_PRESCALER_DIV1;
    huart3.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
    if (HAL_UART_Init(&huart3) != HAL_OK) Error_Handler();
    if (HAL_UARTEx_SetTxFifoThreshold(&huart3, UART_TXFIFO_THRESHOLD_1_8) != HAL_OK) Error_Handler();
    if (HAL_UARTEx_SetRxFifoThreshold(&huart3, UART_RXFIFO_THRESHOLD_1_8) != HAL_OK) Error_Handler();
    if (HAL_UARTEx_DisableFifoMode(&huart3) != HAL_OK) Error_Handler();
}

static void MX_GPIO_Init(void)
{
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
}

void Error_Handler(void)
{
    __disable_irq();
    while (1) {}
}

#ifdef USE_FULL_ASSERT
void assert_failed(uint8_t *file, uint32_t line) { (void)file; (void)line; }
#endif

#include "adc.h"
#include "main.h"
#include "fdcan.h"
#include "can_stm32g4xx.h"
#include <comms.h>
#include "stm32g4xx_hal.h"
#include "stm32g4xx_hal_gpio.h"

// ****** 24V Voltage/Current ADC ****** //
/* 24V voltage */
uint32_t adc_24v_voltage_raw = 0;
float adc_24v_voltage = 0.0f;
float voltage_24v = 0.0f;


/* Arm current */
uint32_t adc_arm_current_raw = 0;
float adc_arm_current_voltage = 0.0f;
float current_arm = 0.0f;


/* Chassis current */
uint32_t adc_chassis_current_raw = 0;
float adc_chassis_current_voltage = 0.0f;
float current_chassis = 0.0f;


/* Misc current */
uint32_t adc_misc_current_raw = 0;
float adc_misc_current_voltage = 0.0f;
float current_misc = 0.0f;
float vref_volts;
uint32_t vref_mv;

//[**********************24V Voltage/Current ADC****************************]//

uint32_t ADC1_Read_Channel(uint32_t channel);

uint32_t ADC2_Read_Channel(uint32_t channel);

float Calculate_24V_Voltage(float adc_voltage);

float Calculate_Current(float adc_voltage);

uint32_t ADC3_ReadVref_mV(void);

extern "C" void app_main(void)
{
    while (1) 
    {
        uint32_t vref_mv = ADC3_ReadVref_mV();

        if (vref_mv != 0)
        {
             vref_volts = vref_mv / 1000.0f;

            // Use vref_volts for other ADC calculations
        }

        // Read V_bat basically 
        adc_24v_voltage_raw = ADC1_Read_Channel(ADC_CHANNEL_2);
        adc_24v_voltage = (((float)adc_24v_voltage_raw * vref_volts) / 4095.0f);
        voltage_24v = Calculate_24V_Voltage(adc_24v_voltage);

        adc_arm_current_raw = ADC2_Read_Channel(ADC_CHANNEL_11);
	    adc_arm_current_voltage = ((float)adc_arm_current_raw / 4095.0f) * vref_volts;
	    current_arm = Calculate_Current(adc_arm_current_voltage);

        adc_chassis_current_raw = ADC2_Read_Channel(ADC_CHANNEL_15);
	    adc_chassis_current_voltage = ((float)adc_chassis_current_raw / 4095.0f) * vref_volts;
	    current_chassis = Calculate_Current(adc_chassis_current_voltage);
        
        adc_misc_current_raw = ADC2_Read_Channel(ADC_CHANNEL_13);
	    adc_misc_current_voltage = ((float)adc_misc_current_raw / 4095.0f) * vref_volts;
	    current_misc = Calculate_Current(adc_misc_current_voltage);

        HAL_GPIO_TogglePin(DEBUG_LED_GPIO_Port, DEBUG_LED_Pin);
        HAL_Delay(1000);
    }
    
}

/**
  * @brief Read a single ADC1 channel
  * @param channel ADC1 channel to read
  * @retval Raw 12-bit ADC value (0-4095)
  */
uint32_t ADC1_Read_Channel(uint32_t channel)
{
  ADC_ChannelConfTypeDef sConfig = {0};
  uint32_t adc_value = 0;

  /* Configure ADC1 channel */
  sConfig.Channel = channel;
  sConfig.Rank = ADC_REGULAR_RANK_1;
  sConfig.SamplingTime = ADC_SAMPLETIME_92CYCLES_5;
  sConfig.SingleDiff = ADC_SINGLE_ENDED;
  sConfig.OffsetNumber = ADC_OFFSET_NONE;
  sConfig.Offset = 0;

  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }

  /* Start conversion */
  if (HAL_ADC_Start(&hadc1) != HAL_OK)
  {
    Error_Handler();
  }

  /* Wait for conversion */
  if (HAL_ADC_PollForConversion(&hadc1, HAL_MAX_DELAY) != HAL_OK)
  {
    Error_Handler();
  }

  /* Read result */
  adc_value = HAL_ADC_GetValue(&hadc1);

  /* Stop ADC */
  if (HAL_ADC_Stop(&hadc1) != HAL_OK)
  {
    Error_Handler();
  }

  return adc_value;
}

/**
  * @brief Read a single ADC2 channel
  * @param channel ADC2 channel to read
  * @retval Raw 12-bit ADC value (0-4095)
  */
uint32_t ADC2_Read_Channel(uint32_t channel)
{
  ADC_ChannelConfTypeDef sConfig = {0};
  uint32_t adc_value = 0;

  /* Configure ADC2 channel */
  sConfig.Channel = channel;
  sConfig.Rank = ADC_REGULAR_RANK_1;
  sConfig.SamplingTime = ADC_SAMPLETIME_92CYCLES_5;
  sConfig.SingleDiff = ADC_SINGLE_ENDED;
  sConfig.OffsetNumber = ADC_OFFSET_NONE;
  sConfig.Offset = 0;

  if (HAL_ADC_ConfigChannel(&hadc2, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }

  /* Start conversion */
  if (HAL_ADC_Start(&hadc2) != HAL_OK)
  {
    Error_Handler();
  }

  /* Wait for conversion */
  if (HAL_ADC_PollForConversion(&hadc2, HAL_MAX_DELAY) != HAL_OK)
  {
    Error_Handler();
  }

  /* Read result */
  adc_value = HAL_ADC_GetValue(&hadc2);

  /* Stop ADC */
  if (HAL_ADC_Stop(&hadc2) != HAL_OK)
  {
    Error_Handler();
  }

  return adc_value;
}

/**
  * @brief Calculate actual 24V rail voltage from ADC voltage
  * @param adc_voltage Voltage measured at PA1
  * @retval Actual 24V rail voltage
  *
  * Voltage divider:
  *
  * R1 = 18k
  * R2 = 2k
  *
  * Vout = Vin * R2 / (R1 + R2)
  *
  * Vin = Vout * (R1 + R2) / R2
  *
  * Vin = Vout * (18k + 2k) / 2k
  *
  * Vin = Vout * 10
  */
float Calculate_24V_Voltage(float adc_voltage)
{
  return adc_voltage * 10.0f;
}

/**
  * @brief Calculate current from TMCS1123 output voltage
  * @param adc_voltage Hall sensor output voltage
  * @retval Measured current in Amps
  *
  * TMCS1123B1:
  *
  * Zero-current output = 1.65V
  * Sensitivity = 25mV/A = 0.025V/A
  *
  * I = (Vout - 1.65V) / 0.025V/A
  */
float Calculate_Current(float adc_voltage)
{
  const float sensor_zero_voltage = 1.65f;
  const float sensor_sensitivity = 0.025f;

  return (adc_voltage - sensor_zero_voltage) / sensor_sensitivity;
}

#define VREFINT_SAMPLE_COUNT 32U

uint32_t ADC3_ReadVref_mV(void)
{
    uint32_t sum = 0;

    for (uint32_t i = 0; i < VREFINT_SAMPLE_COUNT; i++)
    {
        if (HAL_ADC_Start(&hadc3) != HAL_OK)
            return 0;

        if (HAL_ADC_PollForConversion(&hadc3, 10) != HAL_OK)
        {
            HAL_ADC_Stop(&hadc3);
            return 0;
        }

        sum += HAL_ADC_GetValue(&hadc3);

        HAL_ADC_Stop(&hadc3);
    }

    uint32_t raw = sum / VREFINT_SAMPLE_COUNT;

    if (raw == 0)
        return 0;

    return __HAL_ADC_CALC_VREFANALOG_VOLTAGE(
        raw,
        ADC_RESOLUTION_12B
    );
}
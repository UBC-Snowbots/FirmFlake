// Author: Alan Cheng 
#include "adc.h"
#include "main.h"
#include "fdcan.h"
#include "can_stm32g4xx.h"
#include "stm32g4xx_hal_adc_ex.h"
#include <comms.h>
#include <primitive_io_stm32g4xx.hpp>
#include <timer.hpp>
#include <clock_stm32g4xx.hpp>

constexpr float ADC_RESOLUTION_FACTOR = ((1UL << 16) - 1);


// Read Reference Voltage 
/**  TODO: COnvert to DMA based architecture, meaning that whenever I run voltage conversion it'll convert to the latest voltage 
**/

/* Reference Voltage (VDD)*/
float vref_volts;
uint32_t vref_mv;
uint32_t ADC3_ReadVref_mV(void);

/* ADC Re-calibration*/
float previous_calibration_time;
float current_time;

ClockSTM32G4XX clock;
Timer minute_timer(clock, true);
Timer second_timer(clock, true);

// TODO: move this thingy to some other thing, better software practice i think 
struct ADC_Data {
  uint32_t adc_raw;
  float adc_voltage;
  float data;
  /** 
  * @brief Convert raw readings to voltage 
  * @retval Sets adc_voltage to the converted adc_raw 
  **/
  void Convert_ADC_Voltage() {
    uint32_t vref_mv = ADC3_ReadVref_mV();
    if (vref_mv != 0) vref_volts = vref_mv / 1000.0f;
    adc_voltage = (((float)adc_raw * vref_volts) / ADC_RESOLUTION_FACTOR);
  } 

  /**
  * @brief Calculate actual 24V rail voltage from ADC voltage
  * @retval Actual 24V rail voltage
  *
  * Voltage divider:
  * Vin = Vout * (18k + 2k) / 2k
  * Vin = Vout * 10
  */
  void Calculate_24V_Voltage()
  {
    data = adc_voltage * 10.0f;
  }

  /**
    * @brief Calculate current from TMCS1123 output voltage
    * @param adc_voltage Hall sensor output voltage
    * @retval Measured current in Amps
    * TMCS1123B1:
    * Zero-current output = 1.65V
    * Sensitivity = 25mV/A = 0.025V/A
    * I = (Vout - 1.65V) / 0.025V/A
    */
  void Calculate_Current() {
    const float sensor_zero_voltage = 1.65f;
    const float sensor_sensitivity = 0.025f;
    data = (adc_voltage - sensor_zero_voltage) / sensor_sensitivity;
  }
};


extern "C" void app_main(void)
{
  /* ADC Init */
  ADC_Data v_bat = {0};
  ADC_Data arm_current = {0};
  ADC_Data chassis_current = {0};
  ADC_Data misc_current = {0};
  
  // ADC Definitions
  PrimitiveAnalogInputSTM32G4XX adc_arm(
    ARM_SENSE_GPIO_Port, 
    ARM_SENSE_Pin, 
    ADC2, 
    ADC_CHANNEL_11,
    &hadc2.Init
  );

  PrimitiveAnalogInputSTM32G4XX adc_vbat(
    SENSE_24V_GPIO_Port, 
    SENSE_24V_Pin, 
    ADC1, 
    ADC_CHANNEL_2,
    &hadc1.Init
    
  );

  PrimitiveAnalogInputSTM32G4XX adc_chassis(
    CHASSIS_SENSE_GPIO_Port, 
    CHASSIS_SENSE_Pin, 
    ADC2, 
    ADC_CHANNEL_15,
    &hadc2.Init
  );

  PrimitiveAnalogInputSTM32G4XX adc_misc(
    MISC_SENSE_GPIO_Port, 
    MISC_SENSE_Pin, 
    ADC2, 
    ADC_CHANNEL_13,
    &hadc2.Init
  );
  // idt PrimitiveAnalogInput.init() changes CubeMX's init settings, safe to run?
  adc_vbat.init();
  adc_arm.init();
  adc_chassis.init();
  adc_misc.init();
  HAL_ADCEx_Calibration_Start(&hadc1, ADC_SINGLE_ENDED);
  HAL_ADCEx_Calibration_Start(&hadc2, ADC_SINGLE_ENDED);
  HAL_ADCEx_Calibration_Start(&hadc3, ADC_SINGLE_ENDED);
  
  // Setting timers 
  if (!minute_timer.init()) Error_Handler();
  minute_timer.set(60'000);

  if (!second_timer.init()) Error_Handler();
  second_timer.set(1000);

  while (1) 
  {
    // Run re-calibration every minute or so
    // This is to prevent temperature drift from affecting readings
    if (minute_timer.expired()) {
      HAL_ADCEx_Calibration_Start(&hadc1, ADC_SINGLE_ENDED);
      HAL_ADCEx_Calibration_Start(&hadc2, ADC_SINGLE_ENDED);
      HAL_ADCEx_Calibration_Start(&hadc3, ADC_SINGLE_ENDED);
    }
    
    // Heartbeat? for PDB Debug Light 
    if (second_timer.expired()) {
      HAL_GPIO_TogglePin(DEBUG_LED_GPIO_Port, DEBUG_LED_Pin);
    }

    // Read V_Bat 
    adc_vbat.read(v_bat.adc_raw);
    v_bat.Convert_ADC_Voltage();
    v_bat.Calculate_24V_Voltage();

    // Read Arm Current 
    adc_arm.read(arm_current.adc_raw);
    arm_current.Convert_ADC_Voltage();
    arm_current.Calculate_Current();

    // Read Chassis Current 
    adc_chassis.read(chassis_current.adc_raw);
    chassis_current.Convert_ADC_Voltage();
    chassis_current.Calculate_Current();
    
    // Read misc. current 
    adc_misc.read(misc_current.adc_raw);
    misc_current.Convert_ADC_Voltage();
    misc_current.Calculate_Current();
  }
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
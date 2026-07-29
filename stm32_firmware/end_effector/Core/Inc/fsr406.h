#ifndef FSR406_H
#define FSR406_H

#include <stdint.h>

/* Voltage divider:  3.3V → FSR → ADC pin → RM → GND */

#define FSR_VCC_MV        3300U   /* Supply voltage in millivolts          */
#define FSR_ADC_BITS      12U     /* ADC resolution                        */
#define FSR_ADC_MAX       4095U   /* 2^12 - 1                              */
#define FSR_RM_OHMS       2000U  /* 2kΩ   */

/* Converts a 12-bit ADC reading to force in milliNewtons.
 * Returns 0 if no contact (ADC too low to be meaningful).
 * Uses only integer arithmetic.
 *
 * @param  adc_raw   12-bit ADC count (0–4095)
 * @return Force in milliNewtons (divide by 1000 for N, by 101.97 for kg)*/
uint32_t FSR406_ADCtoMilliNewtons(uint16_t adc_raw);

/* Converts milliNewtons to grams-force (1 N = 101.97 gf ≈ 102 gf).
 * Returns value in grams. */
uint32_t FSR406_MilliNewtonsToGrams(uint32_t mn);

/* Writes "X.XXX kg" into buf (needs at least 12 bytes) */
void FSR406_FormatKg(uint32_t grams, char *buf, int buf_len);


#endif /* FSR406_H */

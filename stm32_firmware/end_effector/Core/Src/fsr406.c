#include "fsr406.h"
#include <string.h>
#include <stdio.h>

/* Converts a 12-bit ADC reading to force in milliNewtons.
 * Returns 0 if no contact
 *
 * @param  adc_raw   12-bit ADC count (0–4095)
 * @return Force in milliNewtons */
uint32_t FSR406_ADCtoMilliNewtons(uint16_t adc_raw)
{
    /* Below this the FSR is essentially open */
    if (adc_raw < 10U) return 0U;

    /* ADC to V_out in mV */
    uint32_t vout_mv = ((uint32_t)adc_raw * FSR_VCC_MV) / FSR_ADC_MAX;

    /* If V_out >= VCC something is wrong */
    if (vout_mv >= FSR_VCC_MV) return 0U;

    /* Get voltage drop across the fsr */
    uint32_t vdrop_mv = FSR_VCC_MV - vout_mv;

    /* R_fsr = RM × vdrop / vout */
    if (vout_mv == 0U) return 0U;
    uint32_t r_fsr_ohms;
    uint64_t r_num = (uint64_t)FSR_RM_OHMS * (uint64_t)vdrop_mv;
    r_fsr_ohms = (uint32_t)(r_num / vout_mv);
    /* > 10 MΩ is no contact  */
    if (r_fsr_ohms > 10000000UL) return 0U;
    /* Avoid divison by zero  */
    if (r_fsr_ohms == 0U) r_fsr_ohms = 1U;

    /* Conductance = 1e6 / R_ohms in µS */
    uint32_t cond_uS_x100 = (uint32_t)(100000000UL / r_fsr_ohms);

    /* Force_mN = cond_uS_x100 / 100 gives µS            */
    uint32_t force_mn = (cond_uS_x100) / 100U;

    return force_mn;
}

/* Converts milliNewtons to grams-force (1 N = 101.97 gf ≈ 102 gf).
 * Returns value in grams. */
uint32_t FSR406_MilliNewtonsToGrams(uint32_t mn)
{
    return (mn * 102UL) / 1000UL;
}

/* Writes "X.XXX kg" into buf (needs at least 12 bytes) */
void FSR406_FormatKg(uint32_t grams, char *buf, int buf_len)
{
    uint32_t kg_whole = grams / 1000U;
    uint32_t kg_frac  = grams % 1000U;
    snprintf(buf, (size_t)buf_len, "%lu.%03lu kg", (unsigned long)kg_whole, (unsigned long)kg_frac);
}


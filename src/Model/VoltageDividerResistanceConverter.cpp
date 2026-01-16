#include "Model/VoltageDividerResistanceConverter.h"


/**
 * @brief Constructor for the Voltage Divider sensing circuit conversion from ADC raw to Resistance
 * 
 * @param pullup_Ohms - Fixed resistance connected to V_REF
 */
VoltageDividerResistanceConverter::VoltageDividerResistanceConverter(uint16_t pullup_Ohms):
fixedResistor_(pullup_Ohms == 0 ? Sensors::PULLUP_FIXED_RESISTOR_OHMS : pullup_Ohms),
initialize_(false)
{}


/**
 * @brief Final validation: Fixed resistance validation
 * 
 * @note call  this after Serial.begin() in setup() 
 * 
 */
void VoltageDividerResistanceConverter::begin()
{
    // Skip if already initialize
    if(initialize_)return;

    // Validate input pullup resistor value
    if (fixedResistor_ == 0)
    LOGE("Invalid pullup fixed resistor value: 0Ohms- setting to default  %u" , Sensors::PULLUP_FIXED_RESISTOR_OHMS);
}

/**
 * @brief Converts the adc_raw value to a NTC resistance value scaled by 10 for 0.1Ω  precision
 * 
 * @details
 * Assuming the default voltage divider sensing circuit: 5V → 12.7 kΩ pullup(default) → junction (ADC reads here) → NTC to GND.
 *  
 * How we compute the R_NTC?
 * 
 *   1. V_junction = V_REF * (R_NTC/(R_NTC + pullup_Ohms));
 *   2. For a 10-bit ADC: 
 *       V_junction = V_REF * (adc_raw/1023);
 *   3. If we equalize both formulas and resolve R_NTC: 
 *       R_NTC = (adc_raw * pullup_Ohms)/(1023-adc_raw)
 *   4. Finally scales by 10:
 *      - R_NTC_x10 = (ADC_raw * pullup_ohms * 10) / (ADC_max - ADC_raw)
 * 
 * @param adc_raw - Raw ADC counts(0 - 1023 for 10bits resolution)
 * @return uint32_t - Resistance in 0.1Ω resolution (x10)
 */
uint32_t VoltageDividerResistanceConverter::convertToResistance_x10(uint16_t adc_raw)
{
    // Step1: Validate adc_raw
    if(adc_raw == 0 || adc_raw > Adc::MAX_VALUE)
    {
        LOGD("VoltageDividerResistanceConverter:: Invalid ADC raw value");
        return 0;
    }

    // Step2: Apply voltage divider formula to compute NTC resistance scaled by 10
    return (static_cast<uint32_t>(adc_raw) * fixedResistor_ * 10) / (Adc::MAX_VALUE - adc_raw);
}
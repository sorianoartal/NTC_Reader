#pragma once

#include <stdint.h>

#include "interfaces/IResistanceConverter.h"    // Abstarc interface
#include "logger/Logger.h"                      // For debugging
#include "config/Config.h"                      // Centralize configuration params


/**
 * @brief Converts ADC raw readings values to Thermistor resistance values using the voltage divider formula
 * 
 * @details 
 *  * For the typical voltage divider : 5V → 12.7 kΩ pullup(default) → junction (ADC reads here) → NTC to GND.
 *      - Voltage divider formula:
 *          V_junction(raw Adc) = V_REF * (adc_raw/1023);
 *      - This class use counts for efficiency and scaled by 10 for precision:
 *          R_NTC = (adc_raw * pullup_Ohms * 10)/(1023 - adc_raw);  
 * 
 * @example
 *  // Example:
 *  static VoltageDividerResistanceConverter res_converter;  // Defaults to Config value
 *   uint32_t r_x10 = res_converter.ConvertToResistance_x10(adc_raw);
 */
class VoltageDividerResistanceConverter : public IResistanceConverter
{
    public:

        /// @brief Constructor for the Voltage Divider sensing circuit conversion from ADC raw to Resistance
        /// @param pullup_Ohms - Fixed resistance connected to V_REF
        VoltageDividerResistanceConverter(uint16_t pullup_Ohms = Sensors::PULLUP_FIXED_RESISTOR_OHMS);

        /// @brief Final Initialization: fixed resistor validation
        void begin();

        // --- Implemented methods from IResistanceConverter interface ---

        /// @brief Convert to a resistance value scaled by 10 for 0.1Ω resolution 
        /// @param adc_raw 
        /// @return 
        uint32_t convertToResistance_x10(uint16_t adc_raw) override;

    private:

        const uint16_t fixedResistor_;  // Pullup Fixed resistor in series with the NTC for the Voltage Divider sensing circuit 

        bool initialize_;               // To avoid reinitialization on a VoltageDividerResistance instance is already initialize
};
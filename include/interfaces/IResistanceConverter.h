#pragma once

#include<stdint.h>

/**
 * @brief Abstract interface to convert Adc raw readings into a resistance value
 * 
 */
class IResistanceConverter
{
    public:

        /// @brief Default destructor to ensure proper cleanup
        virtual ~IResistanceConverter() = default;

        /// @brief Convert ADC raw values into Resistance scales by 10 for 0.1Ω resolution (x10)
        /// @param adc_raw - Raw ADC counts(0 - 1023 for 10bits resolution) 
        /// @return Resistance in 0.1Ω resolution (x10)
        virtual uint32_t convertToResistance_x10(uint16_t adc_raw) = 0;
};
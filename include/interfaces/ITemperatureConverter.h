#pragma once

#include<stdint.h>

/**
 * @brief Abstract interface for Temperature Converter implementations
 * 
 * @details
 * Classes that implement this interface should return the temperature value scaled by 10 (0.1°C resolution)
 *  - Example: 
 *      - 100 means 10.0°C
 *      - 40 means -4.0°C
 *     - -32768 means Sentinel value for error / out of range
 */
class ITemperatureConverter 
{
    public:

        /// @brief Default destructor to ensure proper cleanup
        virtual ~ITemperatureConverter() = default;

        /// @brief Convert Resistance value in 0.1Ω resolution (x10) into Temperature(fixed point) scaled by 10 for 0.1°C resolution(e.g, 10000 means 1000.0Ω)
        /// @param resistance_x10 - Resistance in 0.1Ω resolution (x10)
        /// @return Temperature in Celsius in a max. range -40.0°C to +40.0°C scaled by 10 (0.1°C resolution). Use int16_t to cover -400 to +400 range
        virtual int16_t convertToTemperature_x10(uint32_t resistance_x10) const noexcept = 0;
};
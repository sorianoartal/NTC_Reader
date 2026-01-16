#pragma once

#include <stdint.h>                             // For standard integer types
#include "interfaces/ITemperatureConverter.h"   // For ITemperatureConverter interface
#include "config/Config.h"                      // For sensor configuration constants
#include "data/thermistor_lut.h"                // For thermistor lookup table
#include "utils/avr_algorithms.h"               // For AVR specific algorithms
#include "logger/Logger.h"                      // For debugging
#include "data/lut_utils.h"                     // For LUT utilities

/**
 * @brief Concrete temperature converter using a lookup table(LUT) for NTC thermistor + linear interpolation
 * 
 * @details
 *  This class implements the ITemperatureConverter interface to convert resistance values from an NTC thermistor:
 *  - Uses a predefined LUT of sorted  by "decreasing" resistance of ThermistorEntry structs {resistance_x10, temperature_x10}
 *  - Performs  a binary search to find the two surrounding entries bracketing the input resistance
 *  - Once the bracketing entries are found, it applies linear interpolation to estimate the temperature
 *    corresponding to the measured resistance.
 */
class LutTemperatureConverter: public ITemperatureConverter
{
private:
    /* data */
public:
    
    /// @brief Constructor
    LutTemperatureConverter(/* args */);
    
    ~LutTemperatureConverter() = default;

    /// @brief Final initialization
    void begin();

    /**
     * @brief Convert resistance to temperature using LUT and linear interpolation
     * 
     * @param resistance_x10 Resistance in tenths of Ohms (e.g., 10000 = 1000.0 Ohms)
     * @return int32_t Temperature in tenths of degrees Celsius (e.g., 250 = 25.0 °C)
     */
    int16_t convertToTemperature_x10(uint32_t resistance_x10) const noexcept override;

private:
    bool initialize_;
};

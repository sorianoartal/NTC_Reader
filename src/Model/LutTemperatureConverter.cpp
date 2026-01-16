#include "Model/LutTemperatureConverter.h"

/**
 * @brief Construct a new Lut Temperature Converter:: Lut Temperature Converter object
 * 
 */
LutTemperatureConverter::LutTemperatureConverter():
initialize_(false)
{}


/**
 * @brief Final initialization
 * 
 */
void LutTemperatureConverter::begin()
{
    if(initialize_)return;

    LOGD("LutTemperatureConverter:: Initializing...");
}

/**
* @brief Convert resistance to temperature using LUT and linear interpolation
*   What it does?
*   1. Performs a binary search on the NTC_LUT to find the two entries that bracket the input resistance.
*   2. If an exact match is found, it returns the corresponding temperature directly.
*   3. If no exact match is found, it uses linear interpolation between the two bracketing entries to estimate the temperature.
*
* @details  
* - Bracketing: The two entries that bracket the input resistance are identified.
* - Clamping: If the input resistance is outside the LUT range, it clamps to the nearest valid temperature.
* - Linear Interpolation: The temperature is estimated using the formula:
*      T_measured = T_cold + (T_hot - T_cold) * ((R_cold - R_measured) / (R_cold - R_hot))
*   where:
*   R_cold = higher resistance (colder temperature point)
*   R_hot = lower resistance (hotter temperature point)
*   T_cold = lower temperature
*   T_hot = higher temperature
*   R_measured = input resistance

* - noexcept: Pure computation- no failure possible  
* @param resistance_x10 Resistance in tenths of Ohms (e.g., 10000 = 1000.0 Ohms)
* @return int32_t Temperature in tenths of degrees Celsius (e.g., 250 = 25.0 °C)
*/
int16_t LutTemperatureConverter::convertToTemperature_x10(uint32_t resistance_x10) const noexcept
{
    //Validate input resistance 
    if(resistance_x10 == 0)
    {
        LOGE("LutTemperatureConverter::convertToTemperature_x10: Invalid resistance value 0");
        return -32768; // Sentinel error code
    }

    // Step 1: Perform binary search to find bracketing entries
    using namespace lut_utils;
    LutBracket bracket = binarySearchLut(
        NTC_LUT,                                                            /* lookup table */
        resistance_x10,                                                     /* resistance value */
        [](const ThermistorEntry& entry) { return entry.resistance_x10; },  /* Projection function to retrieve the key */
        LutOrder::DECREASING                                                /* order */
    );

    // Step2: Handle bracketing results Edge cases
    if(bracket.outOfRange)
    {
        LOGD("LutTemperatureConverter:: Resistance %u is out of LUT range",resistance_x10);
        
        // Clamp to nearest valid temperature
        if(bracket.clamped)
        {
            if(resistance_x10 > NTC_LUT[0].resistance_x10)
            {
                // Above max resistance (colder than -40.0°C)
                return NTC_LUT[0].temperature_x10;
            }
            else
            {
                // Below min resistance (hotter than +40.0°C)
                return NTC_LUT[NTC_LUT_SIZE - 1].temperature_x10;
            }
        }

        // Should not reach here
        LOGE("LutTemperatureConverter:: Unexpected outOfRange state");
        return -32768; // Sentinel error code
    }

    // Exact found
    if(bracket.foundExact)
    {
        LOGD("LutTemperatureConverter:: Exact match found at index %zu", bracket.exactIdx);
        return NTC_LUT[bracket.exactIdx].temperature_x10;
    }

    // Step3: Perform linear interpolation between the two bracketing entries
    const ThermistorEntry& cold = NTC_LUT[bracket.lowerIdx];
    const ThermistorEntry& hot = NTC_LUT[bracket.upperIdx];

    LOGD("LutTemperatureConverter:: Applying linear interpolation for Resistance %lu between [%d Ω @ %d °C] and [%d Ω @ %d °C]",
        (unsigned long)resistance_x10,
        (int)cold.resistance_x10, (int)cold.temperature_x10,
        (int)hot.resistance_x10, (int)hot.temperature_x10
    );

    return applyLinearInterpolation(
        resistance_x10,
        cold.resistance_x10,
        hot.resistance_x10,
        cold.temperature_x10,
        hot.temperature_x10
    );
}
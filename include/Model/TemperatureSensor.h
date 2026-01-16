#pragma once

#include <stdint.h>                             // For standard integer types

#include "interfaces/ISampler.h"                // For ISampler interface -> To take ADC measurements
#include "interfaces/IResistanceConverter.h"    // For Voltage divider ADC-> Resistance
#include "interfaces/ITemperatureConverter.h"   // For ITemperatureConverter interface Resistance->Temperature
#include "interfaces/IFilter.h"                 // For IFilter interface -> To filter temperature readings
#include "logger/Logger.h"                      // For logging

/**
 * @brief Enum class to represent temperature units
 * 
 */
enum class TemperatureUnit : uint8_t
{
    Celsius,
    Fahrenheit,
    Kelvin
};

/**
 * @brief High-level NTC temperature sensor composer with fluent configuration API
 * 
 * @details
 *  Orchestrate the full pipeline from ADC sampling to temperature output:
 *      Sampler-> ResistanceConverter->TemperatureConverter->(optional)Filter
 * 
 * Provides a fluent API to configure each component step-by-step.
 * Example usage:
 * TemperatureSensor sensor;
 * sensor.addSampler(&sampler)
 *       .addResistanceConverter(&resistanceConverter)
 *       .addTemperatureConverter(&temperatureConverter)
 *       .addFilter(&filter)
 *       .setUnits(TemperatureUnit::Celsius)
 *       .build();
 * 
 * int16_t temperature = sensor.readTemperature_x10();   
 */
class TemperatureSensor
{
    public:

        /// @brief Default constructor
        TemperatureSensor();

        /// @brief Default destructor to ensure proper cleanup
        ~TemperatureSensor() = default;

        // Fluent API methods to configure each component
        TemperatureSensor& addSampler(ISampler* sampler);
        TemperatureSensor& addResistanceConverter(IResistanceConverter* converter);
        TemperatureSensor& addTemperatureConverter(ITemperatureConverter* converter);
        TemperatureSensor& addFilter(IFilter<int16_t>* filter);
        TemperatureSensor& setUnits(TemperatureUnit unit);
        TemperatureSensor& build();

        // Method to read the temperature in tenths of degrees
        int16_t readTemperature_x10() const noexcept;

        // --- Helper methods to read temperature in different units ---
        float readTemperature() const noexcept;
        float readTemperatureC()  noexcept;
        float readTemperatureF()  noexcept;
        float readTemperatureK()  noexcept;

    private:     
        
        ISampler* sampler_ ;                                // Pointer to a Sampler object that knows how to sample ADC values
        IResistanceConverter* resistanceConverter_ ;        // Pointer to a ResistanceConverter object that converts ADC values to resistance
        ITemperatureConverter* temperatureConverter_ ;      // Pointer to a TemperatureConverter object that converts resistance to temperature
        IFilter<int16_t>* filter_ ;                         // Pointer to a Filter<T> that apply(EMA,SMA) filter the read temperature values 
        TemperatureUnit unit_;                              // Desired output temperature unit 
};
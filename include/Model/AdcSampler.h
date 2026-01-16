#pragma once

#include<stdint.h>

#include "config/Config.h"
#include "interfaces/ISampler.h"
#include "logger/Logger.h"
#include "utils/avr_algorithms.h"

/**
 * @brief ADC Sampler concrete implementation
 * 
 * @details
 *  what this class does?
 *  - Implement the ISampler interface
 *  - Samples the Analog pin specify via constructor.
 *  - Applies a setting delay to stabilize the signal.
 *  - Discard the N first readings to flush artifact.
 *  - Average multiple ADC reading to reduce noise.
 *  
 */
class AdcSampler : public ISampler
{
    public:

        /// @brief Configurable: pin, number of samples to average, samples to discard (for settling) and settle time     
        AdcSampler(uint8_t adc_pin, uint8_t samples_to_average, uint8_t samples_to_discard, uint8_t settle_us);

        
        /// @brief Final initialization: validation, pin setup.
        void begin();

        // === Implemented method from ISampler interface ===
        
        /// @brief Samples the specify Analog Pin 
        /// @return ADC raw count value 
        uint16_t sample() override;

    private:

        const uint8_t pin_;                 // Analog pin to read
        const uint8_t samples_per_read_;     //K consecutive ADC reads for averaging, use power of 2 for fast division
        const uint8_t discard_N_first_;     // Discard the N first sample reading       
        const uint8_t settleUs_;            // Microseconds delay after each each for stability

        bool initialize_;                   // To avoid re-configuration
};
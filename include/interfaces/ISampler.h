#pragma once

#include <stdint.h>

/// @brief Abstract sampler interface for the ADC
class ISampler
{
public:

    virtual ~ISampler() = default;

    /**
     * @brief The concrete implementation of the sampler() should do:
     *  - Discard first N samples
     *  - Accumulates M consecutive samples.
     *  - Average them 
     * 
     * @return uint16_t - The average/filter raw ADC value 
     */
    virtual uint16_t sample() = 0;
};
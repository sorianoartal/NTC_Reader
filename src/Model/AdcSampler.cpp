#include "Model/AdcSampler.h"

/**
 * @brief Construct a new Adc Sampler:: Adc Sampler object
 * 
 * @param adc_pin - Adc pin to sample
 * @param samples_to_average - Number of sample to average
 * @param samples_to_discard - N first sample to discard
 * @param settle_us - delay between sample to stabilization
 */
AdcSampler::AdcSampler(uint8_t adc_pin, uint8_t samples_to_average, uint8_t samples_to_discard, uint8_t settle_us):
pin_(adc_pin),
samples_per_read_((samples_to_average > 0) ? samples_to_average : 1),
discard_N_first_(samples_to_discard),
settleUs_((settle_us > 0) ? settle_us : 10),
initialize_(false)
{
}

/**
 * @brief Final initialization: validation, pin setup
 * 
 * @note Call this after Serial.begin() in setup() to avoid side effects
 */
void AdcSampler::begin()
{
    // Check if instance is already initialize
    if(initialize_) return;

    // Analog pin validation  
    if( pin_ > NUM_ANALOG_INPUTS )
     LOGE("AdcSampler:: Invalid ADC pin: %d", pin_);
    
    // Overflow protection
    if (samples_per_read_ > 64)
     LOGW("AdcSampler:: samples_per_read_ overflow: %d", samples_per_read_);
    
    // Configure ADC pin
    pinMode(pin_, INPUT);
}

/**
 * @brief This method implements the full measurement pipeline for a single ADC pin
 * 
 * @details
 * - Validates the channel index.
 * - Selects the channel on the multiplexer.
 * - Applies a settling delay to stabilize the signal (Arduino-specific).
 * - Discards initial readings to flush artifacts.
 * - Averages multiple ADC samples to reduce noise.
 * 
 * @return uint16_t raw average ADC value
 */
uint16_t AdcSampler::sample()
{
    // to store the accumulated raw values read
    uint32_t accumulated = 0;

    // Step1: Discard the N first readings
    avr_algorithms::repeat(discard_N_first_,[&](void){
        analogRead(pin_);
        if(settleUs_ > 0) delayMicroseconds(settleUs_);
    });

    // Step2: Read the Accumulated value of N consecutive samples
    avr_algorithms::repeat(samples_per_read_,[&](void){
        accumulated += analogRead(pin_);
        if(settleUs_ > 0) delayMicroseconds(settleUs_);
    });

    // Step3: average the accumulated samples (with rounding for non-power of 2)   
    uint16_t avg = (samples_per_read_ == 1)
    ? static_cast<uint16_t>(accumulated)
    : static_cast<uint16_t>((accumulated + (samples_per_read_ >> 1)) / samples_per_read_); 

    LOGD("AdcSampler:: ADC pin %d: raw avg = %d", pin_,avg);

    // Step4: Return the average value(Clamp if avg> 1023 max resolution) 
    return (avg > Adc::MAX_VALUE) ? Adc::MAX_VALUE : avg;
}

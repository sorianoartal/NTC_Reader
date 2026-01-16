#pragma once

#include <stdint.h>
#include <Arduino.h>

// =================================================================
//                  Hardware Sensor Configuration
//  All parameter are tunable / Hardware dependent values goes here
// =================================================================

namespace Pins
{
    constexpr uint8_t EVAPORATOR_NTC_ADC_PIN  = A0; // Analog pin for the evaporator temperature sensor.
    constexpr uint8_t COMPARTMENT_NTC_ADC_PIN = A1; // Analog pin for the fridge compartment temperature sensor.
}

namespace Adc
{
    constexpr uint8_t  BIT_RESOLUTION = 10;                     // Arduino NAno default: 10-bit 
    constexpr uint16_t MAX_VALUE = (1 << BIT_RESOLUTION)-1;     // 1023
    constexpr float    V_REF_VOLTS = 5.0f;                      // Default AREF = 5.0V. Change to 3.3V if needed 
    constexpr uint8_t  SAMPLES_TO_AVERAGE = 16;                 // Number of ADC samples to average per reading (power of 2 for fast division)
    constexpr uint8_t  SAMPLES_TO_DISCARD = 4;                  // Number of initial samples to discard for signal settling
    constexpr uint8_t  SETTLE_TIME_US = 50;                     // Microseconds
}

namespace Sensors
{
    // Sensing input circuit voltage divider PULLUP resistance(Use whatever your circuit has)
    constexpr uint16_t PULLUP_FIXED_RESISTOR_OHMS = 12700;  // 12.7K in series with the NTC

    // NTC thermistor Model
    constexpr int8_t LUT_TEMPERATURE_MIN_C  = -40;
    constexpr uint8_t LUT_TEMPERATURE_MAX_C =  40;
    constexpr uint8_t LUT_STEP_C            =   1;
}

namespace Filtering
{
    constexpr float   EMA_ALPHA_DEFAULT  = 0.15f;   // 0.0 -> No smoothing/ 1.0 -> No history
    constexpr uint8_t SMA_WINDOW_DEFAULT = 8 ;      // Effective smoothing length
}

namespace Control
{
    // Compressor Control logic parameters
    constexpr float TARGET_TEMP_C   = 4.0f;             // Fridge compartment temp
    constexpr float HYSTERESIS_C    = 1.5F;             // ON/OFF bandto avoid short cycling
    
    constexpr uint32_t MIN_COMPRESSOR_OFF_MS = 300000;  // Min rest time after run 5min to avoid damage
}
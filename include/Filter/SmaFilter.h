#pragma once

#include "interfaces/IFilter.h"
#include <stdint.h>
#include <logger/Logger.h>

/**
 * @brief Applies a simple moving average (SMA) filter to a new value.
 * 
 *  - Computes the average value of a set of samples within a predetermined window:
 *  - It's usefull for smoothing out noise in the ADC converter readings
 * 
 * @details
 *  How it's works?
 *   - It calculates the average value of the last N readings 
 *   - Simple Moving Average (SMA):  if you have 5 readings: [10, 12, 14, 16, 18], the SMA would be (10 + 12 + 14 + 16 + 18) / 5 = 14.
 *  
 * The formula to apply(T new_val):
 *  prev_avg + (new_val - prev_avg) / static_cast<T>(window_size);
 * 
 * @note This formula is a efficient approximation for a Simple Moving Average(SMA) without storing the full history
 * array, which make it ideal for embedded system where memoty is limited
 * @tparam T 
 */
template<typename T>
class SmaFilter: public IFilter
{
    public:

        /**
         * @brief Construct a new Sma Filter object
         * 
         * @param initial_value initial value for seeding
         * @param window -window size must be > 0
         */
        SmaFilter(T initial_value, uint8_t window):
        window_(window),
        prev_avg_(initial_value)
        {
           
        };

        /**
         * @brief Final initialization: window size validation
         * 
         */
        void begin()
        {
            // Skip if already initialize
            if(initialize_) return;

            // Validate and fallback window input
            if (window == 0)
            {
                LOGW("Invalid SMA window size: %d - Setting to 1", window);
                window_ =1;
            }
        };
    
    private:

        uint8_t window_;        // This the effective "memory" length
        T prev_avg_;            // State: stores the approximated avg

        bool initialize_;       // State: to avoid reinitialization
};
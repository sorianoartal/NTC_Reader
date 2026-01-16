#pragma once

#include "interfaces/IFilter.h"
#include "logger/Logger.h"


/**
 * @brief Applies Exponential Moving Average (EMA) to a new value.
 * 
 * @details EMA is a first-order infinite impulse response (IIR) low-pass filter 
 *  - A difference of the SMA this version of the EMA filter is not windowed, so it depends of all previous values
 *  - Because the calculation of the result does not requires storage this make it memory and computationally friendly(suitable fro embedded environments)
 *  - the constant alpha(0 < alpha < 1) determine how aggressive the filter is:
 *      alpha -> 0: Gives less weight to the to the new_value,
 *      alpha -> 1: Gives more weight to the previous value. 
 *  - Formula:
 *      y = (1-alpha)) * prev_input + alpha * input ; with (0 <alpha <1)
 * 
 * @tparam T 
 */
template<typename T>
class EmaFilter : public IFilter<T>
{
    public:
        
        /**
         * @brief Construct a new Ema Filter object
         * 
         * @param alpha - Smoothing factor (0 < alpha < 1) inclusive
         * @param initial_value - init value to apply to the filtered output
         */
        EmaFilter(float alpha, T initial_value = 0):
        alpha_(alpha),
        prev_value_(initial_value),
        initialize_(false)
        {};

        /// @brief Final initialization: Alpha validation
        void begin()
        {
            // Validate alpha  
            if (alpha_ < 0 || alpha_ >1)
            {
                LOGW("Invalid EMA alpha: %f.2 - clamping to 0.5",alpha_);   // LOG warning
                alpha_ = 0.5;                                               // Set to a safe default value    
            }            
        };


        // --- Implemented method from IFilter ---

        /**
         * @brief Applies Exponential Moving Average (EMA) to a new value.
         * 
         * @param new_value New input value.
         * @return T Filtered value.
         */
        T apply(T new_value) override
        {
            prev_value_ = static_cast<T>((1 - alpha_)* prev_value_ + alpha_*new_value);  // Update prev value
            return prev_value_;                                                          // return current State
        }

        // ----------------------------------------
        
    private:
        float alpha_;       // Smoothing factor
        T prev_value_;      // State: Store previous filtered value

        bool  initialize_;  // To avoid reinitialization
};
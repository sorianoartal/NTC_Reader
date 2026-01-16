#pragma once

#include <stdint.h>
#include <stddef.h>

namespace math
    {

     /**
      * @brief Calculates the absolute difference between two values.
      * 
      * @param a - First value
      * @param b - Second value
      * @return decltype(a) - Absolute difference between both values 
      */
     template<typename T>
     inline auto abs_diff(T a, T b) -> decltype(a)
     {
        return (a > b) ? (a - b) : (b - a);
     }

     /**
     * @brief Ensure that the value stays within the boundaries
     * 
     * @param value - Value to clamp
     * @param min - 
     * @param max 
     * @return decltype(value) 
     */
     template<typename T>
     inline auto clamp(T value, T min ,T max) -> decltype(value)
     {
      return (value < min) ?  min : (value>max) ? max : value;
     }

      /**
      * @brief Remaps a value from one range to another using linear interpolation.
      * @details Handles clamping and division-by-zero. Casts to long for AVR overflow prevention.
      *          See: https://en.wikipedia.org/wiki/Linear_interpolation
      * @note For ADC voltage: Use sourceMin=0, sourceMax=1024 (not 1023) for precision.
      * 
      * @tparam T The type of the values (e.g., int for raw, float for voltage).
      * @param value The input value to remap.
      * @param sourceMin The minimum of the source range.
      * @param sourceMax The maximum of the source range.
      * @param targetMin The minimum of the target range.
      * @param targetMax The maximum of the target range.
      * @return T The remapped value.
      */
      template<typename T>
      inline T remap(T value, T sourceMin, T sourceMax, T targetMin, T targetMax)
      {
        // Step1 : Handle division by zero
        if(sourceMin == sourceMax) return targetMin; 
            
        // Step2: Clamp value to the source range
        value = clamp(value, sourceMin , sourceMax);
        
        // Step3: Apply linear interpolation with long casts to prevent overflow on AVR (16-bit int)
        // N = N1 + ((S - S1) * (N2 - N1)) / (S2 - S1)
        // where N = desired value, S = corresponding value in the current domain
        //  N1, N2 are the min and max of the desired domain
        //  S1, S2 are the min and max of the current domain
        return targetMin + static_cast<T>(
            (static_cast<long>(value) - static_cast<long>(sourceMin)) * 
            (static_cast<long>(targetMax) - static_cast<long>(targetMin)) / 
            (static_cast<long>(sourceMax) - static_cast<long>(sourceMin))
           );
      }

        /**
         * @brief Computes the average of elements in a fixed-size array.
         *
         * @details Sums all elements and divides by the array size. Handles arithmetic types.
         *  Avoid division by Zero 
         *
         * @tparam T Element type (e.g., float, int).
         * @tparam N Array size (deduced).
         * @param array The array to average.
         * @return double The arithmetic mean of the elements.
         */
        template<typename T, size_t N>
        auto average(T (&array)[N]) -> double
        { 
            // Avoid division by Zero       
            static_assert(N>0, "average(): array must not be empty");
            
            T sum = 0;
            avr_algorithms::for_each_element(
                array,
                [&](const T& val){ sum += val;}
            );

            return static_cast<double>(sum) / N;
        }     

         /**
         * @brief Computes the average of a list of number known the total numbers on the list 
         * 
         * @tparam SumT 
         * @tparam CountT 
         * @param sum - Sum of the numbers that want to average
         * @param count - The total of number that are on the list 
         * @return double - Return the average as double or 0.0 if count is not valid 
         */
        template<typename SumT, typename CountT>
        inline double average(SumT sum, CountT count)
        {
            return (count) ? (static_cast<double>(sum) / static_cast<double>(count)) : 0.0;
        }


        /**
         * @brief Return the greatest of both arguments.
         * 
         * @tparam T Type of parameter
         * @param a parameters to compare
         * @param b parameter to compare
         * @return const T&  the greatest of both.
         */
        template<typename T>
        inline const T& max_custom(const T& a, const T& b) noexcept
        {
        return (a > b) ? a : b;
        }

        /**
         * @brief Return the min of both arguments 
         * 
         * @tparam T 
         * @param a 
         * @param b 
         * @return const T& 
         */
        template<typename T>
        inline const T& min_custom(const T& a, const T& b) noexcept
        {
            return (a<b)?a:b;
        }
      

    } // namespace math
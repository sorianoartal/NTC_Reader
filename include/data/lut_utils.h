#pragma once
#include<stdint.h>                  // For types like uint32_t
#include <stddef.h>                 // For size_t
#include "logger/Logger.h"          // For debuging (if needed)
#include "utils/helpers.h"          // For math helper functions
#include <limits.h>                 // For numeric limits
#include "data/thermistor_lut.h"    // For NTC LUT definition
#include "config/Config.h"          // For LUT_TEMPERATURE_MIN_C, LUT_TEMPERATURE_MAX_C

namespace lut_utils
{

    /**
     * @brief Enum that define the order of the LUT(lookup table) 
     * 
     */
    enum class LutOrder: uint8_t 
    {
        DECREASING,     // key[0] > key[1] > ... > key[N-1]
        INCREASING,     // key[0] < key[1] < ... < key[N-1]
        AUTO            // Try to autodetect by looking at the first two entries
    };


    /**
     * @brief Struct that hold the result of a LUT search
     * 
     * @details
     *  - return the bracketing result for the target value
     *  - used for interpolation between two LUT entries
     *  - lowerIdx and upperIdx are the neighbors and surround the target value
     *  - exact is the index of the exact match if found point to the exact entry
     *  - outOfRange is true if the target value is outside the LUT range and you get the near edge pair((0/1 or N-2/N-1).
     */
    struct LutBracket
    {
        public:

            /// @brief Construct with default parameters initialization
            LutBracket() :
                lowerIdx(0),
                upperIdx(0),
                exactIdx(SIZE_MAX),
                foundExact(false),
                outOfRange(false),
                clamped(false)
            {};

        
            size_t lowerIdx;        // Index of the lower bracketing entry
            size_t upperIdx;        // Index of the upper bracketing entry
            size_t exactIdx;        // Index of the exact match, if any
            bool   foundExact;      // True if an exact match was found
            bool   outOfRange;      // True if the value is outside the LUT range
            bool   clamped;         // true if target was outside LUT range and we clamped
    };



    /**
     * @brief Generic binary search to find bracketing entries in a monotonic LUT(lookup table)
     * 
     * @details
     *  - Finds the two neighboring entries surrounding the target key, or 
     *    the exact match if found.
     * 
     * @note Works on any sorted array of struct/classes via projection function that extract the key from the entry.
     * 
     * 
     * @tparam Entry   Type of elements in the LUT (struct/class)
     * @tparam N       Size of the LUT array (deduced)
     * @tparam Key     Type of the search key (e.g. uint32_t)
     * @tparam Proj    Type of projection callable: Key(const Entry&)
     * 
     * @param lut      Const reference to the sorted LUT array
     * @param target   The key value we're searching for
     * @param proj     Lambda or function that extracts key from entry(works with any struct as long as you provide how to get the key)
     * @param order    Expected sort order (or Auto to detect from first two)
     * 
     * @example Usage:
     *  auto proj = [](const ThermistorEntry& entry) { return entry.resistance_x10; };
     *  LutBracket result = binarySearchLut(NTC_LUT, targetResistance, proj, LutOrder::DECREASING);
     * 
     * @see for reference https://thelinuxcode.com/binary-search-algorithms-explained-step-by-step-in-c/
     * 
     * @return LutBracket with bracketing result
     */
    template <typename Entry, size_t N, typename Key, typename Proj>
    LutBracket binarySearchLut(
        const Entry (&lut)[N],
        Key target, 
        Proj proj, 
        LutOrder order = LutOrder::AUTO) noexcept
    {
    
        // Step0: Initialize search bounds and result
        LutBracket result{};
        result.outOfRange = true;   // assume worst case
        size_t left = 0;            // Left index(start) of the LUT
        size_t right = N - 1;       // Right index(end) of the LUT

        // Step1: Determine order if AUTO
        if (order == LutOrder::AUTO) {
            if (N < 2) {
                order = LutOrder::INCREASING;  // fallback
            } else {
                Key k0 = proj(lut[0]);
                Key k1 = proj(lut[1]);
                order = (k0 <= k1) ? LutOrder::INCREASING : LutOrder::DECREASING;
            }
        }
        // Step2: Perform Binary Search to find the exact match or bracketing entries
        while (left <= right)
        {
            // Find the mid point
            size_t mid = left + (right - left) / 2; 
            
            // Extract the mid key entry
            Key midKey = proj(lut[mid]);

            // check if there is a exact match
            if (midKey == target) {
                result.exactIdx   = mid;
                result.foundExact = true;
                result.lowerIdx   = mid;
                result.upperIdx   = mid;
                result.outOfRange = false;
                result.clamped    = false;
                return result;
            }

            // Adjust search bounds based on order
            // If target is less than midKey and order is decreasing go to the left half
            bool isTargetLessThanMid = (target < midKey);
            bool goLeft = (order == LutOrder::INCREASING) ? isTargetLessThanMid : !isTargetLessThanMid; 

            // Adjust new bound for the next iteration
            if (goLeft) 
            {
                right = mid - 1;    // Move to left half
            }
            else
            {
                left = mid + 1;     // Move to right half
            }

        }

        // Step3: No exact match found, determine bracketing indices      
        if(left == 0) 
        {
            // Target is less than the smallest entry
            result.lowerIdx = 0;
            result.upperIdx = 1;
            result.clamped = true;
        }
        else if (right >= N - 1) 
        {
            // Target is greater than the largest entry
            result.lowerIdx = N - 2;
            result.upperIdx = N - 1;
            result.clamped = true;
        }
        else 
        {
            // Normal case: target is between lut[right] and lut[left]
            result.lowerIdx = right;
            result.upperIdx = left;
            result.outOfRange = false; // within range
            result.clamped = false;
        }

        // If we are here not exact match found
        result.foundExact = false;

        // For Debugging
        if (result.foundExact)      LOGD("binarySearchLut: Found exact match at index %d", result.exactIdx);
        else if (result.outOfRange) LOGD("binarySearchLut: Target out of range, clamped=%d, lowerIdx=%d, upperIdx=%d", result.clamped, result.lowerIdx, result.upperIdx);
        else                        LOGD("binarySearchLut: Bracketing found: [%d..%d] for target %d (clamped=%s)", result.lowerIdx, result.upperIdx, (unsigned long)target, (result.clamped)?"false":"true");

        // Finally return result
        return result;
    }
        
    /**
     * @brief Generic linear interpolation between two LUT entries
     * 
     * Computes the interpolated value for a target key between two bracketing entries.
     *   y = y0 + (y1 - y0) * ((x0 - x) / (x0 - x1)) ; // Assumes x0 > x1 for descending keys (NTC resistance).   
     * 
     * @details
     * - Is equivalent to the deduce formula :
     *      t_measured = t_cold + (t_hot - t_cold) * ((r_cold - r_measured_x10) / (r_cold - r_hot))
     * Where:
     *  r_cold = higher resistance (colder temperature point)
     *   r_hot = lower resistance (hotter temperature point)
     *   t_cold = lower temperature
     *   t_hot = higher temperature
     *   Deltas are positive because r_cold > r_hot (NTC: resistance decreases with temperature), so (r_cold - r_hot) > 0, and (r_cold - r_measured) > 0 if r_measured between them.
     */
    template <typename Res , typename Temp>
    Temp applyLinearInterpolation(
        Res r_measured_x10,
        Res r_cold_x10,
        Res r_hot_x10,
        Temp t_cold_x10,
        Temp t_hot_x10
    )noexcept
    {
        // Validate deltas to avoid division by zero
        if(r_cold_x10 == r_hot_x10) 
        {
            LOGE("applyLinearInterpolation: Error - r_cold (%ld) equals r_hot (%ld), cannot interpolate.", (long)r_cold_x10, (long)r_hot_x10);
            return t_cold_x10; // or t_hot_x10, they are the same point
        }

        // Positive deltas(Assume r_cold_x10 > r_hot_x10 for NTC behavior)
        Res delta_r = r_cold_x10 - r_hot_x10;               // Positive delta resistance
        Res delta_r_measured = r_cold_x10 - r_measured_x10;

        Temp delta_t = t_hot_x10 - t_cold_x10;               // Positive delta temperature

        // Interpolate = t_cold + (delta_t * delta_r_measured) / delta_r
        int64_t t_interpolated_x10 = static_cast<int64_t>(t_cold_x10) + static_cast<int64_t>(
            (static_cast<int64_t>(delta_t) * static_cast<int64_t>(delta_r_measured)) / static_cast<int64_t>(delta_r)
        );


        // Clamp to prevent overflow/underflow if interpolated temp is out of range (based on LUT limits)
        if(t_interpolated_x10 < Sensors::LUT_TEMPERATURE_MIN_C * 10LL || t_interpolated_x10 > Sensors::LUT_TEMPERATURE_MAX_C * 10LL)
        {
            LOGW("applyLinearInterpolation: Warning - Interpolated temperature %ld (x10) is out of expected range [%d x10 .. %d x10], clamping.",
                (long)t_interpolated_x10,
                Sensors::LUT_TEMPERATURE_MIN_C * 10,
                Sensors::LUT_TEMPERATURE_MAX_C * 10
            );
        
            t_interpolated_x10 = math::clamp(
                t_interpolated_x10,
                static_cast<int64_t>(NTC_LUT[NTC_LUT_SIZE - 1].temperature_x10),      // min temp in LUT
                static_cast<int64_t>(NTC_LUT[0].temperature_x10)                     // max temp in LUT
            );

        }


        // For debugging
        LOGD("applyLinearInterpolation: r_measured=%ld, r_cold=%ld, r_hot=%ld, t_cold=%ld, t_hot=%ld", 
            (long)r_measured_x10, (long)r_cold_x10, (long)r_hot_x10, (long)t_cold_x10, (long)t_hot_x10);

        LOGD("applyLinearInterpolation: Interpolated Temperature x10: %ld", (long)t_interpolated_x10);

        return static_cast<Temp>(t_interpolated_x10);
    }


} // namespace lut_utils

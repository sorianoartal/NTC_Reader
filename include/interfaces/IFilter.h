#pragma once


/**
 * @brief Abstract signal filter interface
 * 
 * @tparam T - Generic value type to apply the filter 
 */
template<typename T>
class IFilter
{
    public:

        /// @brief Default destructor for proper cleanup
        virtual ~IFilter() = default;

        /// @brief Apply the concrete implementation of the digital processing Filter
        /// @param new_value - new value to be filter
        /// @return T        - filtered value
        virtual T apply(T new_value) = 0;
};
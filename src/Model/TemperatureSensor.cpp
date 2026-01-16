#include "Model/TemperatureSensor.h"

/**
 * @brief Construct a new Temperature Sensor:: Temperature Sensor object
 * with default parameters
 */
TemperatureSensor::TemperatureSensor():
    sampler_(nullptr),
    resistanceConverter_(nullptr),
    temperatureConverter_(nullptr),
    filter_(nullptr),
    unit_(TemperatureUnit::Celsius)
{
}

/**
 * @brief Fluent sampler setter 
 * 
 * @param sampler - Pointer ISampler concrete implementation
 * @return TemperatureSensor& - *this for method chaining 
 */
TemperatureSensor& TemperatureSensor::addSampler(ISampler* sampler)
{
    this->sampler_ = sampler;
    return *this;
}

/**
 * @brief Fluent resistance converter setter
 * 
 * @param converter - Pointer IResistanceConverter concrete implementation
 * @return TemperatureSensor& - *this for method chaining 
 */
TemperatureSensor& TemperatureSensor::addResistanceConverter(IResistanceConverter* converter)
{
    this->resistanceConverter_ = converter;
    return *this;
}

/** 
 * @brief Fluent temperature converter setter
 * 
 * @param converter - Pointer ITemperatureConverter concrete implementation
 * @return TemperatureSensor& - *this for method chaining 
 */
TemperatureSensor& TemperatureSensor::addTemperatureConverter(ITemperatureConverter* converter)
{
    this->temperatureConverter_ = converter;
    return *this;
}


/**
 * @brief Fluent temperature filter setter
 * 
 * @param filter - Pointer IFilter<int16_t> concrete implementation
 * @return * TemperatureSensor& - *this for method chaining 
 */
TemperatureSensor& TemperatureSensor::addFilter(IFilter<int16_t>* filter)
{
    this->filter_ = filter;
    return *this;
}


/**
 * @brief Fluent temperature unit setter
 * 
 * @param unit - TemperatureUnit enum value {Celsius, Fahrenheit, Kelvin}
 * @return TemperatureSensor& - *this for method chaining 
 */
TemperatureSensor& TemperatureSensor::setUnits(TemperatureUnit unit)
{
    this->unit_ = unit;
    return *this;
}

/**
 * @brief Finalize the TemperatureSensor configuration
 * 
 * @return TemperatureSensor& - *this for method chaining 
 */
TemperatureSensor& TemperatureSensor::build()
{
    // Here we could add validation to ensure all components are set
    // For simplicity, we assume the user configures everything correctly

    LOGI("TemperatureSensor built with configuration: Sampler=%p, ResistanceConverter=%p, TemperatureConverter=%p, Filter=%p, Unit=%d",
        static_cast<void*>(sampler_),
        static_cast<void*>(resistanceConverter_),
        static_cast<void*>(temperatureConverter_),
        static_cast<void*>(filter_),
        static_cast<int>(unit_)
    );

    return *this;
}

/**
 * @brief Read temperature as fixed point in tenths of degrees (0.1°C resolution)
 * 
 * @return int16_t - Temperature in tenths of degrees (e.g., 250 = 25.0 °C)
 */
int16_t TemperatureSensor::readTemperature_x10() const noexcept
{
    // validate components
    if(!sampler_ || !resistanceConverter_ || !temperatureConverter_)
    {
        LOGE("TemperatureSensor::readTemperature_x10: Sensor not properly configured");
        return -32768; // Sentinel error code
    } 

    // Step1: Sample raw ADC value
    uint16_t adc_raw = sampler_->sample();
    LOGD("TemperatureSensor::readTemperature_x10: Sampled ADC raw value: %d", adc_raw);

    // Step2: Convert ADC raw to Resistance (0.1Ω resolution)
    uint32_t resistance_x10 = resistanceConverter_->convertToResistance_x10(adc_raw);
    LOGD("TemperatureSensor::readTemperature_x10: Converted Resistance x10: %lu", (unsigned long)resistance_x10);

        // Validate resistance
        if(resistance_x10 == 0)
        {
            LOGE("TemperatureSensor::readTemperature_x10: Invalid resistance value 0");
            return -32768; // Sentinel error code
        }

    // Step3: Convert Resistance to Temperature (0.1°C resolution)
    int16_t temperature_x10 = temperatureConverter_->convertToTemperature_x10(resistance_x10);
    LOGD("TemperatureSensor::readTemperature_x10: Converted Temperature x10 (Celsius): %d", (int)temperature_x10);

        // Validate temperature
        if (temperature_x10 == -32768)
        {
            LOGE("TemperatureSensor::readTemperature_x10: Invalid temperature value from converter");
            return -32768; // Sentinel error code
        }

    // Step4: Apply filter if configured
    if(filter_)
    {
        temperature_x10 = filter_->apply(temperature_x10);
        LOGD("TemperatureSensor::readTemperature_x10: Filtered Temperature x10: %d", temperature_x10);
    }

    return temperature_x10;
}


/// --- Helper methods to read temperature in different units ---

/**
 * @brief read temperature as a float in the selected unit
 * 
 * @return float - Temperature in the selected unit (Celsius, Fahrenheit, Kelvin)
 */
float TemperatureSensor::readTemperature() const noexcept
{
    // Read temperature as fixed point scaled by 10
    int16_t temp_x10 = readTemperature_x10();

    // Check if is valid
    if(temp_x10 == -32768) return -999.9; // Sentinel invalid value

   
    // Convert to celsius by default
    float temp_c = static_cast<float>(temp_x10) / 10.0f;


    // Convert to desired unit
    return (unit_ == TemperatureUnit::Celsius) ? temp_c :
           (unit_ == TemperatureUnit::Fahrenheit) ? (temp_c * 9.0f / 5.0f + 32.0f) :
           (unit_ == TemperatureUnit::Kelvin) ? (temp_c + 273.15f) :
           temp_c; // Fallback to Celsius
}

/**
 * @brief read temperature in Celsius as float
 *  without modify the unit selected 
 * 
 * @return float - Temperature in Celsius
 */
float TemperatureSensor::readTemperatureC() noexcept
{
    // 1.Store temporally the previous unit
    TemperatureUnit prev_unit = unit_;

    // 2.Set unit to Celsius for this read
    unit_ = TemperatureUnit::Celsius;
    
    // 3.Read temperature
    float temp_c = readTemperature();
    
    // 4.Restore previous unit
    unit_ = prev_unit;

    // 5.Return temperature in Celsius
    return temp_c;
}

/**
 * @brief read temperature in Fahrenheit as float
 *  without modify the unit selected 
 * 
 * @return float - Temperature in Fahrenheit
 */
float TemperatureSensor::readTemperatureF() noexcept
{
    // 1.Store temporally the previous unit
    TemperatureUnit prev_unit = unit_;

    // 2.Set unit to Fahrenheit for this read
    unit_ = TemperatureUnit::Fahrenheit;
    
    // 3.Read temperature
    float temp_f = readTemperature();
    
    // 4.Restore previous unit
    unit_ = prev_unit;

    // 5.Return temperature in Fahrenheit
    return temp_f;
}

/**
 * @brief read temperature in Kelvin as float
 *  without modify the unit selected 
 * 
 * @return float - Temperature in Kelvin
 */
float TemperatureSensor::readTemperatureK() noexcept
{
    // 1.Store temporally the previous unit
    TemperatureUnit prev_unit = unit_;

    // 2.Set unit to Kelvin for this read
    unit_ = TemperatureUnit::Kelvin;
    
    // 3.Read temperature
    float temp_k = readTemperature();
    
    // 4.Restore previous unit
    unit_ = prev_unit;

    // 5.Return temperature in Kelvin
    return temp_k;
}
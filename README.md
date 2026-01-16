*  This application reads temperature data from NTC thermistors using:
   - ADC sampling with averaging and settling
   - Voltage divider resistance conversion
   - LUT-based temperature conversion with interpolation
   - Optional EMA filtering for stable readings
  
  - How it works:
  1. Samples ADC multiple times, discarding initial samples for settling.
  2. Averages the valid samples to get a stable ADC reading.
  3. Converts the ADC reading to resistance using the voltage divider formula.
  4. converts the resistance scaled by 10 (0.1Ω) to temperature also scaled by10 to have 0.1°C  resolution using a LUT with linear interpolation.
  5. Applies an optional Exponential Moving Average (EMA) filter to smooth out temperature readings.
 
 - How data for the NTC LUT was generated:
   NTC data is store on a LUT that contains resistance and temperature pairs.
   Each entry represents a temperature from -40°C to +40°C in 1°C steps:
   struct ThermistorEntry {
     uint32_t resistance_x10;         // Resistance in 0.1 Ω (×10), e.g. 100000 = 10000.0 Ω
     int16_t  temperature_x10;        // Temperature in 0.1 °C (×10), e.g. 250 = 25.0°C
   }; 

    ///////////////  EXAMPLE /////////////////////
// MAIN.CPP
   
// --- Global/static Objects for the sensor components ---

// Step1: Create ADC instance for both Fridge NTC sensor 

// Adc sampler for the evaporator sensor
 static AdcSampler evaporatorSampler(
  Pins::EVAPORATOR_NTC_ADC_PIN,     /* Analog pin */
  Adc::SAMPLES_TO_AVERAGE,          /* Samples to average */  
  Adc::SAMPLES_TO_DISCARD,          /* N first samples to discard */
  Adc::SETTLE_TIME_US               /* Settling time in microseconds between readings */
);

// Adc sampler for the fridge Compartment
static AdcSampler fridgeCompartmentSampler(
  Pins::COMPARTMENT_NTC_ADC_PIN,   /* Analog pin */
  Adc::SAMPLES_TO_AVERAGE,         /* Samples to average */  
  Adc::SAMPLES_TO_DISCARD,         /* N first samples to discard */
  Adc::SETTLE_TIME_US              /* Settling time in microseconds between readings */
);

// Step2: Create Resistance Converter instance (ADC->Resistance)
static VoltageDividerResistanceConverter resistanceConverter(Sensors::PULLUP_FIXED_RESISTOR_OHMS);

// Step3: Create Temperature Converter instance (Resistance->Temperature)
static LutTemperatureConverter temperatureConverter;

// Step4: Create Filter instance (optional)
static EmaFilter<int16_t> fridgeFilter(Filtering::EMA_ALPHA_DEFAULT);     // EMA
static EmaFilter<int16_t> evaporatorFilter(Filtering::EMA_ALPHA_DEFAULT); // EMA

// Step5: Create TemperatureSensor instance using the builder pattern
static TemperatureSensor fridgeTempSensor = TemperatureSensor();
static TemperatureSensor evaporatorSensor = TemperatureSensor();


// --- SETUP ---
void setup() {
  
  Serial.begin(115200);
  while (!Serial){delay(10);} // Wait a little more until Serial initialize  

  LOGI("System initializing...");
   
  // 1. Set Analog reference for ADC 
  analogReference(Adc::V_REF_VOLTS);      // 5V reference

  // 2. Initialize instances
  initSubSystems(
    evaporatorSampler,
    fridgeCompartmentSampler,
    resistanceConverter,
    temperatureConverter,
    fridgeFilter,
    evaporatorFilter
    // Add more subsystems here later if needed
  );

  
  // 3. Build TemperatureSensor for the Fridge Compartment
  fridgeTempSensor
    .addSampler(&fridgeCompartmentSampler)
    .addResistanceConverter(&resistanceConverter)
    .addTemperatureConverter(&temperatureConverter)
    .addFilter(&fridgeFilter)
    .setUnits(TemperatureUnit::Celsius)
    .build();

  // 4. Build TemperatureSensor for the Evaporator
  evaporatorSensor
    .addSampler(&evaporatorSampler)
    .addResistanceConverter(&resistanceConverter)
    .addTemperatureConverter(&temperatureConverter)
    .addFilter(&evaporatorFilter)
    .setUnits(TemperatureUnit::Celsius)
    .build();

  LOGI("System initialized.");
   
}


// --- LOOP ---
void loop() {
 
 
  LOGD("\n-----   Read Temperatures ----");

  int16_t fridgeTemp_x10 = fridgeTempSensor.readTemperature_x10();
  int16_t evaporatorTemp_x10 = evaporatorSensor.readTemperature_x10();

  LOGD("\n--- EVAPORATOR ---");
  if(fridgeTemp_x10 != -32768)
  {
    float fridgeTempC = fridgeTempSensor.readTemperatureC();
    LOGI("Fridge Temp:%.1f °C", fridgeTempC);
  }else{
    LOGW("Fridge Temp: Sensor reading error");
  }
  
  LOGD("\n---- FRIDGE COMPARTMENT ---- ");

  if(evaporatorTemp_x10 != -32768)
  {
    
    float evaporatorTempC = evaporatorSensor.readTemperatureC();   
    LOGI("Evaporator Temp:%.1f °C", evaporatorTempC);

  }else{
    
    LOGW("Evaporator Temp: Sensor reading error");
  
  }
  
  LOGD("---------------------------------\n");
 
  // Wait before next reading
  delay(2000); // 2 seconds
  
}

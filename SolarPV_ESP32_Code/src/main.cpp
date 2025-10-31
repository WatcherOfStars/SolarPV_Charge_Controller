#include <Arduino.h>
#include <vars.h>
#include <Wire.h>
#include <INA226.h>
#include <DS3231-RTC.h>

INA226 ina(0x40); // Create an INA226 object with the default I2C address
DS3231 rtc_clock; // create clock object, see https://github.com/hasenradball/DS3231-RTC

void setup(){
  // Initialize I2C
  Wire.begin(); 

  // Start serial communication for debugging
  Serial.begin(115200);
  Serial.println("System Booted");

  // Setup pins
  pinMode(DEBUG_LIGHT, OUTPUT);

  // Check for ina226 shunt
  if (!ina.begin()) {
    Serial.println("INA226 not found!");
    while (1);
  }
  // Configure the INA226 (e.g., calibration, averaging)
  //ina.setCalibration(0.01, 10.0); // Example: 0.01 Ohm shunt, 10A max current
  ina.setMaxCurrentShunt(60); //60A max current
  ina.setAverage(4); // Set averaging to 4 samples

  // Check for real time clock

  // Configure rtc

  // Check for BMS

  // Configure BMS

  // Get system settings

  // Perform initial safety checks

  // Initialize arrays

  // Calculate max discharge tribble based on cell count

  // Turn system on, indicate with debug light
  digitalWrite(DEBUG_LIGHT, HIGH);
}

void loop() {
  //##### SAFETY CHECKS #####
  // perform safety checks before executing main tasks
  safety_checks();

  //##### RETRIEVE DATA #####
  // get data from INA226
  get_shunt_data();

  // get data from RTC
  get_rtc_data();

  // update BMS depending on even or odd day
  update_bms();

  // get data from BMS
  get_bms_data();

  //##### UPDATE ARRAYS AND CALCULATIONS ##### (may want to move some to initialization)
  // update arrays
  // update vars

  // calculate pack average and total voltage
  // calculate min/max cell voltages and indexes
  // calculate pack temerature
  // calculate ambient temperature
  // calculate power in/out of system
  // calculate voltage drop behavior and resulting thresholds from panel voltage, battery voltage and charge, and load draw 

  // arrange 2-byte cell voltage array into 3-byte calculated format

  //##### SPI COMMUNICATION #####
  // SPI write every 10 seconds

  //##### POWER MANAGEMENT #####
  // if batteries full, cut power from panels
  // if batteries below threshold, enable charging from panels
  // if batteries low, cut power to loads
  // if batteries above threshold, enable power to loads

  // turn fan on or off based on temperature readings and current

  //##### PUBLISH DATA #####
  // publish over MQTT every 60 seconds
  // publish to web interface every 60 seconds

  delay(2000); // Wait for 2 seconds before the next reading
}


// Gets the voltage, current, and power from the INA226
void get_shunt_data(){
  float busVoltage = ina.getBusVoltage_mV();
  float shuntVoltage = ina.getShuntVoltage_mV();
  float current = ina.getCurrent_mA();
  float power = ina.getPower_mW();

  Serial.print("Bus Voltage: ");
  Serial.print(busVoltage);
  Serial.println(" V");

  Serial.print("Shunt Voltage: ");
  Serial.print(shuntVoltage);
  Serial.println(" mV");

  Serial.print("Current: ");
  Serial.print(current);
  Serial.println(" mA");

  Serial.print("Power: ");
  Serial.print(power);
  Serial.println(" mW");
}

void get_rtc_data(){
  // Placeholder for RTC update logic
}

// updates the battery management system depending if it's an even or odd day
void update_bms(){
  // Placeholder for BMS update logic
}

// gets cell voltages and temperatures from the BMS
void get_bms_data(){
  // Placeholder for BMS data retrieval logic
}

void safety_checks(){
  // Check overcurrent
  // Check if battery voltages are within safe limits
  // Check temperatures
  // Check component statuses
}


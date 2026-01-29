#include <Arduino.h>
#include <Wire.h>
#include <INA226.h>
#include <RTClib.h>
#include <string>
#include <iostream>
#include <web_ui.cpp>

using namespace std;

#define DEBUG_LIGHT 2 // Pin for debug light

INA226 ina(0x40); // Create an INA226 object with the default I2C address
RTC_DS3231 rtc; // create clock object
char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};


// Current time
unsigned long currentTime = millis();
// Previous time
unsigned long previousTime = 0; 
// Define timeout time in milliseconds (example: 2000ms = 2s)
const long timeoutTime = 2000;

// Current state
float busVoltage = 0;
float shuntVoltage = 0;
float current = 0;
float power = 0;
float temperature = 0;


void setup(){
  // Start serial communication for debugging
  Serial.begin(115200);
	while(!Serial);
	if(SLOW_BOOT) delay(5000); //Delay booting to give time to connect a serial monitor

  //start web ui and MQTT broker
  setupWeb();
  setUpUI();
  

  // Initialize I2C
  Wire.begin(); 

  Serial.println("System Booted");

  // Setup pins
  pinMode(DEBUG_LIGHT, OUTPUT);

  // Check for ina226 shunt
  if (!ina.begin()) {
    Serial.println("INA226 not found!");
    while (1);
  }
  // Configure the INA226 (e.g., calibration, averaging)
  ina.setMaxCurrentShunt(20, 3.7); //20A max current 1 ohm resistance
  ina.setAverage(4); // Set averaging to 4 samples

  // Check for real time clock
  if (! rtc.begin()) {
    Serial.println("Couldn't find RTC");
    while (1); // Halt if RTC is not found
  }

  // Configure rtc
  // Uncomment the following line to set the RTC to the compile time
  // This is typically done once to set the initial time.
  // rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));

  // Uncomment the following line to manually set the time (e.g., January 21, 2024 at 3:00:00)
  // rtc.adjust(DateTime(2025, 11, 6, 11, 5, 0)); 

  // Check for BMS

  // Configure BMS

  // Get system settings

  // Perform initial safety checks

  // Initialize arrays

  // Calculate max discharge tribble based on cell count

  // Turn system on, indicate with debug light
  digitalWrite(DEBUG_LIGHT, HIGH);
}


// Gets the voltage, current, and power from the INA226
void get_shunt_data(){
  // Read values from INA226 (may require calibration)
  busVoltage = ina.getBusVoltage_mV();
  shuntVoltage = ina.getShuntVoltage_mV();
  current = busVoltage / 3.7;
  power = current * busVoltage / 1000; // in mW

  Serial.print("Bus Voltage: ");
  Serial.print(busVoltage);
  Serial.println(" mV");

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
  DateTime now = rtc.now(); // Get the current date and time from the RTC

  Serial.print("Date: ");
  Serial.print(daysOfTheWeek[now.dayOfTheWeek()]); // Print day of the week
  Serial.print(", ");
  Serial.print(now.day(), DEC); // Print day
  Serial.print("/");
  Serial.print(now.month(), DEC); // Print month
  Serial.print("/");
  Serial.print(now.year(), DEC); // Print year

  Serial.print(" Time: ");
  Serial.print(now.hour(), DEC); // Print hour
  Serial.print(":");
  Serial.print(now.minute(), DEC); // Print minute
  Serial.print(":");
  Serial.print(now.second(), DEC); // Print second
  Serial.println();
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


void loop() {
  Serial.println("Starting Main Loop...");
  //##### SAFETY CHECKS #####
  // Serial.println("Performing Safety Checks...");
  // perform safety checks before executing main tasks
  safety_checks();

  //##### RETRIEVE DATA #####
  Serial.println("Retrieving Data...");
  // get data from INA226
  get_shunt_data();

  // get data from RTC
  get_rtc_data();

  // update BMS depending on even or odd day
  update_bms();

  // get data from BMS
  get_bms_data();

  //##### UPDATE ARRAYS AND CALCULATIONS ##### (may want to move some to initialization)
  // Serial.println("Updateing Data...");
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
  // Serial.println("SPI Writing...");
  // SPI write every 10 seconds

  //##### POWER MANAGEMENT #####
  // Serial.println("Managing Power...");
  // if batteries full, cut power from panels
  // if batteries below threshold, enable charging from panels
  // if batteries low, cut power to loads
  // if batteries above threshold, enable power to loads


  // turn fan on or off based on temperature readings and current

  //##### PUBLISH DATA #####
  // Serial.println("Publishing Data...");
  // publish over MQTT every 60 seconds
  // publish to web interface every 60 seconds
  updateWeb();

  
  // Flash debug light to indicate loop completion
  digitalWrite(DEBUG_LIGHT, LOW);
  delay(200);
  digitalWrite(DEBUG_LIGHT, HIGH);

  Serial.println("Main Loop Done!");
  delay(1800); // Wait for 2 seconds before the next reading
}




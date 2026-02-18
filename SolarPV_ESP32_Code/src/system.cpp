#include <system.h>

#include <INA226.h>
#include <RTClib.h>
#include <Wire.h>
#include <algorithm>
#include <iostream>
#include <LTC6802.h>

using namespace constants;
using namespace std;

INA226 ina(0x40); // Create an INA226 object with the default I2C address
RTC_DS3231 rtc; // create clock object
static LTC6802 bms = LTC6802(BMS_ADDRESS, BMS_CS_PIN); // create battery management system object

// Current state
float busVoltage = 0;
float shuntVoltage = 0;
float current = 0;
float power = 0;
float temperature = 0;

void SystemManager::setupSystem() {
    // Initialize I2C
    Wire.begin(); 

    // Setup pins
    pinMode(BMS_CLK_PIN, OUTPUT);
    pinMode(BMS_MOSI_PIN, OUTPUT);
    pinMode(BMS_MISO_PIN, INPUT);
    pinMode(BMS_CS_PIN, OUTPUT);

    pinMode(RESTART_PIN, OUTPUT);
    pinMode(SOLAR_FET_PIN, OUTPUT);
    pinMode(LOAD_FET_PIN, OUTPUT);
    pinMode(FAN_PIN, OUTPUT);

    
    // Setup (-1 error, 0 not attempted, 1 success)
    ina226Status = setupINA226();
    rtcStatus = setupRTC();
    bmsStatus = setupBMS();

    std::cout << "System setup complete. INA226 status: " << ina226Status << ", RTC status: " << rtcStatus << ", BMS status: " << bmsStatus << std::endl;


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
}

// SETUP FUNCTIONS
int SystemManager::setupRTC() {
    if(!flags.ENABLE_RTC) {
        return 0; // RTC setup not attempted
    }

    // Check for real time clock
    if (! rtc.begin()) {
        Serial.println("Couldn't find RTC");
        return -1; // Return an error code if RTC is not found
    }
    return 1; // Return success code
}

int SystemManager::setupINA226() {
    if (!flags.ENABLE_INA226) {
        return 0; // INA226 setup not attempted
    }

    // Check for ina226 shunt
    if (!ina.begin()) {
        Serial.println("INA226 not found!");
        return -1; // Return an error code if INA226 is not found
    }

    // Configure the INA226 (e.g., calibration, averaging)
    ina.setMaxCurrentShunt(MAX_CURRENT, SHUNT_RESISTANCE);
    ina.setAverage(4); // Set averaging to 4 samples
    return 1; // Return success code
}

// sets up the battery management system
int SystemManager::setupBMS(){
    //bms = LTC6802(BMS_ADDRESS, BMS_CS_PIN); // Initialize the BMS object with the I2C address and Wire instance
    LTC6802::initSPI(BMS_MOSI_PIN, BMS_MISO_PIN, BMS_CLK_PIN); // Initialize the SPI bus for the BMS
    bms.cfgRead();         // Read configuration from chip
    bms.cfgSetCDC(1);      // Measure mode 13ms
    bms.cfgSetMCI(0x0fff); // Disable interrupts
    bms.cfgWrite(false);   // Write configuration back to chip
    Serial.println("Initialized chip");
    
    return 1; // Return success code
}

void SystemManager::updateSystem() {
    //Serial.println("Updating System...");
    //##### SAFETY CHECKS #####
    // Serial.println("Performing Safety Checks...");
    // perform safety checks before executing main tasks
    //performSafetyChecks();

    //##### RETRIEVE DATA #####
    Serial.println("Retrieving Data...");
    // get data from INA226
    if(ina226Status == 1) {
        getShuntData();
    }

    // get data from RTC
    if(rtcStatus == 1) {
        getRTCData();
    }

    // update BMS depending on even or odd day
    if(bmsStatus == 1) {
        updateBMS();
    }

    // get data from BMS
    if(bmsStatus == 1) {
        getBMSData();
    }

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

}

// Gets the voltage, current, and power from the INA226
void SystemManager::getShuntData(){
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

void SystemManager::getRTCData(){
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
void SystemManager::updateBMS() {
    bms.cfgWrite(false);          // Write configuration back to chip, because chip resets these every 2.5s when nothing happens on SPI
    bms.temperatureMeasure();     // Measure temperatures on chip
    bms.temperatureRead();        // Read temperatures from chip
    bms.temperatureDebugOutput(); // Send temperatures to serial
    bms.cellsMeasure();           // Measure cell voltages on chip
    bms.cellsRead();              // Read cell voltages from chip
    bms.cellsDebugOutput();       // Send cell voltages to serial
}

// gets cell voltages and temperatures from the BMS
void SystemManager::getBMSData(){
    // Placeholder for BMS data retrieval logic
    updateBMS(); // TEMPORARY!
}

int SystemManager::performSafetyChecks(){
    // Check overcurrent
    // Check if battery voltages are within safe limits
    // Check temperatures
    // Check component statuses
    return 1;
}

void SystemManager::solarFETControl(bool state){
    digitalWrite(SOLAR_FET_PIN, state ? HIGH : LOW);
}

void SystemManager::loadFETControl(bool state){
    digitalWrite(LOAD_FET_PIN, state ? HIGH : LOW);
}

void SystemManager::fanControl(bool state){
    digitalWrite(FAN_PIN, state ? HIGH : LOW);
}

void SystemManager::notifyWebUI(char* topic, char* message) {
    // Handle WebUI notifications here
    std::cout << "Sys received WebUI notification for topic: " << topic << ", message: " << message << std::endl;
    if (strcmp(topic, "test/topic") == 0) {
        // Example: If the topic is "test/topic", print the message
        std::cout << "Handling test/topic with message: " << message << std::endl;
    }

    if (strcmp(topic, "Enable_Solar_FETs") == 0) {
        std::cout << "Handling Enable_Solar_FETs with message: " << message << std::endl;
        if (strcmp(message, "1") == 0) {
            solarFETControl(true);
        } else if (strcmp(message, "0") == 0) {
            solarFETControl(false);
        }
    }
    
    // if (strcmp(topic, "start_client") == 0) {
    //     // Example: If the topic is "start_client", perform some action
    //     std::cout << "Handling start_client with message: " << message << std::endl;
    //     if (strcmp(message, "true") == 0) {
    //         // Start the MQTT client or perform some related action
    //         std::cout << "Starting MQTT client..." << std::endl;
    //         client.setupClient();
    //     }
    // }
}

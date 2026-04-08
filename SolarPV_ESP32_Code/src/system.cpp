#include <system.h>

#include <INA226.h>
#include <RTClib.h>
#include <Wire.h>
#include <algorithm>
#include <iostream>
#include <ArduinoJson.h>
#include <LTC6802.h>

using namespace constants;
using namespace std;

Sys_Flags SystemManager::sys_flags = { 
    ENABLE_BMS: 0, 
    ENABLE_RTC: 1, 
    ENABLE_SOLAR_FETs: 1, 
    ENABLE_LOAD_FETs: 1,
    ENABLE_FAN: 0, 
    ENABLE_INA226: 1 
};
SystemData SystemManager::systemData  = {
    shuntVoltage: 1.00,
    shuntCurrent: 2.00,
    powerUse: 0.00,
    rtcTime: DateTime(2025, 1, 1, 0, 0, 0), // default time (to be updated when RTC is read)
    bmsData: {
        cellVoltages: {0.00, 0.00, 0.00, 0.00, 0.00, 0.00},
        cellTemperatures: {0.00, 0.00, 0.00, 0.00, 0.00, 0.00},
        //stateOfCharge: 100.0,
        //stateOfHealth: 100.0,
        //isCharging: false,
        //isDischarging: false
    }
};
int SystemManager::ina226Status = 0;
int SystemManager::rtcStatus = 0;
int SystemManager::bmsStatus = 0;

INA226 ina(0x40); // Create an INA226 object with the default I2C address
RTC_DS3231 rtc; // create clock object
static LTC6802 bms = LTC6802(BMS_ADDRESS, BMS_CS_PIN); // create battery management system object

float webUITimer = 0; // timer to track when to send updates to the web UI (e.g., every 5 seconds)



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

    ledcSetup(0, 5000, 8); // Setup PWM for fan control (channel 0, 5 kHz frequency, 8-bit resolution)
    ledcAttachPin(FAN_PIN, 0); // Attach the fan control pin to the PWM channel

    digitalWrite(RESTART_PIN, HIGH); // write high to prevent shutdown until a restart is triggered

    
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
    if(!sys_flags.ENABLE_RTC) {
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
    if (!sys_flags.ENABLE_INA226) {
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
    if (!sys_flags.ENABLE_BMS) {
        return 0; // BMS setup not attempted
    }

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

    //##### START OR STOP FUNCTIONS BASED ON FLAGS #####
    checkInitWithFlags();

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

    //##### WEB UI UPDATES #####
    // send updates to web UI every 5 seconds
    if(webUITimer + 5000 < millis()) {
        webUITimer = millis();
        Serial.println("Sending updates to Web UI...");
        sendUpdatesToWebUI();
        Serial.println("Updates sent to Web UI.");
    }
}

void SystemManager::checkInitWithFlags()
{
    if (sys_flags.ENABLE_BMS)
    {
        if (bmsStatus != 1)
        {
            Serial.println("Enabling BMS...");
            bmsStatus = setupBMS();
        }
    }
    else
    {
        if (bmsStatus != 0)
        {
            Serial.println("Disabling BMS...");
            bmsStatus = 0;
        }
    }

    if (sys_flags.ENABLE_RTC)
    {
        if (rtcStatus != 1)
        {
            Serial.println("Enabling RTC...");
            rtcStatus = setupRTC();
        }
    }
    else
    {
        if (rtcStatus != 0)
        {
            Serial.println("Disabling RTC...");
            rtcStatus = 0;
        }
    }

    if (sys_flags.ENABLE_INA226)
    {
        if (ina226Status != 1)
        {
            Serial.println("Enabling INA226...");
            ina226Status = setupINA226();
        }
    }
    else
    {
        if (ina226Status != 0)
        {
            Serial.println("Disabling INA226...");
            ina226Status = 0;
        }
    }
}

// Gets the voltage, current, and power from the INA226
void SystemManager::getShuntData(){
    // Read values from INA226 (may require calibration)
    systemData.shuntVoltage = ina.getShuntVoltage_mV();
    systemData.shuntCurrent = ina.getCurrent_mA();
    systemData.powerUse = systemData.shuntCurrent * systemData.shuntVoltage / 1000; // in mW

    Serial.print("Shunt Voltage: ");
    Serial.print(systemData.shuntVoltage);
    Serial.println(" mV");

    Serial.print("Shunt Voltage: ");
    Serial.print(systemData.shuntVoltage);
    Serial.println(" mV");

    Serial.print("Current: ");
    Serial.print(systemData.shuntCurrent);
    Serial.println(" mA");

    Serial.print("Power: ");
    Serial.print(systemData.powerUse);
    Serial.println(" mW");
}

void SystemManager::getRTCData(){
    // Placeholder for RTC update logic
    systemData.rtcTime = rtc.now(); // Get the current date and time from the RTC

    // Serial.print("Date: ");
    // Serial.print(daysOfTheWeek[systemData.rtcTime.dayOfTheWeek()]); // Print day of the week
    // Serial.print(", ");
    // Serial.print(systemData.rtcTime.day(), DEC); // Print day
    // Serial.print("/");
    // Serial.print(systemData.rtcTime.month(), DEC); // Print month
    // Serial.print("/");
    // Serial.print(systemData.rtcTime.year(), DEC); // Print year

    // Serial.print(" Time: ");
    // Serial.print(systemData.rtcTime.hour(), DEC); // Print hour
    // Serial.print(":");
    // Serial.print(systemData.rtcTime.minute(), DEC); // Print minute
    // Serial.print(":");
    // Serial.print(systemData.rtcTime.second(), DEC); // Print second
    // Serial.println();
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
    if(sys_flags.ENABLE_SOLAR_FETs == 0) state=false; // Turn off if solar FET control is disabled by flags
    digitalWrite(SOLAR_FET_PIN, state ? HIGH : LOW);
}

void SystemManager::loadFETControl(bool state){
    if(sys_flags.ENABLE_LOAD_FETs == 0) state=false; // Turn off if load FET control is disabled by flags
    digitalWrite(LOAD_FET_PIN, state ? HIGH : LOW);
}

void SystemManager::fanControl(bool state){
    if(sys_flags.ENABLE_FAN == 0) state=false; // Turn off if fan control is disabled by flags
    ledcWrite(0, state ? (FAN_DUTY_CYCLE * 255) : 0); // Set fan speed to max duty cycle (1=255) or off (0) using PWM
}

void SystemManager::onNotify(const char* topic, const char* message) {
    // Handle WebUI notifications here
    std::cout << "Sys received WebUI notification for topic: " << topic << ", message: " << message << std::endl;

    // HANDLE REBOOT REQUEST
    if (strcmp(topic, "Restart_System") == 0) {
        std::cout << "Handling System_Reboot with message: " << message << std::endl;
        Serial.println("Rebooting system...");
        digitalWrite(RESTART_PIN, LOW); // restart pin to shut down the system
        delay(1000);
    }

    // HANDLE MANUAL TOGGLES
    else if (strcmp(topic, "Toggle_Solar_FETs") == 0) {
        std::cout << "Handling Toggle_Solar_FETs with message: " << message << std::endl;
        if (strcmp(message, "1") == 0) solarFETControl(true);
        else if (strcmp(message, "0") == 0) solarFETControl(false);
    }

    else if (strcmp(topic, "Toggle_Load_FETs") == 0) {
        std::cout << "Handling Toggle_Load_FETs with message: " << message << std::endl;
        if (strcmp(message, "1") == 0) loadFETControl(true);
        else if (strcmp(message, "0") == 0) loadFETControl(false);
    }

    else if (strcmp(topic, "Toggle_Fan") == 0) {
        std::cout << "Handling Toggle_Fan with message: " << message << std::endl;
        if (strcmp(message, "1") == 0) fanControl(true);
        else if (strcmp(message, "0") == 0) fanControl(false);
    }

    // HANDLE FLAG TOGGLES

    else if (strcmp(topic, "Enable_BMS") == 0) {
        std::cout << "Handling Enable_BMS with message: " << message << std::endl;
        if (strcmp(message, "1") == 0) sys_flags.ENABLE_BMS = 1;
        else if (strcmp(message, "0") == 0) {
            sys_flags.ENABLE_BMS = 0;
            // Also turn off loads and solar FETs if BMS is disabled for safety
            //solarFETControl(false);
            //loadFETControl(false);
        }
    }

    else if (strcmp(topic, "Enable_RTC") == 0) {
        std::cout << "Handling Enable_RTC with message: " << message << std::endl;
        if (strcmp(message, "1") == 0) sys_flags.ENABLE_RTC = 1;
        else if (strcmp(message, "0") == 0) sys_flags.ENABLE_RTC = 0;
    }

    else if (strcmp(topic, "Enable_Solar_FETs") == 0) {
        std::cout << "Handling Enable_Solar_FETs with message: " << message << std::endl;
        if (strcmp(message, "1") == 0) sys_flags.ENABLE_SOLAR_FETs = 1;
        else if (strcmp(message, "0") == 0) {
            sys_flags.ENABLE_SOLAR_FETs = 0;
            // Also turn off solar FETs if control is disabled for safety
            solarFETControl(false);
        }
    }

    else if (strcmp(topic, "Enable_Load_FETs") == 0) {
        std::cout << "Handling Enable_Load_FETs with message: " << message << std::endl;
        if (strcmp(message, "1") == 0) sys_flags.ENABLE_LOAD_FETs = 1;
        else if (strcmp(message, "0") == 0) {
            sys_flags.ENABLE_LOAD_FETs = 0;
            // Also turn off load FETs if control is disabled for safety
            loadFETControl(false);
        }
    }

    else if (strcmp(topic, "Enable_Fan") == 0) {
        std::cout << "Handling Enable_Fan with message: " << message << std::endl;
        if (strcmp(message, "1") == 0) sys_flags.ENABLE_FAN = 1;
        else if (strcmp(message, "0") == 0) {
            sys_flags.ENABLE_FAN = 0;
            // Also turn off fan if control is disabled for safety
            fanControl(false);
        }
    }

    else if (strcmp(topic, "Enable_INA226") == 0) {
        std::cout << "Handling Enable_INA226 with message: " << message << std::endl;
        if (strcmp(message, "1") == 0) sys_flags.ENABLE_INA226 = 1;
        else if (strcmp(message, "0") == 0) sys_flags.ENABLE_INA226 = 0;
    }
}

void SystemManager::sendUpdatesToWebUI(){
    // Build JSON for flags using ArduinoJson
    JsonDocument flagsDoc;
    flagsDoc["Enable_BMS"] = (int)sys_flags.ENABLE_BMS;
    flagsDoc["Enable_RTC"] = (int)sys_flags.ENABLE_RTC;
    flagsDoc["Enable_Solar_FETs"] = (int)sys_flags.ENABLE_SOLAR_FETs;
    flagsDoc["Enable_Load_FETs"] = (int)sys_flags.ENABLE_LOAD_FETs;
    flagsDoc["Enable_Fan"] = (int)sys_flags.ENABLE_FAN;
    flagsDoc["Enable_INA226"] = (int)sys_flags.ENABLE_INA226;
    std::string flagsOut;
    serializeJson(flagsDoc, flagsOut);
    notifyObservers("system_update/flags", flagsOut.c_str());

    // Build JSON for data using ArduinoJson
    JsonDocument dataDoc;
    dataDoc["Toggle_Solar_FETs"] = digitalRead(SOLAR_FET_PIN);
    dataDoc["Toggle_Load_FETs"] = digitalRead(LOAD_FET_PIN);

    auto fanStatus = []() -> int {
        int pwmValue = ledcRead(0); // Read the current PWM value for the fan on channel 0
        if (pwmValue > FAN_DUTY_CYCLE * 255 * 0.5) return 1; // Fan is on
        else return 0; // Fan is off
    };
    dataDoc["Toggle_Fan"] = fanStatus();

    dataDoc["Shunt_Voltage"] = systemData.shuntVoltage;
    dataDoc["Shunt_Current"] = systemData.shuntCurrent;
    dataDoc["Power"] = systemData.powerUse;
    dataDoc["RTC_Time"] = systemData.rtcTime.timestamp();

    // BMS cell voltages
    JsonArray cells = dataDoc["Cell_Voltages"].to<JsonArray>();
    for (int i = 0; i < 6; ++i) cells.add(systemData.bmsData.cellVoltages[i]);

    // BMS temperatures
    JsonArray temps = dataDoc["Cell_Temperatures"].to<JsonArray>();
    for (int i = 0; i < 6; ++i) temps.add(systemData.bmsData.cellTemperatures[i]);

    // Stautus of components
    dataDoc["INA226_Status"] = ina226Status;
    dataDoc["RTC_Status"] = rtcStatus;
    dataDoc["BMS_Status"] = bmsStatus;

    std::string dataOut;
    serializeJson(dataDoc, dataOut);
    notifyObservers("system_update/data", dataOut.c_str());
}

// System Subject implementation
void SystemManager::registerObserver(observer* obs) {
	std::cout << "Registering System Observer " << obs << std::endl;
	observers.push_back(obs);
}
void SystemManager::removeObserver(observer* obs) {
	observers.erase(std::remove(observers.begin(), observers.end(), obs), observers.end());
}
void SystemManager::notifyObservers(const char* topic, const char* message) {
	// std::cout << "Notifying System Observers for topic: " << topic << std::endl;
	// std::cout << "Message: " << message << std::endl;
	for (auto& obs : observers) {
		obs->onNotify(topic, message);
	}
}

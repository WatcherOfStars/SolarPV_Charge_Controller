#include <system.h>

#include <INA226.h>
#include <RTClib.h>
#include <Wire.h>
#include <algorithm>
#include <iostream>
#include <ArduinoJson.h>
#include "ltc6802.h"

using namespace constants;
using namespace std;

Sys_Flags SystemManager::sys_flags = { 
    ENABLE_BMS: 0, 
    ENABLE_RTC: 0, 
    ENABLE_SOLAR_FETs: 0, 
    ENABLE_LOAD_FETs: 0,
    ENABLE_FAN: 0, 
    ENABLE_SOLAR_INA: 0, 
    ENABLE_LOAD_INA: 0,
    ENABLE_FAKE_BATTERY: 0, // enable fake battery data for testing without BMS
};
SystemData SystemManager::systemData  = {
    solarShuntVoltage: 0.00,
    loadShuntVoltage: 0.00,
    solarShuntCurrent: 0.00,
    loadShuntCurrent: 0.00,
    solarPowerUse: 0.00,
    loadPowerUse: 0.00,
    rtcTime: DateTime(2025, 1, 1, 0, 0, 0), // default time (to be updated when RTC is read)
    batt: {
        cellVoltages: {0.00, 0.00, 0.00, 0.00, 0.00, 0.00},
        cellTemperatures: {0.00, 0.00, 0.00, 0.00, 0.00, 0.00},
        maxCellVoltage: 0.00,
        minCellVoltage: 0.00,
        maxCellIndex: 0,
        minCellIndex: 0,
        averageCellVoltage: 0.00,
        //stateOfCharge: 100.0,
        //stateOfHealth: 100.0,
        isCharging: false,
        isDischarging: false
    },
    error: 0
};
volatile int SystemManager::solarInaStatus = 0;
volatile int SystemManager::loadInaStatus = 0;
volatile int SystemManager::rtcStatus = 0;
volatile int SystemManager::bmsStatus = 0;

INA226 solarIna(0x40, &Wire); // Create an INA226 object for the solar INA with the default I2C address
INA226 loadIna(0x41, &Wire); // Create an INA226 object for the load INA with a different I2C address
RTC_DS3231 rtc; // create clock object

float webUITimer = 0; // timer to track when to send updates to the web UI (e.g., every 5 seconds)
bool firstUpdate = true; // flag to indicate if this is the first update (used to send initial data to web UI immediately on startup)

DeviceConfig deviceConfig;


void SystemManager::setupSystem() {
    // get consts from config
    deviceConfig = ConfigManager::getInstance().deviceConfig;

    // Initialize I2C (one is swapped due to pcb design)
    Wire.begin(deviceConfig.wire_sda_pin, deviceConfig.wire_scl_pin); 
    Wire1.begin(deviceConfig.wire_scl_pin, deviceConfig.wire_sda_pin); 


    // Setup pins
    pinMode(deviceConfig.restart_pin, OUTPUT);
    pinMode(deviceConfig.solar_fet_pin, OUTPUT);
    pinMode(deviceConfig.solar_safety_fet_pin, OUTPUT);
    pinMode(deviceConfig.load_fet_pin, OUTPUT);
    pinMode(deviceConfig.fan_pin, OUTPUT);

    ledcSetup(0, 5000, 8); // Setup PWM for fan control (channel 0, 5 kHz frequency, 8-bit resolution)
    ledcAttachPin(deviceConfig.fan_pin, 0); // Attach the fan control pin to the PWM channel

    digitalWrite(deviceConfig.restart_pin, HIGH); // write high to prevent shutdown until a restart is triggered
    solarFETControl(false);
    loadFETControl(false);
    digitalWrite(deviceConfig.solar_safety_fet_pin, HIGH); // close safety fets- they will only be opened when an error occurs
    
    // Setup (-1 error, 0 not attempted, 1 success)
    // solarInaStatus = setupSolarINA(); DISABLED DUE TO HARDWARE ISSUE (low impedance path between battery and solar grounds, causes magic smoke)
    loadInaStatus = setupLoadINA();
    rtcStatus = setupRTC();
    bmsStatus = setupBMS();

    systemData.batt.isCharging = false; // default to not charging, will be updated when BMS data is read
    systemData.batt.isDischarging = false; // default to not discharging, will be updated when BMS data is read

    std::cout << "System setup complete. Load INA status: " << loadInaStatus << ", RTC status: " << rtcStatus << ", BMS status: " << bmsStatus << std::endl;


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
    if (! rtc.begin(&Wire1)) {
        Serial.println("Couldn't find RTC");
        return -1; // Return an error code if RTC is not found
    }
    return 1; // Return success code
}

int SystemManager::setupSolarINA() {
    if (!sys_flags.ENABLE_SOLAR_INA) {
        return 0; // Solar INA setup not attempted
    }

    // Check for solar INA shunt
    if (!solarIna.begin()) {
        Serial.println("Solar INA not found!");
        return -1; // Return an error code if Solar INA is not found
    }

    // Configure the INA226 (e.g., calibration, averaging)
    solarIna.setMaxCurrentShunt(deviceConfig.max_current, deviceConfig.solar_shunt_resistance);
    solarIna.setAverage(4); // Set averaging to 4 samples
    return 1; // Return success code
}

int SystemManager::setupLoadINA() {
    if (!sys_flags.ENABLE_LOAD_INA) {
        return 0; // Load INA setup not attempted
    }

    // Check for load INA shunt
    if (!loadIna.begin()) {
        Serial.println("Load INA not found!");
        return -1; // Return an error code if Load INA is not found
    }

    // Configure the INA226 (e.g., calibration, averaging)
    loadIna.setMaxCurrentShunt(deviceConfig.max_current, deviceConfig.load_shunt_resistance);
    loadIna.setAverage(4); // Set averaging to 4 samples
    return 1; // Return success code
}

// sets up the battery management system
int SystemManager::setupBMS(){
    if (!sys_flags.ENABLE_BMS) {
        return 0; // BMS setup not attempted
    }

    return setupLTC6802(); // setup LTC6802 BMS
}

void SystemManager::updateSystem() {
    //Serial.println("Updating System...");
    Serial.println("-----");
    //##### SAFETY CHECKS #####
    Serial.println("Performing Safety Checks...");
    if(!firstUpdate)systemData.error = performSafetyChecks();

    if(systemData.error != 0) {
        Serial.println("Safety check failed with error code: " + String(systemData.error) + ". Taking appropriate action.");
        // take appropriate action based on error code (e.g., shut down system, send alert, etc.)
        return; // exit updateSystem early
    }

    //##### START OR STOP FUNCTIONS BASED ON FLAGS #####
    checkInitWithFlags();
    Serial.println("Status flags: Load INA: " + String(loadInaStatus) + ", RTC: " + String(rtcStatus) + ", BMS: " + String(bmsStatus));

    //##### RETRIEVE DATA #####
    Serial.println("Retrieving Data...");
    // get data from solar INA226
    // if(solarInaStatus == 1) getSolarShuntData();

    // get data from load INA226 
    if(loadInaStatus == 1) getLoadShuntData();

    // get data from RTC
    if(rtcStatus == 1) rtcStatus = getRTCData();

    // get data from BMS
    if(bmsStatus == 1) {
        bmsStatus = updateBMS(); // update BMS depending on even or odd day
        if(!sys_flags.ENABLE_FAKE_BATTERY) getBMSData(); // only get BMS data if not using fake battery data
    }

    // manage power only if BMS working or using test data (i.e., BMS disabled)
    if(bmsStatus == 1 || sys_flags.ENABLE_FAKE_BATTERY) {
        // calculate min/max cell voltages and indexes
        systemData.batt.minCellVoltage = *min_element(systemData.batt.cellVoltages, systemData.batt.cellVoltages + deviceConfig.num_cells);
        systemData.batt.maxCellVoltage = *max_element(systemData.batt.cellVoltages, systemData.batt.cellVoltages + deviceConfig.num_cells);
        systemData.batt.minCellIndex = distance(systemData.batt.cellVoltages, min_element(systemData.batt.cellVoltages, systemData.batt.cellVoltages + deviceConfig.num_cells));
        systemData.batt.maxCellIndex = distance(systemData.batt.cellVoltages, max_element(systemData.batt.cellVoltages, systemData.batt.cellVoltages + deviceConfig.num_cells));

        // calculate pack average and total voltage
        uint16_t totalVoltage = 0;
        for (int i = 0; i < deviceConfig.num_cells; ++i) {
            totalVoltage += systemData.batt.cellVoltages[i];
        }
        systemData.batt.averageCellVoltage = totalVoltage / deviceConfig.num_cells;
        
        //##### POWER MANAGEMENT #####
        Serial.println("Managing Power...");
        Serial.print("Charging: ");
        Serial.print(systemData.batt.isCharging);
        Serial.print("   Discharging: ");
        Serial.print(systemData.batt.isDischarging);
        Serial.print("   Max Cell V: ");
        Serial.print(systemData.batt.maxCellVoltage);
        Serial.print("   Min Cell V: ");
        Serial.print(systemData.batt.minCellVoltage);
        Serial.print("   Max allowed V: ");
        Serial.print(deviceConfig.max_cell_voltage);
        Serial.print("   Min allowed V: ");
        Serial.println(deviceConfig.min_cell_voltage);

        // if batteries full, cut power from panels
        if(systemData.batt.maxCellVoltage > deviceConfig.max_cell_voltage && systemData.batt.isCharging) {
            Serial.println("Battery full, cutting power from panels. Average cell voltage: " + String(systemData.batt.averageCellVoltage) + " V");
            solarFETControl(false); // cut power from panels
        } 
        // if batteries below threshold, enable charging from panels
        else if(systemData.batt.maxCellVoltage < deviceConfig.max_cell_voltage - deviceConfig.cell_voltage_hysteresis && !systemData.batt.isCharging) {
            Serial.println("Battery low, enabling power from panels. Average cell voltage: " + String(systemData.batt.averageCellVoltage) + " V");
            solarFETControl(true); // enable power from panels
        }
        // if batteries low, cut power to loads
        else if(systemData.batt.minCellVoltage < deviceConfig.min_cell_voltage && systemData.batt.isDischarging) {
            Serial.println("Battery critically low, cutting power to loads. Average cell voltage: " + String(systemData.batt.averageCellVoltage) + " V");
            loadFETControl(false); // cut power to loads
        }
        // if batteries above threshold, enable power to loads
        else if(systemData.batt.minCellVoltage > deviceConfig.min_cell_voltage + deviceConfig.cell_voltage_hysteresis && !systemData.batt.isDischarging) {
            Serial.println("Battery above threshold, enabling power to loads. Average cell voltage: " + String(systemData.batt.averageCellVoltage) + " V");
            loadFETControl(true); // enable power to loads
        }
    }
    //##### UPDATE ARRAYS AND CALCULATIONS ##### (may want to move some to initialization)
    // Serial.println("Updating Data...");
    // update arrays
    // update vars



    // calculate pack temerature
    // calculate ambient temperature
    // calculate power in/out of system
    // calculate voltage drop behavior and resulting thresholds from panel voltage, battery voltage and charge, and load draw 

    // arrange 2-byte cell voltage array into 3-byte calculated format

    //##### SPI COMMUNICATION #####
    // Serial.println("SPI Writing...");
    // SPI write every 10 seconds

    

    // turn fan on or off based on temperature readings and current

    //##### WEB UI UPDATES #####
    // send updates to web UI every 5 seconds
    if(webUITimer + 5000 < millis()) {
        webUITimer = millis();
        Serial.println("Sending updates to Web UI...");
        sendUpdatesToWebUI();
        Serial.println("Updates sent to Web UI.");
    }
    if(firstUpdate) firstUpdate = false; // reset first update flag after initial update
    Serial.println("-----");
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

    if (sys_flags.ENABLE_SOLAR_INA)
    {
        if (solarInaStatus != 1)
        {
            Serial.println("Enabling Solar INA...");
            solarInaStatus = setupSolarINA();
        }
    }
    else
    {
        if (solarInaStatus != 0)
        {
            Serial.println("Disabling Solar INA...");
            solarInaStatus = 0;
        }
    }

    if (sys_flags.ENABLE_LOAD_INA)
    {
        if (loadInaStatus != 1)
        {
            Serial.println("Enabling Load INA...");
            loadInaStatus = setupLoadINA();
        }
    }
    else
    {
        if (loadInaStatus != 0)
        {
            Serial.println("Disabling Load INA...");
            loadInaStatus = 0;
        }
    }   
}

// Gets the voltage, current, and power from the INA226
int SystemManager::getSolarShuntData(){
    // Read values from INA226 (may require calibration)
    systemData.solarShuntVoltage = solarIna.getShuntVoltage_mV();
    //systemData.solarShuntCurrent = solarIna.getCurrent_mA();
    systemData.solarShuntCurrent = systemData.solarShuntVoltage / deviceConfig.solar_shunt_resistance;
    systemData.solarPowerUse = systemData.solarShuntCurrent * systemData.solarShuntVoltage / 1000; // in mW

    // Serial.print("Shunt Voltage: ");
    // Serial.print(systemData.solarShuntVoltage);
    // Serial.println(" mV");

    // Serial.print("Current: ");
    // Serial.print(systemData.solarShuntCurrent);
    // Serial.println(" mA");

    // Serial.print("Power: ");
    // Serial.print(systemData.solarPowerUse);
    // Serial.println(" mW");
    return 1;
}

// Gets the voltage, current, and power from the INA226
int SystemManager::getLoadShuntData(){
    // Read values from INA226 (may require calibration)
    systemData.loadShuntVoltage = loadIna.getShuntVoltage_mV();
    //systemData.loadShuntCurrent = loadIna.getCurrent_mA();
    systemData.loadShuntCurrent = systemData.loadShuntVoltage / deviceConfig.load_shunt_resistance;
    systemData.loadPowerUse = systemData.loadShuntCurrent * systemData.loadShuntVoltage / 1000; // in mW

    Serial.print("Load Shunt Voltage: ");
    Serial.print(systemData.loadShuntVoltage);
    Serial.println(" mV");

    // Serial.print("Shunt Voltage: ");
    // Serial.print(systemData.loadShuntVoltage);
    // Serial.println(" mV");

    Serial.print("Load Current: ");
    Serial.print(systemData.loadShuntCurrent);
    Serial.println(" mA");

    // Serial.print("Power: ");
    // Serial.print(systemData.loadPowerUse);
    // Serial.println(" mW");
    return 1;
}

int SystemManager::getRTCData(){
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
    return 1;
}

// updates the battery management system depending if it's an even or odd day
int SystemManager::updateBMS() {
    return updateLTC6802(); // update LTC6802 BMS
}

// gets cell voltages and temperatures from the BMS
int SystemManager::getBMSData(){
    //Serial.print("Getting BMS data...   ");
    // BMS data retrieval logic
    float* cellVoltages = getCellVoltages(); // get cell voltages from LTC6802
    for (int i = 0; i < 6; ++i) {
        // Serial.print("Cell ");        Serial.print(i);
        // Serial.print(" Voltage: ");
        // Serial.print(cellVoltages[i]);
        // Serial.println(" V");
        systemData.batt.cellVoltages[i] = cellVoltages[i];
    }
    //Serial.println();
    return 1;
}


int SystemManager::performSafetyChecks(){
    // This function performs safety checks on the system and returns an error code if any issues are detected. Error codes can be defined as follows:
    // 0: No error
    // 1: Overcurrent detected
    // 2: Solar FET failure detected
    // 3: Load FET failure detected
    // 5: Cell voltage below safety threshold detected
    // 6: BMS communication failure detected
    // 7: RTC communication failure detected
    // 8: Solar INA226 communication failure detected
    // 9: Load INA226 communication failure detected

    // Check overcurrent
    if(systemData.loadShuntCurrent > deviceConfig.max_current && loadInaStatus == 1) {
        Serial.println("Overcurrent detected! Shutting off loads. Current: " + String(systemData.loadShuntCurrent) + " mA");
        loadFETControl(false); // cut power to loads
        return 1; // return error code for overcurrent
    }

    // Check for FET failures
    // // Solar should be disconnected but current is still flowing - DISABLED DUE TO HARDWARE ISSUE (low impedance path between battery and solar grounds, causes magic smoke)
    // if(!systemData.batt.isCharging && systemData.solarShuntCurrent > deviceConfig.max_current * 0.1 && solarInaStatus == 1) { // if solar FETs should be off but current is above 10% of max, assume FETs failed closed
    //     Serial.println("Solar FET failure detected! Current: " + String(systemData.solarShuntCurrent) + " mA");
    //     // TODO: add second layer of FETs
    //     //return 2; // return error code for solar FET failure
    // }

    // Check for battery overcharge
    if(systemData.batt.maxCellVoltage > deviceConfig.max_cell_voltage + 0.1 && (bmsStatus == 1 || sys_flags.ENABLE_FAKE_BATTERY)) { 
        Serial.println("Battery overcharge detected! Max cell voltage: " + String(systemData.batt.maxCellVoltage) + " V. Cutting power from panels.");
        solarFETControl(false); // cut power from panels
        digitalWrite(deviceConfig.solar_safety_fet_pin, LOW); // open safety fets to disconnect battery from solar input in case solar FETs failed closed
        return 2; // return error code for battery overcharge
    }

    // Battery should be disconnected but current is still flowing
    if(!systemData.batt.isDischarging && systemData.loadShuntCurrent > deviceConfig.max_current * 0.1 && loadInaStatus == 1) { // if load FETs should be off but current is above 10% of max, assume FETs failed closed
        Serial.println("Load FET failure detected! Current: " + String(systemData.loadShuntCurrent) + " mA");
        // TODO: add second layer of FETs
        //return 3; // return error code for load FET failure
    }

    // Check for battery below safety voltage
    if(systemData.batt.minCellVoltage < deviceConfig.safety_cell_voltage && (bmsStatus == 1 || sys_flags.ENABLE_FAKE_BATTERY)) {
        Serial.println("Cell below safety voltage, shutting down " + String(systemData.batt.minCellIndex) + "! Cell voltage: " + String(systemData.batt.minCellVoltage) + " V. Shutting off loads.");
        loadFETControl(false); // cut power to loads
        digitalWrite(deviceConfig.restart_pin, LOW); // restart pin to shut down the system
        delay(1000);
        return 5; // return error code for cell voltage below safety threshold
    }

    // Check if BMS connected using read
    uint8_t temp_cfr[6];
    if(sys_flags.ENABLE_BMS && bmsStatus == 1 && readLTC6802(0x02, 6, temp_cfr) == -1) { // if BMS is enabled and was previously working but now read fails, assume communication failure
        Serial.println("BMS communication failure detected!");
        bmsStatus = -1; // update BMS status to indicate failure
        return 6; // return error code for BMS communication failure
    }

    // // Check BMS communication
    // if(sys_flags.ENABLE_BMS && bmsStatus != 1) {
    //     Serial.println("BMS communication failure detected!");
    //     return 6; // return error code for BMS communication failure
    // }

    // // Check RTC communication
    // if(sys_flags.ENABLE_RTC && rtcStatus != 1) {
    //     Serial.println("RTC communication failure detected!");
    //     return 7; // return error code for RTC communication failure
    // }

    // // Check INA226 communication
    // if(sys_flags.ENABLE_SOLAR_INA && solarInaStatus != 1) {
    //     Serial.println("Solar INA communication failure detected!");
    //     return 8; // return error code for solar INA communication failure
    // }
    // if(sys_flags.ENABLE_LOAD_INA && loadInaStatus != 1) {
    //     Serial.println("Load INA communication failure detected!");
    //     return 9; // return error code for load INA communication failure
    // }
    // Check temperatures
    // Check component statuses for disconnects or faults
    return 0;
}

void SystemManager::solarFETControl(bool state){
    if(sys_flags.ENABLE_SOLAR_FETs == 0) state=false; // Turn off if solar FET control is disabled by flags
    digitalWrite(deviceConfig.solar_fet_pin, state ? HIGH : LOW);
    systemData.batt.isCharging = state; // update charging status based on solar FET state
}

void SystemManager::loadFETControl(bool state){
    if(sys_flags.ENABLE_LOAD_FETs == 0) state=false; // Turn off if load FET control is disabled by flags
    digitalWrite(deviceConfig.load_fet_pin, state ? HIGH : LOW);
    systemData.batt.isDischarging = state; // update discharging status based on load FET state
}

void SystemManager::fanControl(bool state){
    if(sys_flags.ENABLE_FAN == 0) state=false; // Turn off if fan control is disabled by flags
    ledcWrite(0, state ? (deviceConfig.fan_duty_cycle * 255) : 0); // Set fan speed to max duty cycle (1=255) or off (0) using PWM
}

void SystemManager::balanceCells() {
    // This function will be called when the balance timer expires. Implement balancing logic here.
    Serial.println("Balance timer triggered, balancing cells");
    if(bmsStatus == 1){
        BattData batt = systemData.batt;
        if(rtcStatus == 1) {
            // if even day, pull down high cells
            if(systemData.rtcTime.day() % 2 == 0) {
                pullDownBalance(batt.cellVoltages, &batt.averageCellVoltage);
            }
            // if odd day, pull up low cells
            else {
                pullUpBalance(batt.cellVoltages, &batt.averageCellVoltage, &batt.minCellIndex);
            }
        }
        else {
            // RTC is disabled/unavailable, so avoid using default or stale day values.
            // Fall back to a deterministic balancing strategy that does not depend on time.
            pullDownBalance(batt.cellVoltages, &batt.averageCellVoltage);
        }
    }
    
}

void SystemManager::onNotify(const char* topic, const char* message) {
    // Handle WebUI notifications here
    std::cout << "Sys received WebUI notification for topic: " << topic << ", message: " << message << std::endl;

    // HANDLE REBOOT REQUEST
    if (strcmp(topic, "Restart_System") == 0) {
        std::cout << "Handling System_Reboot with message: " << message << std::endl;
        Serial.println("Rebooting system...");
        digitalWrite(deviceConfig.restart_pin, LOW); // restart pin to shut down the system
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

    else if (strcmp(topic, "Enable_Solar_INA") == 0) {
        std::cout << "Handling Enable_Solar_INA with message: " << message << std::endl;
        if (strcmp(message, "1") == 0) sys_flags.ENABLE_SOLAR_INA = 1;
        else if (strcmp(message, "0") == 0) sys_flags.ENABLE_SOLAR_INA = 0;
    }

    else if (strcmp(topic, "Enable_Load_INA") == 0) {
        std::cout << "Handling Enable_Load_INA with message: " << message << std::endl;
        if (strcmp(message, "1") == 0) sys_flags.ENABLE_LOAD_INA = 1;
        else if (strcmp(message, "0") == 0) sys_flags.ENABLE_LOAD_INA = 0;
    }
    
    else if (strcmp(topic, "Test_Voltage_Slider") == 0 && sys_flags.ENABLE_FAKE_BATTERY && !sys_flags.ENABLE_BMS) {
        std::cout << "Handling Test_Voltage_Slider with message: " << message << std::endl;
        // set each cell voltage to the slider value for testing ONLY if enables and BMS disabled
        float testVoltage = atof(message); // convert message to float
        for (int i = 0; i < 6; ++i) {
            systemData.batt.cellVoltages[i] = testVoltage;
        }
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
    flagsDoc["Enable_Solar_INA"] = (int)sys_flags.ENABLE_SOLAR_INA;
    flagsDoc["Enable_Load_INA"] = (int)sys_flags.ENABLE_LOAD_INA;
    std::string flagsOut;
    serializeJson(flagsDoc, flagsOut);
    notifyObservers("system_update/flags", flagsOut.c_str());

    // Build JSON for data using ArduinoJson
    JsonDocument dataDoc;
    dataDoc["Toggle_Solar_FETs"] = digitalRead(deviceConfig.solar_fet_pin);
    dataDoc["Toggle_Load_FETs"] = digitalRead(deviceConfig.load_fet_pin);

    auto fanStatus = []() -> int {
        int pwmValue = ledcRead(0); // Read the current PWM value for the fan on channel 0
        if (pwmValue > deviceConfig.fan_duty_cycle * 255 * 0.5) return 1; // Fan is on
        else return 0; // Fan is off
    };
    dataDoc["Toggle_Fan"] = fanStatus();

    dataDoc["Solar_Shunt_Current"] = systemData.solarShuntCurrent;
    dataDoc["Solar_Shunt_Voltage"] = systemData.solarShuntVoltage;
    dataDoc["Load_Shunt_Current"] = systemData.loadShuntCurrent;
    dataDoc["Load_Shunt_Voltage"] = systemData.loadShuntVoltage;
    dataDoc["RTC_Time"] = systemData.rtcTime.timestamp();

    // BMS cell voltages
    JsonArray cells = dataDoc["Cell_Voltages"].to<JsonArray>();
    for (int i = 0; i < 6; ++i) cells.add(systemData.batt.cellVoltages[i]);

    // BMS temperatures
    JsonArray temps = dataDoc["Cell_Temperatures"].to<JsonArray>();
    for (int i = 0; i < 6; ++i) temps.add(systemData.batt.cellTemperatures[i]);

    // Stautus of components
    dataDoc["Solar_Shunt_Status"] = solarInaStatus;
    dataDoc["Load_Shunt_Status"] = loadInaStatus;
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

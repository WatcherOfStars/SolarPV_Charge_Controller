#ifndef SYSTEM_H
#define SYSTEM_H

#include <Arduino.h>
#include "observer.h"
#include <RTClib.h>
#include "config.h"

//consts
// namespace constants { inline constexpr double PI = 3.14; }
namespace constants {
    static char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
    // device contsants
    static uint8_t NUM_CELLS = 6; // number of cells in the battery pack
    static float MAX_CELL_VOLTAGE = 4.0; // maximum voltage for a single cell
    static float MIN_CELL_VOLTAGE = 3.4; // minimum voltage for a single cell
    static float SAFETY_CELL_VOLTAGE = 3.1; // voltage to trigger safety cutoff
    static float CELL_VOLTAGE_HYSTERESIS = 0.1; // voltage
    static float MAX_CELL_TEMPERATURE = 60.0; // maximum safe temperature for a cell in degrees Celsius
    static float MIN_CELL_TEMPERATURE = -20.0; // minimum safe temperature for a cell in degrees Celsius
    static float MAX_PACK_VOLTAGE = NUM_CELLS * MAX_CELL_VOLTAGE; // maximum voltage for the entire battery pack
    static float MIN_PACK_VOLTAGE = NUM_CELLS * MIN_CELL_VOLTAGE; // minimum voltage for the entire battery pack
    static float SHUNT_RESISTANCE = 0.00075; // shunt resistance in ohms for current measurement, must be > 0.002 for accurate readings
    static float MAX_CURRENT = 20.0; // maximum current in amps for the system
    static float FAN_ON_TEMPERATURE = 40.0; // temperature in degrees Celsius to turn the cooling fan on
    static float FAN_OFF_TEMPERATURE = 35.0; // temperature in degrees Celsius to turn the cooling fan off
    static float LOW_BATTERY_THRESHOLD = 20.0; // state of charge percentage to consider the battery low
    static float HIGH_BATTERY_THRESHOLD = 80.0; // state of charge percentage to consider the battery high    
    static float FULL_BATTERY_THRESHOLD = 95.0; // state of charge percentage to consider the battery full
    static float LOW_BATTERY_DISCHARGE_THRESHOLD = 15.0; // state of charge percentage to cut power to loads
    static float FAN_DUTY_CYCLE = 0.5; // duty cycle for cooling fan when on (0.0 to 1.0)
    
    static uint8_t WIRE_SCL_PIN = 22; // I2C clock pin
    static uint8_t WIRE_SDA_PIN = 21; // I2C data 
    static uint8_t RESTART_PIN = 2; // pin to trigger relay system restart
    static uint8_t SOLAR_FET_PIN = 17; // pin to control solar FETs
    static uint8_t LOAD_FET_PIN = 16; // pin to control load FETs
    static uint8_t FAN_PIN = 4; // pin to control cooling fan
}

struct Sys_Flags {
    unsigned int ENABLE_BMS : 1;
    unsigned int ENABLE_RTC : 1;
    unsigned int ENABLE_SOLAR_FETs : 1;
    unsigned int ENABLE_LOAD_FETs : 1;
    unsigned int ENABLE_FAN : 1;
    unsigned int ENABLE_INA226 : 1;
};

struct BMSData {
    float cellVoltages[6]; // Assuming a 6-cell battery pack
    float cellTemperatures[6]; // Temperature for each cell
    float maxCellVoltage; // Maximum cell voltage in the pack
    float minCellVoltage; // Minimum cell voltage in the pack
    int maxCellIndex; // Index of the cell with the maximum voltage
    int minCellIndex; // Index of the cell with the minimum voltage
    float averageCellVoltage; // Average cell voltage across the pack
    //float stateOfCharge; // State of charge percentage for the battery pack
    //float stateOfHealth; // State of health percentage for the battery pack
    bool isCharging; // Whether the battery is currently charging
    bool isDischarging; // Whether the battery is currently discharging
};

struct SystemData {
    float shuntVoltage; // Shunt voltage in mV
    float shuntCurrent; // Current in A
    float powerUse; // Power in mW
    DateTime rtcTime; // current time from RTC
    BMSData bmsData; // Battery management system data
};

//class definitions
class SystemManager : public observer, public subject {
private:
    std::vector<observer*> observers; //list of observers
    // Set sys_flags
    static Sys_Flags sys_flags; // system flags to control which components are active

    static SystemData systemData; // struct to hold system data for easy access and updates
    
    int setupBMS(); //to be called to setup BMS
    void updateBMS(); //to be called to update BMS
    int setupRTC(); //to be called to setup RTC
    int setupINA226(); //to be called to setup INA226

    // status
    static int ina226Status; // status of INA226 setup (0 = not attempted, -1 = failed, 1 = successful)
    static int rtcStatus; // status of RTC setup (0 = not attempted, -1 = failed, 1 = successful)
    static int bmsStatus; // status of BMS setup (0 = not attempted, -1 = failed, 1 = successful)

    

public:
    void setupSystem(); //to be called in setup
    void updateSystem(); // to be called in loop

    void checkInitWithFlags();

    void sendUpdatesToWebUI(); //to be called to send updates to the web UI

    void registerObserver(observer* obs) override;
    void removeObserver(observer* obs) override;
    void notifyObservers(const char* topic, const char* message) override;

    void getShuntData(); //to be called to get shunt data
    void getRTCData(); //to be called to get RTC data
    void getBMSData(); //to be called to get BMS data
    int performSafetyChecks(); //to be called to perform safety checks
    void solarFETControl(bool state); //to control solar FETs
    void loadFETControl(bool state); //to control load FETs
    void fanControl(bool state); //to control cooling fan
    void onNotify(const char* topic, const char* message) override;

};

#endif
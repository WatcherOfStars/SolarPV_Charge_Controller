#ifndef SYSTEM_H
#define SYSTEM_H

#include <Arduino.h>
#include <observer.h>
#include <RTClib.h>

//consts
// namespace constants { inline constexpr double PI = 3.14; }
namespace constants {
    inline constexpr char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
    
    // device constants
    inline constexpr uint8_t NUM_CELLS = 6; // number of cells in the battery pack
    inline constexpr float MAX_CELL_VOLTAGE = 4.2; // maximum voltage for a single cell
    inline constexpr float MIN_CELL_VOLTAGE = 3.0; // minimum voltage for a single cell
    inline constexpr float MAX_CELL_TEMPERATURE = 60.0; // maximum safe temperature for a cell in degrees Celsius
    inline constexpr float MIN_CELL_TEMPERATURE = -20.0; // minimum safe temperature for a cell in degrees Celsius
    inline constexpr float MAX_PACK_VOLTAGE = NUM_CELLS * MAX_CELL_VOLTAGE; // maximum voltage for the entire battery pack
    inline constexpr float MIN_PACK_VOLTAGE = NUM_CELLS * MIN_CELL_VOLTAGE; // minimum voltage for the entire battery pack
    inline constexpr float SHUNT_RESISTANCE = 0.00075; // shunt resistance in ohms for current measurement  
    inline constexpr float MAX_CURRENT = 20.0; // maximum current in amps for the system
    inline constexpr float FAN_ON_TEMPERATURE = 40.0; // temperature in degrees Celsius to turn the cooling fan on
    inline constexpr float FAN_OFF_TEMPERATURE = 35.0; // temperature in degrees Celsius to turn the cooling fan off
    inline constexpr float LOW_BATTERY_THRESHOLD = 20.0; // state of charge percentage to consider the battery low
    inline constexpr float HIGH_BATTERY_THRESHOLD = 80.0; // state of charge percentage to consider the battery high    
    inline constexpr float FULL_BATTERY_THRESHOLD = 95.0; // state of charge percentage to consider the battery full
    inline constexpr float LOW_BATTERY_DISCHARGE_THRESHOLD = 15.0; // state of charge percentage to cut power to loads
    inline constexpr float FAN_DUTY_CYCLE = 0.5; // duty cycle for cooling fan when on (0.0 to 1.0)

    // pins
    inline constexpr uint8_t BMS_CLK_PIN = 18; // BMS clock pin
    inline constexpr uint8_t BMS_MOSI_PIN = 23; // BMS MOSI pin
    inline constexpr uint8_t BMS_MISO_PIN = 19; // BMS MISO pin
    inline constexpr uint8_t BMS_CS_PIN = 5; // BMS chip select pin
    inline constexpr uint16_t BMS_ADDRESS = 0x80; // BMS I2C address (placeholder, to be updated with actual address)

    inline constexpr uint8_t WIRE_SCL_PIN = 22; // I2C clock pin
    inline constexpr uint8_t WIRE_SDA_PIN = 21; // I2C data pin

    inline constexpr uint8_t RESTART_PIN = 2; // pin to trigger relay system restart
    inline constexpr uint8_t SOLAR_FET_PIN = 17; // pin to control solar FETs
    inline constexpr uint8_t LOAD_FET_PIN = 16; // pin to control load FETs
    inline constexpr uint8_t FAN_PIN = 4; // pin to control cooling fan
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
    float temperatures[6]; // Temperature for each cell
    //float stateOfCharge; // State of charge in percentage
    //float stateOfHealth; // State of health in percentage
    //bool isCharging; // Charging status
    //bool isDischarging; // Discharging status
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
    void notifyObservers(char* topic, char* message) override;

    void getShuntData(); //to be called to get shunt data
    void getRTCData(); //to be called to get RTC data
    void getBMSData(); //to be called to get BMS data
    int performSafetyChecks(); //to be called to perform safety checks
    void solarFETControl(bool state); //to control solar FETs
    void loadFETControl(bool state); //to control load FETs
    void fanControl(bool state); //to control cooling fan
    void onNotify(char* topic, char* message) override;

};

#endif
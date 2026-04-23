#ifndef SYSTEM_H
#define SYSTEM_H

#include <Arduino.h>
#include "observer.h"
#include <RTClib.h>
#include "config.h"


struct Sys_Flags {
    unsigned int ENABLE_BMS : 1;
    unsigned int ENABLE_RTC : 1;
    unsigned int ENABLE_SOLAR_FETs : 1;
    unsigned int ENABLE_LOAD_FETs : 1;
    unsigned int ENABLE_FAN : 1;
    unsigned int ENABLE_SOLAR_INA : 1;
    unsigned int ENABLE_LOAD_INA : 1;
    unsigned int ENABLE_FAKE_BATTERY : 1; // for testing without BMS, will generate fake battery data
};

struct BattData {
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
    float solarShuntVoltage; // Shunt voltage in mV
    float loadShuntVoltage; // Shunt voltage in mV
    float solarShuntCurrent; // Current in A
    float loadShuntCurrent; // Current in A
    float solarPowerUse; // Power in mW
    float loadPowerUse; // Power in mW
    DateTime rtcTime; // current time from RTC
    BattData batt; // Battery management system data
    int error; // error code for system errors
};

//class definitions
class SystemManager : public observer, public subject {
private:
    std::vector<observer*> observers; //list of observers
    // Set sys_flags
    static Sys_Flags sys_flags; // system flags to control which components are active

    static SystemData systemData; // struct to hold system data for easy access and updates
    
    int setupBMS(); //to be called to setup BMS
    int updateBMS(); //to be called to update BMS
    int setupRTC(); //to be called to setup RTC
    int setupSolarINA(); //to be called to setup Solar INA
    int setupLoadINA(); //to be called to setup Load INA

    // status
    static int solarInaStatus; // status of Solar INA setup (0 = not attempted, -1 = failed, 1 = successful)
    static int loadInaStatus; // status of Load INA setup (0 = not attempted, -1 = failed, 1 = successful)
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

    int getSolarShuntData(); //to be called to get shunt data
    int getLoadShuntData(); //to be called to get shunt data
    int getRTCData(); //to be called to get RTC data
    int getBMSData(); //to be called to get BMS data
    int performSafetyChecks(); //to be called to perform safety checks
    void solarFETControl(bool state); //to control solar FETs
    void loadFETControl(bool state); //to control load FETs
    void fanControl(bool state); //to control cooling fan
    void onNotify(const char* topic, const char* message) override;

};

#endif
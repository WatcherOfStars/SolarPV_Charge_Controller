#ifndef SYSTEM_H
#define SYSTEM_H

#include <Arduino.h>

//consts
// namespace constants { inline constexpr double PI = 3.14; }
namespace constants {
    inline constexpr char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
}

//class definitions
class SystemManager {
private:
    void setupBMS(); //to be called to setup BMS
    void updateBMS(); //to be called to update BMS

public:
    void setupSystem(); //to be called in setup
    void updateSystem(); //to be called in loop

    void getShuntData(); //to be called to get shunt data
    void getRTCData(); //to be called to get RTC data
    void getBMSData(); //to be called to get BMS data
    void performSafetyChecks(); //to be called to perform safety checks
};

#endif
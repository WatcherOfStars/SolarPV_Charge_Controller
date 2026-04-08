#ifndef MAIN_H
#define MAIN_H

#include <Arduino.h>
#include <client.h>
#include <web_ui.h>
#include <observer.h>
#include <broker.h>
#include <system.h>
#include <config.h>


// Create global objects
SystemManager sys; // Create System object
BrokerManager broker; // Create BrokerManager object
WebUI webUI; // Create WebUI object
MqttClientManager client; // Create ClientManager object


//consts
// namespace constants { inline constexpr double PI = 3.14; }
namespace constants {
    inline constexpr int SLOW_BOOT = 0; //Delay booting to give time to connect a serial monitor
    inline constexpr int DEBUG_LIGHT = 2; // Pin for debug light
}

//class definitions
class mainEventHandler : public observer {
private:
    // Internal state variables can be added here
public:
    void onNotify(const char* topic, const char* message) override;
};

#endif
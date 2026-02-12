#ifndef MAIN_H
#define MAIN_H

#include <Arduino.h>
#include <client.h>
#include <web_ui.h>


//consts
// namespace constants { inline constexpr double PI = 3.14; }
namespace constants {
    inline constexpr int SLOW_BOOT = 0; //Delay booting to give time to connect a serial monitor
    inline constexpr int DEBUG_LIGHT = 2; // Pin for debug light
}

WebUI webUI; // Create WebUI object
mqttClientManager client; // Create ClientManager object

//class definitions
class mainEventHandler : public webuiClientObserver, public mqttClientObserver {
private:
    // Internal state variables can be added here
public:
    void notifyMQTT(char* topic, char* message) override;
    void notifyWebUI(char* topic, char* message) override;
};

#endif
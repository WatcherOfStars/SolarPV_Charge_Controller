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



//class definitions
class mainEventHandler : public observer {
private:
    // Internal state variables can be added here
public:
    void onNotify(const char* topic, const char* message) override;
};

#endif
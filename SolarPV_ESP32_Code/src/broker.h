#ifndef BROKER_H
#define BROKER_H

#include <Arduino.h>
#include <sMQTTBroker.h>

//consts
// namespace constants { inline constexpr double PI = 3.14; }
namespace constants {
    inline constexpr const char* MQTT_CLIENT_USER = "solarpv"; //username for mqtt clients
    inline constexpr const char* MQTT_CLIENT_PASSWORD = "solarpv123"; //password for mqtt clients
    inline constexpr const short MQTT_PORT = 1883; //port for mqtt broker
}

// // This class extends sMQTTBroker to implement custom event handling and should not be called outside the broker.
class MyBroker : public sMQTTBroker{
public:
    bool onEvent(sMQTTEvent *event) override;
};


// Manages the internal MQTT broker instance and provides setup and update functions.
class BrokerManager {
private:
    MyBroker broker;

public:
    void setupBroker(); //to be called in setup
    void updateBroker(); //to be called in loop
    sMQTTBroker& getBroker(); //returns reference to the internal broker object
};


#endif
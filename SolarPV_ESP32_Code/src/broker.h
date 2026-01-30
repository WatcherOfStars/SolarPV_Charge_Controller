#ifndef BROKER_H
#define BROKER_H

//consts
// namespace constants { inline constexpr double PI = 3.14; }
namespace constants {
    inline constexpr char* MQTT_CLIENT_USER = "solarpv"; //username for mqtt clients
    inline constexpr char* MQTT_CLIENT_PASSWORD = "solarpv123"; //password for mqtt clients
    inline constexpr short MQTT_PORT = 9000; //port for mqtt broker
}

//forward declaration
class sMQTTBroker;
class MyBroker;

//broker manager class prototype
class BrokerManager {
public:
    void setupBroker(); //to be called in setup
    void updateBroker(); //to be called in loop
    sMQTTBroker& getBroker(); //returns reference to the internal broker object
};


#endif
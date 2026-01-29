#ifndef BROKER_H
#define BROKER_H

//consts
// namespace constants { inline constexpr double PI = 3.14; }
namespace constants {
    inline constexpr char* MQTT_CLIENT_USER = "solarpv"; 
    inline constexpr char* MQTT_CLIENT_PASSWORD = "solarpv123";
}

//function prototypes
void setupBroker();
void updateBroker();
sMQTTBroker& getBroker();

//class definitions
class MyBroker; //forward declaration


#endif
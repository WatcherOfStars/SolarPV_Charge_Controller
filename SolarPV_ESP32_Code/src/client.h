#ifndef CLIENT_H
#define CLIENT_H

#include <vector>
#include <PubSubClient.h>

//consts
// namespace constants { inline constexpr double PI = 3.14; }
namespace constants {
    // device constants
    inline constexpr char* MQTT_CLIENT_NAME = "solarpv_client"; //name for mqtt client
    inline constexpr char* MQTT_CLIENT_SUB = "inTopic"; //sub for mqtt clients
    inline constexpr char* MQTT_CLIENT_PUB = "outTopic"; //pub for mqtt clients

    // mqtt broker constants
    inline constexpr char* MQTT_CLIENT_USER = "solarpv"; //username for mqtt clients
    inline constexpr char* MQTT_CLIENT_PASSWORD = "solarpv123"; //password for mqtt clients
    inline constexpr u8_t MQTT_BROKER_ADDRESS[] = {10, 16, 204, 165}; //address of mqtt broker
    inline constexpr short MQTT_PORT = 9000; //port for mqtt broker
}

//observer and subject class prototypes
class mqttClientObserver{ //todo: do we need aliases for topic and message types?
public:
    virtual void notifyMQTT(char* topic, char* message) = 0; //function to notify observers of incoming messages. 
    virtual ~mqttClientObserver() = default; //destructor
};

class mqttClientSubject{
public:
    virtual void registerObserver(mqttClientObserver* obs) = 0;
    virtual void removeObserver(mqttClientObserver* obs) = 0;
    virtual void notifyObservers(char* topic, char* message) = 0;
    virtual ~mqttClientSubject() = default; //destructor
};

//class prototype
class mqttClientManager : public mqttClientSubject {
private:
    std::vector<mqttClientObserver*> observers; //list of observers

public:
    void setupClient(); //to be called in setup
    void updateClient(); //to be called in loop
    PubSubClient* getClient(); //returns reference to the internal client object

    void registerObserver(mqttClientObserver* obs) override;
    void removeObserver(mqttClientObserver* obs) override;
    void notifyObservers(char* topic, char* message) override;
};

#endif
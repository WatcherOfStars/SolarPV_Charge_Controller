#ifndef CLIENT_H
#define CLIENT_H

#include <Arduino.h>
#include <vector>
#include "ESP32MQTTClient.h"
#include "esp_idf_version.h" // check IDF version
#include <observer.h>
#include <config.h>
//#include <cstdint>



//class prototype
class MqttClientManager : public subject {
private:
    std::vector<observer*> observers; //list of observers

public:
    void setupClient(); //to be called in setup
    ESP32MQTTClient* getClient(); //returns reference to the internal client object

    void subscribeToTopic(const char* topic); //subscribe to a topic, for testing purposes
    void publishToTopic(const char* topic, const char* message); //publish a message to a topic, for testing purposes
    void registerObserver(observer* obs) override;
    void removeObserver(observer* obs) override;
    void notifyObservers(const char* topic, const char* message) override;
};

#endif
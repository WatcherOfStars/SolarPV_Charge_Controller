#include <client.h>

#include <WiFi.h>
#include <iostream>
#include <algorithm>

using namespace constants;
using namespace std;


// Empty callback function for MQTT client
void callback(char* topic, byte* payload, unsigned int length);


IPAddress server(MQTT_BROKER_ADDRESS); //TODO MOVE TO .h
WiFiClient wificlient; //TODO MAY NOT WORK
PubSubClient pub_sub_client(server, MQTT_PORT, callback, wificlient); //internal MQTT client instance


// ========== MQTT Client manager ==========
// Manages the internal MQTT client instance and provides setup and update functions.
// virtual ~mqttClientManager() noexcept = default;

void mqttClientManager::setupClient(){
    // Set callback to notify observers instead of empty callback
    auto my_callback = [this](char* topic, byte* payload, unsigned int length) { this->notifyObservers(topic, (char*)payload); };
    pub_sub_client.setCallback(my_callback);

    // Connect to the MQTT broker
    if (pub_sub_client.connect(MQTT_CLIENT_NAME, MQTT_CLIENT_USER, MQTT_CLIENT_PASSWORD)) {
        std::cout << "Connected to MQTT broker" << std::endl;
        pub_sub_client.publish(MQTT_CLIENT_PUB,"hello world");
        pub_sub_client.subscribe(MQTT_CLIENT_SUB);
    } else {
        std::cout << "Failed to connect to MQTT broker" << std::endl;
    }
}

void mqttClientManager::updateClient(){
    // Update MQTT client here
    pub_sub_client.loop();
}

PubSubClient* mqttClientManager::getClient(){
    // Return reference to internal client object here
    return &pub_sub_client;
}

// Observer pattern methods
void mqttClientManager::registerObserver(mqttClientObserver* obs) {
    observers.push_back(obs);
}
void mqttClientManager::removeObserver(mqttClientObserver* obs) {
    observers.erase(std::remove(observers.begin(), observers.end(), obs), observers.end());
}
void mqttClientManager::notifyObservers(char* topic, char* message) {
    for (auto& obs : observers) {
        obs->notifyMQTT(topic, message);
    }
}
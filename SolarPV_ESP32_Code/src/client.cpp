#include <client.h>

#include <WiFi.h>
#include <iostream>
#include <algorithm>

using namespace constants;
using namespace std;



// ========== MQTT Client manager ==========
ESP32MQTTClient mqttClient; // all params are set later
// Manages the internal MQTT client instance and provides setup and update functions.

void mqttClientManager::setupClient(){ //Client &set_client
    // Set callback to notify observers instead of empty callback
    //auto my_callback = [this](char* topic, uint8_t* payload, unsigned int length) { this->notifyObservers(topic, (char*)payload); };

    mqttClient.enableDebuggingMessages();
    mqttClient.setMqttClientName(("solarpv-client-"+WiFi.macAddress()).c_str()); // set client name to something unique using the MAC address

    std::cout << "Connecting to MQTT broker at " << C_MQTT_BROKER_ADDRESS << std::endl;
    std::cout << "Using username: " << C_MQTT_CLIENT_USER << std::endl;
    std::cout << "Using password: " << C_MQTT_CLIENT_PASSWORD << std::endl;
    std::cout << "Using client name: " << mqttClient.getClientName() << std::endl;

    mqttClient.setURI(C_MQTT_BROKER_ADDRESS, C_MQTT_CLIENT_USER, C_MQTT_CLIENT_PASSWORD); // set broker address and credentials
    mqttClient.enableLastWillMessage("lwt", "I am going offline");
    mqttClient.setKeepAlive(30);
    // mqttClient.subscribe(CLIENT_SUB, [this](const std::string &topic, const std::string &payload) { // set callback to notify observers instead of empty callback
    //     this->notifyObservers((char*)topic.c_str(), (char*)payload.c_str());
    // });
    mqttClient.setOnMessageCallback([this](const std::string &topic, const std::string &payload) { // set callback to notify observers instead of empty callback
        notifyObservers((char*)topic.c_str(), (char*)payload.c_str());
    });

    mqttClient.loopStart();
}

ESP32MQTTClient* mqttClientManager::getClient(){
    // Return reference to internal client object here
    return &mqttClient;
}

void mqttClientManager::subscribeToTopic(const char* topic){
    mqttClient.subscribe(topic, [this](const std::string &topic, const std::string &payload) { // set callback to notify observers instead of empty callback
        this->notifyObservers((char*)topic.c_str(), (char*)payload.c_str());
    });
}

void mqttClientManager::publishToTopic(const char* topic, const char* message){
    mqttClient.publish(topic, message);
}

// for testing purposes, subscribe to a topic on connect and print received messages to serial
void onMqttConnect(esp_mqtt_client_handle_t client)
{
    if (mqttClient.isMyTurn(client)) // can be omitted if only one client
    {
        std::cout << "MQTT client connected! Subscribed to topic: " << constants::CLIENT_SUB << std::endl;
        mqttClient.subscribe(constants::CLIENT_SUB, [](const std::string &topic, const std::string &payload)
                             { log_i("%s: %s", topic.c_str(), payload.c_str()); });
    }
}

#if ESP_IDF_VERSION < ESP_IDF_VERSION_VAL(5, 0, 0)
esp_err_t handleMQTT(esp_mqtt_event_handle_t event)
{
    mqttClient.onEventCallback(event);
    return ESP_OK;
}
#else  // IDF CHECK
void handleMQTT(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
    auto *event = static_cast<esp_mqtt_event_handle_t>(event_data);
    mqttClient.onEventCallback(event);
}
#endif // // IDF CHECK

// Observer pattern methods
void mqttClientManager::registerObserver(mqttClientObserver* obs) {
    std::cout << "Registering MQTT Client Observer " << obs << std::endl;
    observers.push_back(obs);
}
void mqttClientManager::removeObserver(mqttClientObserver* obs) {
    observers.erase(std::remove(observers.begin(), observers.end(), obs), observers.end());
}
void mqttClientManager::notifyObservers(char* topic, char* message) {
    std::cout << "Notifying MQTT Client Observers for topic: " << topic << std::endl;
    std::cout << "Message: " << message << std::endl;   
    for (auto& obs : observers) {
        obs->notifyMQTT(topic, message);
    }
}
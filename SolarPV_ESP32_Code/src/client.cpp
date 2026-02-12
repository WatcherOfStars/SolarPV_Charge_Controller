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
    mqttClient.setMqttClientName(("solarpv-client-"+WiFi.macAddress()).c_str());

    //std::cout << "Connecting to MQTT broker at " << C_MQTT_BROKER_ADDRESS << ":" << C_MQTT_PORT << std::endl;
    //std::cout << "Using client name: " << CLIENT_NAME << std::endl;
    //std::cout << "Using username: " << C_MQTT_CLIENT_USER << std::endl;
    //std::cout << "Using password: " << C_MQTT_CLIENT_PASSWORD << std::endl;

    std::cout << "Connecting to MQTT broker at " << C_MQTT_BROKER_ADDRESS << std::endl;
    std::cout << "Using username: " << C_MQTT_CLIENT_USER << std::endl;
    std::cout << "Using password: " << C_MQTT_CLIENT_PASSWORD << std::endl;
    std::cout << "Using client name: " << mqttClient.getClientName() << std::endl;

    mqttClient.setURI(C_MQTT_BROKER_ADDRESS, C_MQTT_CLIENT_USER, C_MQTT_CLIENT_PASSWORD);
    mqttClient.enableLastWillMessage("lwt", "I am going offline");
    mqttClient.setKeepAlive(30);
    mqttClient.setOnMessageCallback([this](const std::string &topic, const std::string &payload) {
        notifyObservers((char*)topic.c_str(), (char*)payload.c_str());
    });

    mqttClient.loopStart();
}

void mqttClientManager::updateClient(){
    // Update MQTT client here
}

ESP32MQTTClient* mqttClientManager::getClient(){
    // Return reference to internal client object here
    return &mqttClient;
}

void onMqttConnect(esp_mqtt_client_handle_t client)
{
    if (mqttClient.isMyTurn(client)) // can be omitted if only one client
    {
        mqttClient.subscribe("test/topic", [](const std::string &payload)
                             { log_i("%s: %s", subscribeTopic, payload.c_str()); });

        mqttClient.subscribe(CLIENT_SUB, [](const std::string &topic, const std::string &payload)
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
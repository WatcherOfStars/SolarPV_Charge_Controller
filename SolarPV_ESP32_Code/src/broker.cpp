#include <broker.h>

#include <WiFi.h>
#include <iostream>
#include <algorithm>

using namespace std;

String username;
String password;

// Handle MQTT events, including client connection, wifi disconnects, subscriptions, etc.
bool MyBroker::onEvent(sMQTTEvent *event)
{
    switch(event->Type())
    {
    case NewClient_sMQTTEventType:
        {
            sMQTTNewClientEvent *e=(sMQTTNewClientEvent*)event;
            // Check username and password used for new connection
            if ((e->Login() != username.c_str()) || (e->Password() != password.c_str())) {
                Serial.println("Invalid username or password");  
                return false;
                }
        };
        break;
    case LostConnect_sMQTTEventType:
        WiFi.reconnect();
        break;
    case UnSubscribe_sMQTTEventType:
    case Subscribe_sMQTTEventType:
        {
            sMQTTSubUnSubClientEvent *e=(sMQTTSubUnSubClientEvent*)event;
        }
        break;
    }
    return true;
}


// Initialize the MQTT broker on the specified port.
void BrokerManager::setupBroker(){
    broker.init(ConfigManager::getInstance().mqttConfig.port);
    username = ConfigManager::getInstance().mqttConfig.username;
    password = ConfigManager::getInstance().mqttConfig.password;
    std::cout << "MQTT Broker initialized on port " << ConfigManager::getInstance().mqttConfig.port << std::endl;
}

// Update broker, should be called in main loop.
void BrokerManager::updateBroker(){
    broker.update();
    //std::cout << "MQTT Broker updated on port " << config.mqttConfig.port << std::endl;
}

// Get reference to the internal broker instance.
sMQTTBroker& BrokerManager::getBroker(){
    return broker;
}

#include <broker.h>

#include <WiFi.h>
#include <iostream>
#include <algorithm>

using namespace constants;
using namespace std;

// Handle MQTT events, including client connection, wifi disconnects, subscriptions, etc.
bool MyBroker::onEvent(sMQTTEvent *event)
{
    switch(event->Type())
    {
    case NewClient_sMQTTEventType:
        {
            sMQTTNewClientEvent *e=(sMQTTNewClientEvent*)event;
            // Check username and password used for new connection
            if ((e->Login() != MQTT_CLIENT_USER) || (e->Password() != MQTT_CLIENT_PASSWORD)) {
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
    broker.init(MQTT_PORT);
    std::cout << "MQTT Broker initialized on port " << MQTT_PORT << std::endl;
}

// Update broker, should be called in main loop.
void BrokerManager::updateBroker(){
    broker.update();
    //std::cout << "MQTT Broker updated on port " << MQTT_PORT << std::endl;
}

// Get reference to the internal broker instance.
sMQTTBroker& BrokerManager::getBroker(){
    return broker;
}

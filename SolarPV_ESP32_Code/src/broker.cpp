#include <broker.h>

#include <WiFi.h>

using namespace constants;

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


// ========== MQTT Broker manager ==========
// Manages the internal MQTT broker instance and provides setup and update functions.
class BrokerManager{
private:
    MyBroker broker;

public:
    // Initialize the MQTT broker on the specified port.
    void setupBroker(){
        broker.init(MQTT_PORT);
    }

    // Update broker, should be called in main loop.
    void updateBroker(){
        broker.update();
    }

    // Get reference to the internal broker instance.
    sMQTTBroker& getBroker(){
        return broker;
    }
};
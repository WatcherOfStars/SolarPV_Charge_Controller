#include <Arduino.h>
#include <EEPROM.h>
#include <WiFi.h>
#include <ESPmDNS.h>
#include <sMQTTBroker.h>
#include <vars.h>

// ========== MQTT Broker class ==========

class MyBroker:public sMQTTBroker
{
public:
    bool onEvent(sMQTTEvent *event) override
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
};


MyBroker broker;


void setupBroker(){
    const unsigned short mqttPort=9000;
    broker.init(mqttPort);
}

void updateBroker(){
    broker.update();
}
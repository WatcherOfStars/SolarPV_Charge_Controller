#ifndef WEB_UI_H
#define WEB_UI_H

#include <Arduino.h>
#include <ESPUI.h>
#include <WiFi.h>
#include <broker.h>
#include <vector>
#include "observer.h"


//consts
// namespace constants { inline constexpr double PI = 3.14; }
namespace constants {
    inline constexpr const char* HOSTNAME = "ESPUITest";
    inline constexpr int FORCE_USE_HOTSPOT = 0;
}

// //observer and subject class prototypes
// class webuiClientObserver{ //todo: do we need aliases for topic and message types?
// public:
//     virtual void notifyWebUI(char* topic, char* message) = 0; //function to notify observers of incoming messages. 
//     virtual ~webuiClientObserver() = default; //destructor
// };

// class webuiClientSubject{
// public:
//     virtual void registerObserver(webuiClientObserver* obs) = 0;
//     virtual void removeObserver(webuiClientObserver* obs) = 0;
//     virtual void notifyObservers(char* topic, char* message) = 0;
//     virtual ~webuiClientSubject() = default; //destructor
// };


//class prototype
class WebUI : public subject, public observer {
public:
    void setupWebConn();
    void updateWebUI();
    void setupWebUI();
    void connectWifi();
    void setBroker(BrokerManager* brokerManager){
        this->broker = brokerManager;
    };

    void registerObserver(observer* obs) override;
    void removeObserver(observer* obs) override;
    void notifyObservers(char* topic, char* message) override;

    void onNotify(char* topic, char* message) override; //function to notify webui of incoming mqtt messages.


private:
    std::vector<observer*> observers;
    BrokerManager* broker; //pointer to the broker manager

    //UI handles
    uint16_t wifi_ssid_text, wifi_pass_text;
    uint16_t main_button;
    uint16_t mainSwitcher, test_message_text, mainTime;

    // prototypes
    void generalCallback(Control *sender, int type);
    void getTimeCallback(Control *sender, int type);
    void updateObserversCallback(Control *sender, int type);
    void startClientCallback(Control *sender, int type);

    //utility prototypes
    void readStringFromEEPROM(String& buf, int baseaddress, int size);
    
    //custom callbacks
    void enterWifiDetailsCallback(Control *sender, int type);
    void textCallback(Control *sender, int type);
    void sendTestPub(Control *sender, int type);

    
};



//class definitions

#endif
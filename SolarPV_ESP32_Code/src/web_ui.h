#ifndef WEB_UI_H
#define WEB_UI_H

#include <Arduino.h>
#include <ESPUI.h>
#include <WiFi.h>
#include <client.h>
#include <broker.h>
#include <vector>


//consts
// namespace constants { inline constexpr double PI = 3.14; }
namespace constants {
    inline constexpr const char* HOSTNAME = "ESPUITest";
    inline constexpr int FORCE_USE_HOTSPOT = 0;
}

//observer and subject class prototypes
class webuiClientObserver{ //todo: do we need aliases for topic and message types?
public:
    virtual void notifyWebUI(char* topic, char* message) = 0; //function to notify observers of incoming messages. 
    virtual ~webuiClientObserver() = default; //destructor
};

class webuiClientSubject{
public:
    virtual void registerObserver(webuiClientObserver* obs) = 0;
    virtual void removeObserver(webuiClientObserver* obs) = 0;
    virtual void notifyObservers(char* topic, char* message) = 0;
    virtual ~webuiClientSubject() = default; //destructor
};


//class prototype
class WebUI : public webuiClientSubject, public mqttClientObserver {
public:
    void setupWebConn();
    void updateWeb();
    void setupWebUI();
    void connectWifi();
    void setBroker(BrokerManager* brokerManager){
        this->broker = brokerManager;
    };

    void registerObserver(webuiClientObserver* obs) override;
    void removeObserver(webuiClientObserver* obs) override;
    void notifyObservers(char* topic, char* message) override;

    void notifyMQTT(char* topic, char* message) override; //function to notify webui of incoming mqtt messages.


private:
    std::vector<webuiClientObserver*> observers;
    BrokerManager* broker; //pointer to the broker manager

    //UI handles
    uint16_t wifi_ssid_text, wifi_pass_text;
    uint16_t send_test_pub_button, send_test_to_subjects_button, start_client_button;
    uint16_t mainLabel, mainSwitcher, mainSlider, test_message_test, mainNumber, mainScrambleButton, mainTime;

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
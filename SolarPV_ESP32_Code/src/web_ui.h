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
    inline constexpr const char* HOSTNAME = "ESPUITest2";
    inline constexpr int FORCE_USE_HOTSPOT = 0;
}


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
    void notifyObservers(const char* topic, const char* message) override;

    void onNotify(const char* topic, const char* message) override; //function to notify webui of incoming mqtt messages.


private:
    std::vector<observer*> observers;
    BrokerManager* broker; //pointer to the broker manager

    //UI handles
    uint16_t wifi_ssid_text, wifi_pass_text;
    uint16_t main_button;
    //control switches
    uint16_t toggle_solar_switcher, toggle_load_switcher, toggle_fan_switcher;
    //system flag switches
    uint16_t enable_bms_switcher, enable_rtc_switcher, enable_ina226_switcher, enable_solar_switcher, enable_load_switcher, enable_fan_switcher;
    //values
    uint16_t rtc_time_label, shunt_voltage_label, current_label, cell_voltages_label, cell_temperatures_label;
    //status labels
    uint16_t ina226_status_label, rtc_status_label, bms_status_label;
    //other
    uint16_t test_message_text, mainTime;

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
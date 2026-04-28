#ifndef WEB_UI_H
#define WEB_UI_H

#include <Arduino.h>
#include <ESPUI.h>
#include <WiFi.h>
#include <broker.h>
#include <vector>
#include "observer.h"
#include "config.h"



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
    static constexpr const char* kConfigPath = "/config.json";
    static constexpr const char* kConfigUploadTempPath = "/config_upload.tmp";
    static constexpr size_t kMaxConfigUploadBytes = 32768;

    std::vector<observer*> observers;
    BrokerManager* broker; //pointer to the broker manager
    bool configUploadFailed = false;
    size_t configUploadBytes = 0;
    String configUploadError;

    //UI handles
    uint16_t wifi_ssid_text, wifi_pass_text;
    uint16_t main_button;
    //control switches
    uint16_t toggle_solar_switcher, toggle_load_switcher, toggle_fan_switcher;
    //system flag switches
    uint16_t enable_bms_switcher, enable_rtc_switcher, enable_solar_ina_switcher, enable_load_ina_switcher, enable_solar_switcher, enable_load_switcher, enable_fan_switcher;
    //values
    uint16_t rtc_time_label, solar_current_label, load_current_label, cell_voltages_label, cell_temperatures_label;
    //status labels
    uint16_t solar_shunt_status_label, load_shunt_status_label, rtc_status_label, bms_status_label, dns_status_label;
    //divider sections
    uint16_t control_buttons_section, fet_toggle_section, system_flags_section, system_data_section, component_status_section;
    //other
    uint16_t test_message_text, mainTime, testVoltageSlider;

    // prototypes
    void generalCallback(Control *sender, int type);
    void getTimeCallback(Control *sender, int type);
    void updateObserversCallback(Control *sender, int type);
    void startClientCallback(Control *sender, int type);

    //utility prototypes
    void readStringFromEEPROM(String& buf, int baseaddress, int size);
    bool validateConfigJsonFile(const char* path, String& errorMessage);
    void registerConfigEndpoints();
    
    //custom callbacks
    void enterWifiDetailsCallback(Control *sender, int type);
    void textCallback(Control *sender, int type);
    void sendTestPub(Control *sender, int type);

    
};



//class definitions

#endif
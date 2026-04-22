#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>
#include <ArduinoJson.h>

struct WifiConfig {
    String hostname;
    bool force_use_hotspot;
    String ssid;
    String password;
};

struct MqttConfig {
    String broker;
    uint16_t port;
    String username;
    String password;
    String data_topic;
    String command_topic;
    String status_topic;
    String client_id;
};

struct DeviceConfig {
    String device_id;
    bool slow_boot;
    int num_cells;
    float max_cell_voltage;
    float min_cell_voltage;
    float safety_cell_voltage;
    float cell_voltage_hysteresis;
    float max_cell_temperature;
    float min_cell_temperature;
    float max_pack_voltage;
    float min_pack_voltage;
    float shunt_resistance;
    float max_current;
    float fan_on_temperature;
    float fan_off_temperature;
    float low_battery_threshold;
    float high_battery_threshold;
    float full_battery_threshold;
    float low_battery_discharge_threshold;
    float fan_duty_cycle;
    int bms_clk_pin;
    int bms_mosi_pin;
    int bms_miso_pin;
    int bms_cs_pin;
    int wire_scl_pin;
    int wire_sda_pin;
    int restart_pin;
    int solar_fet_pin;
    int load_fet_pin;
    int fan_pin;
};


class ConfigManager {
public:
    // singleton
    static ConfigManager& getInstance(){
        static ConfigManager instance;
        return instance;
    }

    ConfigManager(const ConfigManager&) = delete; // delete copy constructor
    ConfigManager& operator=(const ConfigManager&) = delete; // delete copy assignment operator
    
    static WifiConfig wifiConfig;
    static MqttConfig mqttConfig;
    static DeviceConfig deviceConfig;

    void loadConfig(const char* filename);
    void printConfig();
    void writeConfig(const char* filename, const WifiConfig& wifiConfig, const MqttConfig& mqttConfig, const DeviceConfig& deviceConfig);
    void writeConfig(const char* filename, JsonDocument config);

private:
    ConfigManager() {} // private constructor for singleton
};

#endif
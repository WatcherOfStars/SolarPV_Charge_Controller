#include <config.h>
#include <LittleFS.h>

WifiConfig ConfigManager::wifiConfig;
MqttConfig ConfigManager::mqttConfig;
DeviceConfig ConfigManager::deviceConfig;

void ConfigManager::loadConfig(const char* filename) {
    // Load configuration from JSON file in SPIFFS
    // This is a placeholder implementation and should be expanded to actually read from the file system
    Serial.println("Loading configuration from file...");
    if (!LittleFS.begin(true)) {
        Serial.println("Failed to mount LittleFS");
        return;
    }
    File configFile = LittleFS.open(filename, "r");
    if (!configFile) {
        Serial.println("Failed to open config file");
        return;
    }
    while (configFile.available()) {
        Serial.print((char)configFile.read());
    }
    configFile.seek(0); // Reset position to beginning for deserialization
    size_t size = configFile.size();
    Serial.print("Config file size: ");
    Serial.println(size);
    // if (size > 1024) {
    //     Serial.println("Config file size is too large");
    //     return;
    // }
    std::unique_ptr<char[]> buf(new char[size]);
    configFile.readBytes(buf.get(), size);
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, buf.get());
    configFile.close();
    if (error) {
        Serial.print("Failed to parse config file: ");
        Serial.println(error.c_str());
        return;
    }
    LittleFS.end();

    // Populate config structs from JSON
    wifiConfig.hostname = doc["wifi_config"]["hostname"];
    wifiConfig.force_use_hotspot = doc["wifi_config"]["force_use_hotspot"];
    wifiConfig.ssid = doc["wifi_config"]["ssid"];
    wifiConfig.password = doc["wifi_config"]["password"];

    mqttConfig.broker = doc["mqtt_config"]["broker"];
    mqttConfig.port = doc["mqtt_config"]["port"];
    mqttConfig.username = doc["mqtt_config"]["username"];
    mqttConfig.password = doc["mqtt_config"]["password"];
    mqttConfig.data_topic = doc["mqtt_config"]["data_topic"];
    mqttConfig.command_topic = doc["mqtt_config"]["command_topic"];
    mqttConfig.status_topic = doc["mqtt_config"]["status_topic"];
    mqttConfig.client_id = doc["mqtt_config"]["client_id"];

    deviceConfig.device_id = doc["device_config"]["device_id"];
    deviceConfig.slow_boot = doc["device_config"]["slow_boot"];

    deviceConfig.num_cells = doc["device_config"]["num_cells"];
    deviceConfig.max_cell_voltage = doc["device_config"]["max_cell_voltage"];
    deviceConfig.min_cell_voltage = doc["device_config"]["min_cell_voltage"];
    deviceConfig.max_cell_temperature = doc["device_config"]["max_cell_temperature"];
    deviceConfig.min_cell_temperature = doc["device_config"]["min_cell_temperature"];
    deviceConfig.max_pack_voltage = doc["device_config"]["max_pack_voltage"];
    deviceConfig.min_pack_voltage = doc["device_config"]["min_pack_voltage"];

    deviceConfig.shunt_resistance = doc["device_config"]["shunt_resistance"];
    deviceConfig.max_current = doc["device_config"]["max_current"];

    deviceConfig.fan_on_temperature = doc["device_config"]["fan_on_temperature"];
    deviceConfig.fan_off_temperature = doc["device_config"]["fan_off_temperature"];
    deviceConfig.low_battery_threshold = doc["device_config"]["low_battery_threshold"];
    deviceConfig.high_battery_threshold = doc["device_config"]["high_battery_threshold"];
    deviceConfig.full_battery_threshold = doc["device_config"]["full_battery_threshold"];
    deviceConfig.low_battery_discharge_threshold = doc["device_config"]["low_battery_discharge_threshold"];

    deviceConfig.fan_duty_cycle = doc["device_config"]["fan_duty_cycle"];

    deviceConfig.bms_clk_pin = doc["device_config"]["bms_clk_pin"];
    deviceConfig.bms_mosi_pin = doc["device_config"]["bms_mosi_pin"];
    deviceConfig.bms_miso_pin = doc["device_config"]["bms_miso_pin"];
    deviceConfig.bms_cs_pin = doc["device_config"]["bms_cs_pin"];
    deviceConfig.bms_address = doc["device_config"]["bms_address"];

    deviceConfig.wire_scl_pin = doc["device_config"]["wire_scl_pin"];
    deviceConfig.wire_sda_pin = doc["device_config"]["wire_sda_pin"];

    deviceConfig.restart_pin = doc["device_config"]["restart_pin"];
    deviceConfig.solar_fet_pin = doc["device_config"]["solar_fet_pin"];
    deviceConfig.load_fet_pin = doc["device_config"]["load_fet_pin"];
    deviceConfig.fan_pin = doc["device_config"]["fan_pin"];
}

void ConfigManager::printConfig() {
    //loadConfig("/config.json"); // Load config before printing
    // Print current configuration to serial
    Serial.println("Current Configuration:");
    Serial.print("Hostname: ");
    Serial.println(wifiConfig.hostname);
    Serial.print("Force Use Hotspot: ");
    Serial.println(wifiConfig.force_use_hotspot);
    Serial.print("SSID: ");
    Serial.println(wifiConfig.ssid);
    Serial.print("Password: ");
    Serial.println(wifiConfig.password);

    Serial.print("MQTT Broker: ");
    Serial.println(mqttConfig.broker);
    Serial.print("MQTT Port: ");
    Serial.println(mqttConfig.port);
    Serial.print("MQTT Username: ");
    Serial.println(mqttConfig.username);
    Serial.print("MQTT Password: ");
    Serial.println(mqttConfig.password);
    Serial.print("MQTT Data Topic: ");
    Serial.println(mqttConfig.data_topic);
    Serial.print("MQTT Command Topic: ");
    Serial.println(mqttConfig.command_topic);
    Serial.print("MQTT Status Topic: ");
    Serial.println(mqttConfig.status_topic);
    Serial.print("MQTT Client ID: ");
    Serial.println(mqttConfig.client_id);

    Serial.print("Device ID: ");
    Serial.println(deviceConfig.device_id);
    Serial.print("Slow Boot: ");
    Serial.println(deviceConfig.slow_boot);

    // Print other device config parameters as needed
}

void ConfigManager::writeConfig(const char* filename, const WifiConfig& wifiConfig, const MqttConfig& mqttConfig, const DeviceConfig& deviceConfig) {
    // Write configuration to JSON file in SPIFFS
    // This is a placeholder implementation and should be expanded to actually write to the file system
    Serial.println("Writing configuration to file...");
    JsonDocument doc;
    doc["wifi_config"]["hostname"] = wifiConfig.hostname;
    doc["wifi_config"]["force_use_hotspot"] = wifiConfig.force_use_hotspot;
    doc["wifi_config"]["ssid"] = wifiConfig.ssid;
    doc["wifi_config"]["password"] = wifiConfig.password;

    doc["mqtt_config"]["broker"] = mqttConfig.broker;
    doc["mqtt_config"]["port"] = mqttConfig.port;
    doc["mqtt_config"]["username"] = mqttConfig.username;
    doc["mqtt_config"]["password"] = mqttConfig.password;
    doc["mqtt_config"]["data_topic"] = mqttConfig.data_topic;
    doc["mqtt_config"]["command_topic"] = mqttConfig.command_topic;
    doc["mqtt_config"]["status_topic"] = mqttConfig.status_topic;
    doc["mqtt_config"]["client_id"] = mqttConfig.client_id;

    doc["device_config"]["device_id"] = deviceConfig.device_id;
    doc["device_config"]["slow_boot"] = deviceConfig.slow_boot;

    doc["device_config"]["num_cells"] = deviceConfig.num_cells;
    doc["device_config"]["max_cell_voltage"] = deviceConfig.max_cell_voltage;
    doc["device_config"]["min_cell_voltage"] = deviceConfig.min_cell_voltage;
    doc["device_config"]["max_cell_temperature"] = deviceConfig.max_cell_temperature;
    doc["device_config"]["min_cell_temperature"] = deviceConfig.min_cell_temperature;
    doc["device_config"]["max_pack_voltage"] = deviceConfig.max_pack_voltage;
    doc["device_config"]["min_pack_voltage"] = deviceConfig.min_pack_voltage;

    doc["device_config"]["shunt_resistance"] = deviceConfig.shunt_resistance;
    doc["device_config"]["max_current"] = deviceConfig.max_current;

    doc["device_config"]["fan_on_temperature"] = deviceConfig.fan_on_temperature;
    doc["device_config"]["fan_off_temperature"] = deviceConfig.fan_off_temperature;
    doc["device_config"]["low_battery_threshold"] = deviceConfig.low_battery_threshold;
    doc["device_config"]["high_battery_threshold"] = deviceConfig.high_battery_threshold;
    doc["device_config"]["full_battery_threshold"] = deviceConfig.full_battery_threshold;
    doc["device_config"]["low_battery_discharge_threshold"] = deviceConfig.low_battery_discharge_threshold;
    doc["device_config"]["fan_duty_cycle"] = deviceConfig.fan_duty_cycle;
    doc["device_config"]["bms_clk_pin"] = deviceConfig.bms_clk_pin;
    doc["device_config"]["bms_mosi_pin"] = deviceConfig.bms_mosi_pin;
    doc["device_config"]["bms_miso_pin"] = deviceConfig.bms_miso_pin;
    doc["device_config"]["bms_cs_pin"] = deviceConfig.bms_cs_pin;
    doc["device_config"]["bms_address"] = deviceConfig.bms_address;
    doc["device_config"]["wire_scl_pin"] = deviceConfig.wire_scl_pin;
    doc["device_config"]["wire_sda_pin"] = deviceConfig.wire_sda_pin;
    doc["device_config"]["restart_pin"] = deviceConfig.restart_pin;
    doc["device_config"]["solar_fet_pin"] = deviceConfig.solar_fet_pin;
    doc["device_config"]["load_fet_pin"] = deviceConfig.load_fet_pin;
    doc["device_config"]["fan_pin"] = deviceConfig.fan_pin;

    // Serialize JSON to string and write to file
    File configFile = LittleFS.open(filename, "w");
    if (!configFile) {
        Serial.println("Failed to open config file for writing");
        return;
    }
    serializeJson(doc, configFile);
    configFile.close();
}

void ConfigManager::writeConfig(const char* filename, JsonDocument config) {
    // Write configuration to JSON file in LittleFS
    Serial.println("Writing configuration to file...");
    File configFile = LittleFS.open(filename, "w");
    if (!configFile) {
        Serial.println("Failed to open config file for writing");
        return;
    }
    serializeJson(config, configFile);
    configFile.close();
}
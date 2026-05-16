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
        Serial.println("ERROR: Failed to open config file");
        LittleFS.end();
        return;
    }
    
    while (configFile.available()) {
        Serial.print((char)configFile.read());
    }
    configFile.seek(0); // Reset position to beginning for deserialization
    size_t size = configFile.size();
    Serial.print("Config file size: ");
    Serial.println(size);
    
    std::unique_ptr<char[]> buf(new char[size]);
    configFile.readBytes(buf.get(), size);
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, buf.get());
    configFile.close();
    
    if (error) {
        Serial.print("ERROR: Failed to parse config file: ");
        Serial.println(error.c_str());
        LittleFS.end();
        return;
    }
    LittleFS.end();

    // Populate config structs from JSON
    wifiConfig.hostname = doc["wifi_config"]["hostname"].as<String>();
    wifiConfig.force_use_hotspot = doc["wifi_config"]["force_use_hotspot"].as<bool>();
    wifiConfig.password = doc["wifi_config"]["password"].as<String>();

    mqttConfig.broker = doc["mqtt_config"]["broker"].as<String>();
    mqttConfig.port = doc["mqtt_config"]["port"].as<int>();
    mqttConfig.username = doc["mqtt_config"]["username"].as<String>();
    mqttConfig.password = doc["mqtt_config"]["password"].as<String>();
    mqttConfig.data_topic = doc["mqtt_config"]["data_topic"].as<String>();
    mqttConfig.command_topic = doc["mqtt_config"]["command_topic"].as<String>();
    mqttConfig.status_topic = doc["mqtt_config"]["status_topic"].as<String>();
    mqttConfig.client_id = doc["mqtt_config"]["client_id"].as<String>();

    deviceConfig.device_id = doc["device_config"]["device_id"].as<String>();
    deviceConfig.slow_boot = doc["device_config"]["slow_boot"].as<bool>();

    deviceConfig.num_cells = doc["device_config"]["num_cells"].as<int>();
    deviceConfig.max_cell_voltage = doc["device_config"]["max_cell_voltage"].as<float>();
    deviceConfig.min_cell_voltage = doc["device_config"]["min_cell_voltage"].as<float>();
    deviceConfig.safety_cell_voltage = doc["device_config"]["safety_cell_voltage"].as<float>();
    deviceConfig.cell_voltage_hysteresis = doc["device_config"]["cell_voltage_hysteresis"].as<float>();
    deviceConfig.cell_balance_start = doc["device_config"]["cell_balance_start"].as<float>();
    deviceConfig.max_cell_temperature = doc["device_config"]["max_cell_temperature"].as<float>();
    deviceConfig.min_cell_temperature = doc["device_config"]["min_cell_temperature"].as<float>();
    
    // Prevent division by zero if num_cells is not properly loaded
    if (deviceConfig.num_cells > 0) {
        deviceConfig.max_pack_voltage = deviceConfig.max_cell_voltage * deviceConfig.num_cells;
        deviceConfig.min_pack_voltage = deviceConfig.min_cell_voltage * deviceConfig.num_cells;
    } else {
        Serial.println("ERROR: num_cells is still 0 after loading config!");
        deviceConfig.num_cells = 6; // Fallback to default
        deviceConfig.max_pack_voltage = deviceConfig.max_cell_voltage * deviceConfig.num_cells;
        deviceConfig.min_pack_voltage = deviceConfig.min_cell_voltage * deviceConfig.num_cells;
    }

    deviceConfig.solar_shunt_resistance = doc["device_config"]["solar_shunt_resistance"].as<float>();
    deviceConfig.load_shunt_resistance = doc["device_config"]["load_shunt_resistance"].as<float>();
    deviceConfig.max_current = doc["device_config"]["max_current"].as<float>();

    deviceConfig.thermistor_beta_value = doc["device_config"]["thermistor_beta_value"].as<float>();
    deviceConfig.thermistor_series_resistor = doc["device_config"]["thermistor_series_resistor"].as<float>();
    deviceConfig.thermistor_nominal_resistance = doc["device_config"]["thermistor_nominal_resistance"].as<float>();
    deviceConfig.thermistor_nominal_temperature = doc["device_config"]["thermistor_nominal_temperature"].as<float>();
    deviceConfig.max_board_temperature = doc["device_config"]["max_board_temperature"].as<float>(); 

    deviceConfig.fan_on_temperature = doc["device_config"]["fan_on_temperature"].as<float>();
    deviceConfig.fan_off_temperature = doc["device_config"]["fan_off_temperature"].as<float>();
    deviceConfig.low_battery_threshold = doc["device_config"]["low_battery_threshold"].as<float>();
    deviceConfig.high_battery_threshold = doc["device_config"]["high_battery_threshold"].as<float>();
    deviceConfig.full_battery_threshold = doc["device_config"]["full_battery_threshold"].as<float>();
    deviceConfig.low_battery_discharge_threshold = doc["device_config"]["low_battery_discharge_threshold"].as<float>();

    deviceConfig.fan_duty_cycle = doc["device_config"]["fan_duty_cycle"].as<float>();

    deviceConfig.bms_clk_pin = doc["device_config"]["bms_clk_pin"].as<int>();
    deviceConfig.bms_mosi_pin = doc["device_config"]["bms_mosi_pin"].as<int>();
    deviceConfig.bms_miso_pin = doc["device_config"]["bms_miso_pin"].as<int>();
    deviceConfig.bms_cs_pin = doc["device_config"]["bms_cs_pin"].as<int>();

    deviceConfig.wire_scl_pin = doc["device_config"]["wire_scl_pin"].as<int>();
    deviceConfig.wire_sda_pin = doc["device_config"]["wire_sda_pin"].as<int>();

    deviceConfig.restart_pin = doc["device_config"]["restart_pin"].as<int>();
    deviceConfig.solar_fet_pin = doc["device_config"]["solar_fet_pin"].as<int>();
    deviceConfig.solar_safety_fet_pin = doc["device_config"]["solar_safety_fet_pin"].as<int>();
    deviceConfig.load_fet_pin = doc["device_config"]["load_fet_pin"].as<int>();
    deviceConfig.fan_pin = doc["device_config"]["fan_pin"].as<int>();
    deviceConfig.thermistor_pin = doc["device_config"]["thermistor_pin"].as<int>();
    deviceConfig.boot_LED_pin = doc["device_config"]["boot_LED_pin"].as<int>();
}

void ConfigManager::printConfig() {
    //loadConfig("/config.json"); // Load config before printing
    // Print current configuration to serial
    Serial.println("Current Configuration:");
    Serial.print("Hostname: ");
    Serial.println(wifiConfig.hostname);
    Serial.print("Force Use Hotspot: ");
    Serial.println(wifiConfig.force_use_hotspot);
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
    doc["device_config"]["safety_cell_voltage"] = deviceConfig.safety_cell_voltage;
    doc["device_config"]["cell_voltage_hysteresis"] = deviceConfig.cell_voltage_hysteresis;
    doc["device_config"]["cell_balance_start"] = deviceConfig.cell_balance_start;
    doc["device_config"]["max_cell_temperature"] = deviceConfig.max_cell_temperature;
    doc["device_config"]["min_cell_temperature"] = deviceConfig.min_cell_temperature;

    doc["device_config"]["load_shunt_resistance"] = deviceConfig.load_shunt_resistance;
    doc["device_config"]["solar_shunt_resistance"] = deviceConfig.solar_shunt_resistance;
    doc["device_config"]["max_current"] = deviceConfig.max_current;

    doc["device_config"]["thermistor_beta_value"] = deviceConfig.thermistor_beta_value;
    doc["device_config"]["thermistor_series_resistor"] = deviceConfig.thermistor_series_resistor;
    doc["device_config"]["thermistor_nominal_resistance"] = deviceConfig.thermistor_nominal_resistance;
    doc["device_config"]["thermistor_nominal_temperature"] = deviceConfig.thermistor_nominal_temperature;
    doc["device_config"]["max_board_temperature"] = deviceConfig.max_board_temperature;

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
    doc["device_config"]["wire_scl_pin"] = deviceConfig.wire_scl_pin;
    doc["device_config"]["wire_sda_pin"] = deviceConfig.wire_sda_pin;
    doc["device_config"]["restart_pin"] = deviceConfig.restart_pin;
    doc["device_config"]["solar_fet_pin"] = deviceConfig.solar_fet_pin;
    doc["device_config"]["solar_safety_fet_pin"] = deviceConfig.solar_safety_fet_pin;
    doc["device_config"]["load_fet_pin"] = deviceConfig.load_fet_pin;
    doc["device_config"]["fan_pin"] = deviceConfig.fan_pin;
    doc["device_config"]["thermistor_pin"] = deviceConfig.thermistor_pin;
    doc["device_config"]["boot_LED_pin"] = deviceConfig.boot_LED_pin;

    // Serialize JSON to string and write to file
    File configFile = LittleFS.open(filename, "w");
    if (!configFile) {
        Serial.println("Failed to open config file for writing");
        return;
    }
    serializeJson(doc, configFile);
    configFile.close();
    LittleFS.end();
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
    LittleFS.end();
}
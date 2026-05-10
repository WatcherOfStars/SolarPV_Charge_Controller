#include <web_ui.h>

#include <EEPROM.h>
#include <WiFi.h>
#include <ESPmDNS.h>
//#include <DNSServer.h>
#include <iostream>
#include <algorithm>
#include <ArduinoJson.h>
#include <LittleFS.h>

using namespace std;

volatile bool updates = false;

WifiConfig wifiConfig;
bool webUiReady = false;

void WebUI::setupWebConn(){
	//CALL THIS FIRST TO SETUP WIFI CONNECTION BEFORE SETTING UP THE UI
	wifiConfig = ConfigManager::getInstance().wifiConfig;

	connectWifi();

	// Display the IP address of the ESP32 (MIGHT BREAK)
	IPAddress IP = WiFi.softAPIP();
	Serial.print("AP SSID: ");
	Serial.println(WiFi.softAPNetworkID());
	Serial.print("AP IP address: ");
	Serial.println(IP);

	WiFi.setSleep(false); //turn off sleeping to increase UI responsivness (at the cost of power use)
} 


void WebUI::updateWebUI(){

}

void WebUI::setupWebUI(){
	//Turn off verbose debugging
	//ESPUI.setVerbosity(Verbosity::Quiet);

	ESPUI.sliderContinuous = false;

	/*
	* Tab: Main Controls
	* Controls and data reporting.
	*-----------------------------------------------------------------------------------------------------------*/
	auto maintab = ESPUI.addControl(Tab, "", "Main Controls");

	control_buttons_section = ESPUI.addControl(Separator, "General Controls", "", None, maintab);

	//Callback shortcuts
	auto my_generalCallback = [this](Control *sender, int type) { this->generalCallback(sender, type); };
	auto my_sendTestPub = [this](Control *sender, int type) { this->sendTestPub(sender, type); };
	auto my_getTimeCallback = [this](Control *sender, int type) { this->getTimeCallback(sender, type); };
	auto my_textCallback = [this](Control *sender, int type) { this->textCallback(sender, type); };
	auto my_enterWifiDetailsCallback = [this](Control *sender, int type) { this->enterWifiDetailsCallback(sender, type); };
	auto my_updateObserversCallback = [this](Control *sender, int type) { this->updateObserversCallback(sender, type); };

	//buttons
	//main_button = ESPUI.addControl(Button, "Testing Buttons", "Send Test Data", Wetasphalt, control_buttons_section, my_sendTestPub);
	//ESPUI.addControl(Button, "", "Update Observers", Wetasphalt, control_buttons_section, my_updateObserversCallback);
	//ESPUI.addControl(Button, "", "Start_Client", Wetasphalt, control_buttons_section, my_updateObserversCallback);
	ESPUI.addControl(Button, "Restart_System", "Restart_System", Wetasphalt, control_buttons_section, my_updateObserversCallback);


	//test_message_text = ESPUI.addControl(Text, "Test Data Text", "change me!", Wetasphalt, maintab, my_generalCallback);

	// manual FET control
	String labelStyle = "width: 60px; margin-left: .3rem; margin-right: .1rem; background-color: unset; font-size: 1rem; color: #34495e; border: unset; layout: type: flex; flex_flow: row, flex_align_main: space_evenly, flex_align_track: center; align-items: center;";
	
	fet_toggle_section = ESPUI.addControl(Separator, "Manual FET Control", "", None, maintab);
	ESPUI.setElementStyle(ESPUI.addControl(Label, "", "Toggle Solar FETs:", None, fet_toggle_section), labelStyle);
	toggle_solar_switcher = ESPUI.addControl(Switcher, "Toggle_Solar_FETs", "", Wetasphalt, fet_toggle_section, my_updateObserversCallback);
	ESPUI.setElementStyle(ESPUI.addControl(Label, "", "Toggle Load FETs:", None, fet_toggle_section), labelStyle);
	toggle_load_switcher = ESPUI.addControl(Switcher, "Toggle_Load_FETs", "", Wetasphalt, fet_toggle_section, my_updateObserversCallback);
	ESPUI.setElementStyle(ESPUI.addControl(Label, "", "Toggle Fan:", None, fet_toggle_section), labelStyle);
	toggle_fan_switcher = ESPUI.addControl(Switcher, "Toggle_Fan", "", Wetasphalt, fet_toggle_section, my_updateObserversCallback);

	// system flags
	system_flags_section = ESPUI.addControl(Separator, "System Flags", "", None, maintab);
	ESPUI.setElementStyle(ESPUI.addControl(Label, "", "Enable BMS:", None, system_flags_section), labelStyle);
	enable_bms_switcher = ESPUI.addControl(Switcher, "Enable_BMS", "", Wetasphalt, system_flags_section, my_updateObserversCallback);
	ESPUI.setElementStyle(ESPUI.addControl(Label, "", "Enable RTC:", None, system_flags_section), labelStyle);
	enable_rtc_switcher = ESPUI.addControl(Switcher, "Enable_RTC", "", Wetasphalt, system_flags_section, my_updateObserversCallback);
	// ESPUI.setElementStyle(ESPUI.addControl(Label, "", "Enable Solar INA:", None, system_flags_section), labelStyle);
	// enable_solar_ina_switcher = ESPUI.addControl(Switcher, "Enable_Solar_INA", "", Wetasphalt, system_flags_section, my_updateObserversCallback);
	ESPUI.setElementStyle(ESPUI.addControl(Label, "", "Enable Load INA:", None, system_flags_section), labelStyle);
	enable_load_ina_switcher = ESPUI.addControl(Switcher, "Enable_Load_INA", "", Wetasphalt, system_flags_section, my_updateObserversCallback);
	ESPUI.setElementStyle(ESPUI.addControl(Label, "", "Enable Solar FETs:", None, system_flags_section), labelStyle);
	enable_solar_switcher = ESPUI.addControl(Switcher, "Enable_Solar_FETs", "", Wetasphalt, system_flags_section, my_updateObserversCallback);
	ESPUI.setElementStyle(ESPUI.addControl(Label, "", "Enable Load FETs:", None, system_flags_section), labelStyle);
	enable_load_switcher = ESPUI.addControl(Switcher, "Enable_Load_FETs", "", Wetasphalt, system_flags_section, my_updateObserversCallback);	
	ESPUI.setElementStyle(ESPUI.addControl(Label, "", "Enable Fan:", None, system_flags_section), labelStyle);
	enable_fan_switcher = ESPUI.addControl(Switcher, "Enable_Fan", "", Wetasphalt, system_flags_section, my_updateObserversCallback);

	// display values
	String dataStyle = "width: 60px; margin-left: .1rem; margin-right: .3rem; background-color: #34495e; font-size: 2rem; color: #bebebe; border: unset; layout: type: flex; flex_flow: row, flex_align_main: space_evenly, flex_align_track: center; align-items: center;";
	String dataArrayStyle = "width: 300px; margin-left: .1rem; margin-right: .3rem; background-color: #34495e; font-size: 1rem; color: #bebebe; border: unset; layout: type: flex; flex_flow: row, flex_align_main: space_evenly, flex_align_track: center; align-items: center;";

	// System Data
	system_data_section = ESPUI.addControl(Separator, "System Data", "", None, maintab);
	ESPUI.setElementStyle(ESPUI.addControl(Label, "", "Time:", None, system_data_section), labelStyle);
	rtc_time_label = ESPUI.addControl(Label, "RTC_Time", "", Wetasphalt, system_data_section, my_generalCallback);
	ESPUI.setElementStyle(rtc_time_label, dataArrayStyle);

	// ESPUI.setElementStyle(ESPUI.addControl(Label, "", "Solar Current:", None, system_data_section), labelStyle);
	// solar_current_label = ESPUI.addControl(Label, "Solar_Shunt_Current", "", Wetasphalt, system_data_section, my_generalCallback);
	// ESPUI.setElementStyle(solar_current_label, dataStyle);
	// ESPUI.setElementStyle(ESPUI.addControl(Label, "", "Solar Shunt Voltage:", None, system_data_section), labelStyle);
	// solar_shunt_voltage_label = ESPUI.addControl(Label, "Solar_Shunt_Voltage", "", Wetasphalt, system_data_section, my_generalCallback);
	// ESPUI.setElementStyle(solar_shunt_voltage_label, dataStyle);

	ESPUI.setElementStyle(ESPUI.addControl(Label, "", "Load Current:", None, system_data_section), labelStyle);
	load_current_label = ESPUI.addControl(Label, "Load_Shunt_Current", "", Wetasphalt, system_data_section, my_generalCallback);
	ESPUI.setElementStyle(load_current_label, dataStyle);
	ESPUI.setElementStyle(ESPUI.addControl(Label, "", "Load Shunt Voltage:", None, system_data_section), labelStyle);
	load_shunt_voltage_label = ESPUI.addControl(Label, "Load_Shunt_Voltage", "", Wetasphalt, system_data_section, my_generalCallback);
	ESPUI.setElementStyle(load_shunt_voltage_label, dataStyle);

	ESPUI.setElementStyle(ESPUI.addControl(Label, "", "Cell Voltages:", None, system_data_section), labelStyle);
	cell_voltages_label = ESPUI.addControl(Label, "BMS_Cell_Voltages", "", Wetasphalt, system_data_section, my_generalCallback);
	ESPUI.setElementStyle(cell_voltages_label, dataArrayStyle);
	ESPUI.setElementStyle(ESPUI.addControl(Label, "", "Cell Temps:", None, system_data_section), labelStyle);
	cell_temperatures_label = ESPUI.addControl(Label, "BMS_Cell_Temperatures", "", Wetasphalt, system_data_section, my_generalCallback);
	ESPUI.setElementStyle(cell_temperatures_label, dataArrayStyle);

	// Component status indicators
	component_status_section = ESPUI.addControl(Separator, "Component Status (1:working, 0:disabled, -1:error/disconnected)", "", None, maintab);
	// ESPUI.setElementStyle(ESPUI.addControl(Label, "", "Solar Shunt Status:", None, component_status_section), labelStyle);
	// solar_shunt_status_label = ESPUI.addControl(Label, "Solar_Shunt_Status", "-", Wetasphalt, component_status_section, my_generalCallback);
	// ESPUI.setElementStyle(solar_shunt_status_label, dataStyle);
	ESPUI.setElementStyle(ESPUI.addControl(Label, "", "Load Shunt Status:", None, component_status_section), labelStyle);
	load_shunt_status_label = ESPUI.addControl(Label, "Load_Shunt_Status", "-", Wetasphalt, component_status_section, my_generalCallback);
	ESPUI.setElementStyle(load_shunt_status_label, dataStyle);
	ESPUI.setElementStyle(ESPUI.addControl(Label, "", "RTC Status:", None, component_status_section), labelStyle);
	rtc_status_label = ESPUI.addControl(Label, "RTC_Status", "-", Wetasphalt, component_status_section, my_generalCallback);
	ESPUI.setElementStyle(rtc_status_label, dataStyle);
	ESPUI.setElementStyle(ESPUI.addControl(Label, "", "BMS Status:", None, component_status_section), labelStyle);
	bms_status_label = ESPUI.addControl(Label, "BMS_Status", "-", Wetasphalt, component_status_section, my_generalCallback);
	ESPUI.setElementStyle(bms_status_label, dataStyle);


	// test voltage slider
	testVoltageSlider = ESPUI.addControl(Slider, "Test_Voltage_Slider", "3.9", Wetasphalt, maintab, my_updateObserversCallback);
	ESPUI.addControl(Min, "", "0", None, testVoltageSlider);
	ESPUI.addControl(Max, "", "5", None, testVoltageSlider);

	//These are the values for the selector's options. (Note that they *must* be declared static
	//so that the storage is allocated in global memory and not just on the stack of this function.)
	// static String modes[] {"Value 1", "Value 2", "Value 3", "Value 4", "Value 5"};
	// auto mode_selector = ESPUI.addControl(Select, "Selector", "Mode", Wetasphalt, maintab, my_generalCallback);
	// for(auto const& v : modes) {
	// 	ESPUI.addControl(Option, v.c_str(), v, None, mode_selector);
	// }

	// //Voltage selector
	// auto voltage_selector = ESPUI.addControl(Select, "", "System Voltage", Wetasphalt, mode_selector, my_generalCallback);
	// ESPUI.addControl(Option, "12V", "12V", None, voltage_selector);
	// ESPUI.addControl(Option, "24V", "24V", None, voltage_selector);
	// ESPUI.addControl(Option, "48V", "48V", None, voltage_selector);

	//Time display
	mainTime = ESPUI.addControl(Time, "Current Time", "", Wetasphalt, maintab, my_getTimeCallback);

	/*
	* Tab: System Setup
	* Contains step-by-step instructions to fully set up the system
	*-----------------------------------------------------------------------------------------------------------*/
	auto systemSetupTab = ESPUI.addControl(Tab, "", "System Setup");
	ESPUI.addControl(Label, "Step 1", "Disable all system flags before setup.", None, systemSetupTab);
	ESPUI.addControl(Label, "Step 2", "Configure connection to LAN if desired using the WiFi credentials tab.", None, systemSetupTab);
	ESPUI.addControl(Label, "Step 3", "Verify system reboots, then hold power button to restart. If WiFi credentials were changed, new IP address will be 10.16.204.165.", None, systemSetupTab);
	ESPUI.addControl(Label, "Step 4", "Enable fan in main controls tab and verify it turns on.", None, systemSetupTab);
	ESPUI.addControl(Label, "Step 5", "Enable RTC and set time.", None, systemSetupTab);
	ESPUI.addControl(Label, "Step 6", "Ensure battery(-) is physically disconnected, connect solar. Enable solar in the main controls tab and verify voltage with multimeter. Then enable load and verify voltage.", None, systemSetupTab);
	ESPUI.addControl(Label, "Step 7", "Disconnect solar and loads, disable all FETs. Connect the battery to battery(+) and battery(-) and the BMS to SPI pins. Enable the BMS in the main controls tab. Verify cell voltage and temperature readings.", None, systemSetupTab);
	ESPUI.addControl(Label, "Step 8", "Verify cell balancing over 1 hour.", None, systemSetupTab);
	ESPUI.addControl(Label, "Step 9", "Physically connect and then enable loads in the main control tab. Draw known power and verify current readings.", None, systemSetupTab);
	ESPUI.addControl(Label, "Step 10", "Physically connect and then enable solar in the main control tab. Verify current reading and cells charging.", None, systemSetupTab);
	ESPUI.addControl(Label, "Step 11", "Charge batteries to max. Verify charging stops.", None, systemSetupTab);
	ESPUI.addControl(Label, "Step 12", "Drain batteries with load. Verify charging resumes, then disable solar FETs to achieve full drain to min. Verify loads disconnect when battery is depleted.", None, systemSetupTab);
	ESPUI.addControl(Label, "Step 13", "Reconnect and re-enable all system flags. Verify full operation.", None, systemSetupTab);


	/*
	* Tab: WiFi Credentials
	* You use this tab to enter the SSID and password of a wifi network to autoconnect to.
	*-----------------------------------------------------------------------------------------------------------*/
	auto wifitab = ESPUI.addControl(Tab, "", "WiFi Credentials");
	wifi_ssid_text = ESPUI.addControl(Text, "SSID", "", Alizarin, wifitab, my_textCallback);
	//Note that adding a "Max" control to a text control sets the max length
	ESPUI.addControl(Max, "", "32", None, wifi_ssid_text);
	wifi_pass_text = ESPUI.addControl(Text, "Password", "", Alizarin, wifitab, my_textCallback);
	ESPUI.addControl(Max, "", "64", None, wifi_pass_text);
	ESPUI.addControl(Button, "Save", "Save", Peterriver, wifitab, my_enterWifiDetailsCallback);


	/*
	* Tab: Config File
	* Download and upload config.json with validation.
	*-----------------------------------------------------------------------------------------------------------*/
	auto configTab = ESPUI.addControl(Tab, "", "Config Upload/Download");
	ESPUI.addControl(Label, "Warning", "Contains WiFi and MQTT credentials. Only transfer on a trusted network.", Alizarin, configTab);
	ESPUI.addControl(Label, "Download", "<button onclick=\"window.location='/config/download'\">Download config.json</button>", None, configTab);
	ESPUI.addControl(Label, "Upload", "<form method='POST' action='/config/upload' enctype='multipart/form-data'><input type='file' name='config' accept='.json,application/json' required><button type='submit'>Upload config.json</button></form>", None, configTab);


	//Finally, start up the UI. 
	//This should only be called once we are connected to WiFi.
	ESPUI.begin(wifiConfig.hostname.c_str());
	registerConfigEndpoints();
	webUiReady = true;
}


// ========== Callbacks ==========
void WebUI::getTimeCallback(Control *sender, int type) {
	if(type == B_UP) {
		ESPUI.updateTime(mainTime);
	}
}

//Send test data callback
void WebUI::sendTestPub(Control *sender, int type) {
	Serial.print("CB: id(");
	Serial.print(sender->id);
	Serial.print(") Type(");
	Serial.print(type);
	Serial.print(") '");
	Serial.print(sender->label);
	Serial.print("' = ");
	Serial.println(sender->value);
	String message = ESPUI.getControl(test_message_text)->value;
	std::string msgStr = message.c_str();
	broker->getBroker().publish("test/topic", msgStr, 0, false);

}

//Most elements in this test UI are assigned this generic callback which prints some
//basic information. Event types are defined in ESPUI.h
void WebUI::generalCallback(Control *sender, int type) {
	Serial.print("CB: id(");
	Serial.print(sender->id);
	Serial.print(") Type(");
	Serial.print(type);
	Serial.print(") '");
	Serial.print(sender->label);
	Serial.print("' = ");
	Serial.println(sender->value);
}

// ========== Utility functions ==========
void WebUI::readStringFromEEPROM(String& buf, int baseaddress, int size) {
	buf.reserve(size);
	for (int i = baseaddress; i < baseaddress+size; i++) {
		char c = EEPROM.read(i);
		buf += c;
		if(!c) break;
	}	
}

bool WebUI::validateConfigJsonFile(const char* path, String& errorMessage) {
	File configFile = LittleFS.open(path, "r");
	if (!configFile) {
		errorMessage = "Failed to open uploaded config file";
		return false;
	}

	JsonDocument doc;
	DeserializationError error = deserializeJson(doc, configFile);
	configFile.close();
	if (error) {
		errorMessage = String("Invalid JSON: ") + error.c_str();
		return false;
	}

	if (!doc.is<JsonObject>()) {
		errorMessage = "Config must be a JSON object";
		return false;
	}

	if (!doc["wifi_config"].is<JsonObject>() || !doc["mqtt_config"].is<JsonObject>() || !doc["device_config"].is<JsonObject>()) {
		errorMessage = "Missing one or more required sections: wifi_config, mqtt_config, device_config";
		return false;
	}

	return true;
}

void WebUI::registerConfigEndpoints() {
	AsyncWebServer* server = ESPUI.WebServer();
	if (server == nullptr) {
		Serial.println("Config endpoints not registered: web server is null");
		return;
	}

	server->on("/config/download", HTTP_GET, [this](AsyncWebServerRequest* request) {
		if (!LittleFS.begin(false)) {
			request->send(500, "text/plain", "Failed to mount LittleFS");
			return;
		}

		if (!LittleFS.exists(kConfigPath)) {
			request->send(404, "text/plain", "config.json not found");
			return;
		}

		AsyncWebServerResponse* response = request->beginResponse(LittleFS, kConfigPath, "application/json", true);
		response->addHeader("Cache-Control", "no-store");
		request->send(response);
	});

	server->on("/config/upload", HTTP_POST,
		[this](AsyncWebServerRequest* request) {
			if (configUploadFailed) {
				String message = "Upload failed: " + configUploadError;
				request->send(400, "text/plain", message);
				return;
			}

			request->send(200, "text/plain", "Upload successful. New config.json is active.");
		},
		[this](AsyncWebServerRequest* request, const String& filename, size_t index, uint8_t *data, size_t len, bool final) {
			if (index == 0) {
				configUploadFailed = false;
				configUploadBytes = 0;
				configUploadError = "";

				if (!LittleFS.begin(false)) {
					configUploadFailed = true;
					configUploadError = "Failed to mount LittleFS";
					return;
				}

				if (filename.length() == 0 || (!filename.endsWith(".json") && !filename.endsWith(".JSON"))) {
					configUploadFailed = true;
					configUploadError = "Only .json uploads are allowed";
					return;
				}

				LittleFS.remove(kConfigUploadTempPath);
				File initFile = LittleFS.open(kConfigUploadTempPath, "w");
				if (!initFile) {
					configUploadFailed = true;
					configUploadError = "Failed to create temporary upload file";
					return;
				}
				initFile.close();
			}

			if (configUploadFailed) {
				if (final) {
					LittleFS.remove(kConfigUploadTempPath);
				}
				return;
			}

			if ((configUploadBytes + len) > kMaxConfigUploadBytes) {
				configUploadFailed = true;
				configUploadError = "File too large (max 32KB)";
				LittleFS.remove(kConfigUploadTempPath);
				return;
			}

			File tempFile = LittleFS.open(kConfigUploadTempPath, "a");
			if (!tempFile) {
				configUploadFailed = true;
				configUploadError = "Failed to open temporary upload file";
				return;
			}

			size_t written = tempFile.write(data, len);
			tempFile.close();
			if (written != len) {
				configUploadFailed = true;
				configUploadError = "Failed to write full upload chunk";
				LittleFS.remove(kConfigUploadTempPath);
				return;
			}

			configUploadBytes += len;

			if (final) {
				String validationError;
				if (!validateConfigJsonFile(kConfigUploadTempPath, validationError)) {
					configUploadFailed = true;
					configUploadError = validationError;
					LittleFS.remove(kConfigUploadTempPath);
					return;
				}

				LittleFS.remove(kConfigPath);
				if (!LittleFS.rename(kConfigUploadTempPath, kConfigPath)) {
					configUploadFailed = true;
					configUploadError = "Failed to move uploaded file into place";
					LittleFS.remove(kConfigUploadTempPath);
					return;
				}

				ConfigManager::getInstance().loadConfig(kConfigPath);
				Serial.println("New config.json uploaded and loaded successfully");
			}
		});
}

void WebUI::connectWifi() {
	int connect_timeout;
	WiFi.setHostname(wifiConfig.hostname.c_str());
	Serial.println("Begin wifi...");

	//Load credentials from EEPROM 
	if(!(wifiConfig.force_use_hotspot)) {
		yield();
		EEPROM.begin(100);
		String stored_ssid, stored_pass;
		readStringFromEEPROM(stored_ssid, 0, 32);
		readStringFromEEPROM(stored_pass, 32, 96);
		EEPROM.end();
	
		//Try to connect with stored credentials, fire up an access point if they don't work.
		#if defined(ESP32)
			WiFi.begin(stored_ssid.c_str(), stored_pass.c_str());
		#else
			WiFi.begin(stored_ssid, stored_pass);
		#endif
		connect_timeout = 28; //7 seconds
		while (WiFi.status() != WL_CONNECTED && connect_timeout > 0) {
			delay(250);
			Serial.print(".");
			connect_timeout--;
		}
	}
	
	if (WiFi.status() == WL_CONNECTED) {

		Serial.println(WiFi.localIP());
		Serial.println("Wifi started");

		if (!MDNS.begin(wifiConfig.hostname.c_str())) {
			Serial.println("Error setting up MDNS responder!");
		}
	} else {
		Serial.println("\nCreating access point...");
		WiFi.mode(WIFI_AP);
		WiFi.softAPConfig(IPAddress(192, 168, 1, 1), IPAddress(192, 168, 1, 1), IPAddress(255, 255, 255, 0));
		WiFi.softAP(wifiConfig.hostname.c_str(), wifiConfig.password.c_str()); //append device ID to hotspot name to make it identifiable

		connect_timeout = 20;
		do {
			delay(250);
			Serial.print(",");
			connect_timeout--;
		} while(connect_timeout);
	}
}

void WebUI::enterWifiDetailsCallback(Control *sender, int type) {
	if(type == B_UP) {
		Serial.println("Saving credentials to EPROM...");
		Serial.println(ESPUI.getControl(wifi_ssid_text)->value);
		Serial.println(ESPUI.getControl(wifi_pass_text)->value);
		unsigned int i;
		EEPROM.begin(100);
		for(i = 0; i < ESPUI.getControl(wifi_ssid_text)->value.length(); i++) {
			EEPROM.write(i, ESPUI.getControl(wifi_ssid_text)->value.charAt(i));
			if(i==30) break; //Even though we provided a max length, user input should never be trusted
		}
		EEPROM.write(i, '\0');

		for(i = 0; i < ESPUI.getControl(wifi_pass_text)->value.length(); i++) {
			EEPROM.write(i + 32, ESPUI.getControl(wifi_pass_text)->value.charAt(i));
			if(i==94) break; //Even though we provided a max length, user input should never be trusted
		}
		EEPROM.write(i + 32, '\0');
		EEPROM.end();
	}
}

void WebUI::textCallback(Control *sender, int type) {
	//This callback is needed to handle the changed values, even though it doesn't do anything itself.
}

// Notify observers callback
void WebUI::updateObserversCallback(Control *sender, int type) {
	Serial.print("CB: id(");
	Serial.print(sender->id);
	Serial.print(") Type(");
	Serial.print(type);
	Serial.print(") '");
	Serial.print(sender->label);
	Serial.print("' = ");
	Serial.println(sender->value);

	//std::string topicStr = std::to_string(sender->id) + std::string("/") + std::to_string(type);
	String msgStr = sender->value;
	char* topic = (char*)sender->label;

	// Notify all observers with the message
	notifyObservers(topic, (char*)msgStr.c_str());
}

// WebUI Observer implementation
void WebUI::onNotify(const char* topic, const char* message) {
	//This function is called when an MQTT message is received.
	// Serial.print("WebUI received notification on topic: ");
	// Serial.print(topic);
	// Serial.print(" with message: ");
	// Serial.println(message);
	if (topic == nullptr || message == nullptr) {
		Serial.println("WebUI onNotify received null topic or message");
		return;
	}

	//Handle toggle value updates
	if (strcmp(topic, "system_update/data") == 0) {
		Serial.println("Handling system_update/data");
		//Parse the JSON messageto update the relevant flags and values. 
		JsonDocument doc;
		DeserializationError error = deserializeJson(doc, message);
		if (error) {
			Serial.print("deserializeJson() failed: ");
			Serial.println(error.c_str());
			return;
		}
		if (!doc.is<JsonObject>()) {
			Serial.println("system_update/data payload is not a JSON object");
			return;
		}

		// update switchers based on message
		if (!doc["Toggle_Solar_FETs"].isNull()) ESPUI.updateSwitcher(toggle_solar_switcher, doc["Toggle_Solar_FETs"]);
		if (!doc["Toggle_Load_FETs"].isNull()) ESPUI.updateSwitcher(toggle_load_switcher, doc["Toggle_Load_FETs"]);
		if (!doc["Toggle_Fan"].isNull()) ESPUI.updateSwitcher(toggle_fan_switcher, doc["Toggle_Fan"]);

		//update labels based on message
		//debut print to verify message parsing
		// Serial.print("WebUI recieved Shunt_Voltage: ");
		// Serial.println(doc["Shunt_Voltage"].as<const char*>());
		// Serial.print("WebUI recieved Shunt_Current: ");
		// Serial.println(doc["Shunt_Current"].as<const char*>());

		if (!doc["RTC_Time"].isNull()) ESPUI.updateLabel(rtc_time_label, doc["RTC_Time"]);
		if (!doc["Solar_Shunt_Current"].isNull()) ESPUI.updateLabel(solar_current_label, doc["Solar_Shunt_Current"]);
		if (!doc["Load_Shunt_Current"].isNull()) ESPUI.updateLabel(load_current_label, doc["Load_Shunt_Current"]);
		if (!doc["Solar_Shunt_Voltage"].isNull()) ESPUI.updateLabel(solar_shunt_voltage_label, doc["Solar_Shunt_Voltage"]);
		if (!doc["Load_Shunt_Voltage"].isNull()) ESPUI.updateLabel(load_shunt_voltage_label, doc["Load_Shunt_Voltage"]);
		String cellVoltagesText;
		String cellTemperaturesText;
		if (doc["Cell_Voltages"].is<JsonArray>() || doc["Cell_Voltages"].is<JsonVariantConst>()) {
			serializeJson(doc["Cell_Voltages"], cellVoltagesText);
			ESPUI.updateLabel(cell_voltages_label, cellVoltagesText);
		}
		if (doc["Cell_Temperatures"].is<JsonArray>() || doc["Cell_Temperatures"].is<JsonVariantConst>()) {
			serializeJson(doc["Cell_Temperatures"], cellTemperaturesText);
			ESPUI.updateLabel(cell_temperatures_label, cellTemperaturesText);
		}
		if (!doc["Solar_Shunt_Status"].isNull()) ESPUI.updateLabel(solar_shunt_status_label, doc["Solar_Shunt_Status"]);
		if (!doc["Load_Shunt_Status"].isNull()) ESPUI.updateLabel(load_shunt_status_label, doc["Load_Shunt_Status"]);
		if (!doc["RTC_Status"].isNull()) ESPUI.updateLabel(rtc_status_label, doc["RTC_Status"]);
		if (!doc["BMS_Status"].isNull()) ESPUI.updateLabel(bms_status_label, doc["BMS_Status"]);
	}

	//Handle system flag updates
	if (strcmp(topic, "system_update/flags") == 0) {
		Serial.println("Handling system_update/flags");

		//Parse the JSON messageto update the relevant flags and values. 
		JsonDocument doc;
		DeserializationError error = deserializeJson(doc, message);
		if (error) {
			Serial.print("deserializeJson() failed: ");
			Serial.println(error.c_str());
			return;
		}
		if (!doc.is<JsonObject>()) {
			Serial.println("system_update/flags payload is not a JSON object");
			return;
		}

		//update flags based on message
		if (!doc["Enable_BMS"].isNull()) ESPUI.updateSwitcher(enable_bms_switcher, doc["Enable_BMS"]);
		if (!doc["Enable_RTC"].isNull()) ESPUI.updateSwitcher(enable_rtc_switcher, doc["Enable_RTC"]);
		if (!doc["Enable_Solar_INA"].isNull()) ESPUI.updateSwitcher(enable_solar_ina_switcher, doc["Enable_Solar_INA"]);
		if (!doc["Enable_Load_INA"].isNull()) ESPUI.updateSwitcher(enable_load_ina_switcher, doc["Enable_Load_INA"]);
		if (!doc["Enable_Solar_FETs"].isNull()) ESPUI.updateSwitcher(enable_solar_switcher, doc["Enable_Solar_FETs"]);
		if (!doc["Enable_Load_FETs"].isNull()) ESPUI.updateSwitcher(enable_load_switcher, doc["Enable_Load_FETs"]);
		if (!doc["Enable_Fan"].isNull()) ESPUI.updateSwitcher(enable_fan_switcher, doc["Enable_Fan"]);
	}
}

// WebUI Subject implementation
void WebUI::registerObserver(observer* obs) {
	std::cout << "Registering WebUI Observer " << obs << std::endl;
	observers.push_back(obs);
}
void WebUI::removeObserver(observer* obs) {
	observers.erase(std::remove(observers.begin(), observers.end(), obs), observers.end());
}
void WebUI::notifyObservers(const char* topic, const char* message) {
	// std::cout << "Notifying WebUI Observers for topic: " << topic << std::endl;
	// std::cout << "Message: " << message << std::endl;
	for (auto& obs : observers) {
		obs->onNotify(topic, message);
	}
}


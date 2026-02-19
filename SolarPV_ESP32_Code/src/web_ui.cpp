#include <web_ui.h>

#include <EEPROM.h>
#include <WiFi.h>
#include <ESPmDNS.h>
#include <iostream>
#include <algorithm>

using namespace constants;
using namespace std;

volatile bool updates = false;

void WebUI::setupWebConn(){
	connectWifi();

	// Display the IP address of the ESP32 (MIGHT BREAK)
	IPAddress IP = WiFi.softAPIP();
	Serial.print("AP IP address: ");
	Serial.println(IP);

	WiFi.setSleep(false); //turn off sleeping to increase UI responsivness (at the cost of power use)
} 


void WebUI::updateWebUI(){
	

}

void WebUI::setupWebUI(){
	//Turn off verbose debugging
	//ESPUI.setVerbosity(Verbosity::Quiet);

	//Make sliders continually report their position as they are being dragged.
	ESPUI.sliderContinuous = true;

	/*
	* Tab: Main Controls
	* Controls and data reporting.
	*-----------------------------------------------------------------------------------------------------------*/
	auto maintab = ESPUI.addControl(Tab, "", "Main Controls");

	ESPUI.addControl(Separator, "General Controls", "", None, maintab);

	//Callback shortcuts
	auto my_generalCallback = [this](Control *sender, int type) { this->generalCallback(sender, type); };
	auto my_sendTestPub = [this](Control *sender, int type) { this->sendTestPub(sender, type); };
	auto my_getTimeCallback = [this](Control *sender, int type) { this->getTimeCallback(sender, type); };
	auto my_textCallback = [this](Control *sender, int type) { this->textCallback(sender, type); };
	auto my_enterWifiDetailsCallback = [this](Control *sender, int type) { this->enterWifiDetailsCallback(sender, type); };
	auto my_updateObserversCallback = [this](Control *sender, int type) { this->updateObserversCallback(sender, type); };
	auto my_startClientCallback = [this](Control *sender, int type) { this->startClientCallback(sender, type); };

	//buttons
	main_button = ESPUI.addControl(Button, "Testing Buttons", "Send Test Data", Wetasphalt, maintab, my_sendTestPub);
	ESPUI.addControl(Button, "", "Update Observers", Wetasphalt, main_button, my_updateObserversCallback);
	ESPUI.addControl(Button, "", "Start Client", Wetasphalt, main_button, my_startClientCallback);
	ESPUI.addControl(Button, "", "Restart System", Wetasphalt, main_button, my_updateObserversCallback);


	test_message_text = ESPUI.addControl(Text, "Test Data Text", "change me!", Wetasphalt, maintab, my_generalCallback);


	mainSwitcher = ESPUI.addControl(Switcher, "Toggle_Solar_FETs", "Toggle_Solar_FETs", Wetasphalt, maintab, my_updateObserversCallback);
	ESPUI.addControl(Switcher, "Toggle_Load_FETs", "Toggle_Load_FETs", Wetasphalt, maintab, my_updateObserversCallback);
	ESPUI.addControl(Switcher, "Toggle_Fan", "Toggle_Fan", Wetasphalt, maintab, my_updateObserversCallback);
	ESPUI.addControl(Separator, "System Flags", "", None, maintab);
	ESPUI.addControl(Switcher, "Enable_BMS", "Enable_BMS", Wetasphalt, maintab, my_updateObserversCallback);
	ESPUI.addControl(Switcher, "Enable_RTC", "Enable_RTC", Wetasphalt, maintab, my_updateObserversCallback);
	ESPUI.addControl(Switcher, "Enable_Solar_FETs", "Enable_Solar_FETs", Wetasphalt, maintab, my_updateObserversCallback);
	ESPUI.addControl(Switcher, "Enable_Load_FETs", "Enable_Load_FETs", Wetasphalt, maintab, my_updateObserversCallback);	
	ESPUI.addControl(Switcher, "Enable_FAN", "Enable_FAN", Wetasphalt, maintab, my_updateObserversCallback);
	ESPUI.addControl(Switcher, "Enable_INA226", "Enable_INA226", Wetasphalt, maintab, my_updateObserversCallback);

	//Sliders default to being 0 to 100, but if you want different limits you can add a Min and Max control
	// mainSlider = ESPUI.addControl(Slider, "Slider", "200", Wetasphalt, maintab, generalCallback);
	// ESPUI.addControl(Min, "", "10", None, mainSlider);
	// ESPUI.addControl(Max, "", "400", None, mainSlider);

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


	//Number inputs also accept Min and Max components, but you should still validate the values.
	// mainNumber = ESPUI.addControl(Number, "Number Input", "42", Wetasphalt, maintab, my_generalCallback);
	// ESPUI.addControl(Min, "", "10", None, mainNumber);
	// ESPUI.addControl(Max, "", "50", None, mainNumber);


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


	//Finally, start up the UI. 
	//This should only be called once we are connected to WiFi.
	ESPUI.begin(HOSTNAME);
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

//Send test data callback
void WebUI::startClientCallback(Control *sender, int type) {
	Serial.print("CB: id(");
	Serial.print(sender->id);
	Serial.print(") Type(");
	Serial.print(type);
	Serial.print(") '");
	Serial.print(sender->label);
	Serial.print("' = ");
	Serial.println(sender->value);
	notifyObservers("start_client", "true");
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

void WebUI::connectWifi() {
	int connect_timeout;
	WiFi.setHostname(HOSTNAME);
	Serial.println("Begin wifi...");

	//Load credentials from EEPROM 
	if(!(FORCE_USE_HOTSPOT)) {
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

		if (!MDNS.begin(HOSTNAME)) {
			Serial.println("Error setting up MDNS responder!");
		}
	} else {
		Serial.println("\nCreating access point...");
		WiFi.mode(WIFI_AP);
		WiFi.softAPConfig(IPAddress(192, 168, 1, 1), IPAddress(192, 168, 1, 1), IPAddress(255, 255, 255, 0));
		WiFi.softAP(HOSTNAME);

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
void WebUI::onNotify(char* topic, char* message) {
	//This function is called when an MQTT message is received.
	Serial.print("WebUI received MQTT message on topic: ");
	Serial.print(topic);
	Serial.print(" with message: ");
	Serial.println(message);
}

// WebUI Subject implementation
void WebUI::registerObserver(observer* obs) {
	std::cout << "Registering WebUI Observer " << obs << std::endl;
	observers.push_back(obs);
}
void WebUI::removeObserver(observer* obs) {
	observers.erase(std::remove(observers.begin(), observers.end(), obs), observers.end());
}
void WebUI::notifyObservers(char* topic, char* message) {
	std::cout << "Notifying WebUI Observers for topic: " << topic << std::endl;
	std::cout << "Message: " << message << std::endl;
	for (auto& obs : observers) {
		obs->onNotify(topic, message);
	}
}


#include <Arduino.h>
#include <EEPROM.h>
#include <ESPUI.h>
#include <WiFi.h>
#include <ESPmDNS.h>
#include<sMQTTBroker.h>
#include<Arduino.h>


//Settings
#define SLOW_BOOT 0
#define HOSTNAME "ESPUITest"
#define FORCE_USE_HOTSPOT 0
const char* MQTT_CLIENT_USER = "solarpv"; // username for mqtt clients. Set your own value here.
const char* MQTT_CLIENT_PASSWORD = "solarpv123"; // password for mqtt clients. Set your own value here.


//Function Prototypes
void connectWifi();
void setUpUI();
void enterWifiDetailsCallback(Control *sender, int type);
void textCallback(Control *sender, int type);
void sendTestPub(Control *sender, int type);
void generalCallback(Control *sender, int type);
void updateCallback(Control *sender, int type);
void getTimeCallback(Control *sender, int type);
void graphAddCallback(Control *sender, int type);
void graphClearCallback(Control *sender, int type);
void extendedCallback(Control* sender, int type, void* param);

//UI handles
uint16_t wifi_ssid_text, wifi_pass_text;
uint16_t mainLabel, mainSwitcher, mainSlider, test_message_test, mainNumber, mainScrambleButton, mainTime;
uint16_t styleButton, styleLabel, styleSwitcher, styleSlider, styleButton2, styleLabel2, styleSlider2;
uint16_t graph;
volatile bool updates = false;

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

void setUpUI(){
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

	//buttons
	auto button_group = ESPUI.addControl(Button, "Testing Buttons", "Send Test Data", Wetasphalt, maintab, sendTestPub);
	ESPUI.addControl(Button, "", "Button B", Wetasphalt, button_group, generalCallback);
	ESPUI.addControl(Button, "", "Button C", Wetasphalt, button_group, generalCallback);

	test_message_test = ESPUI.addControl(Text, "Test Data Text", "change me!", Wetasphalt, maintab, generalCallback);


	mainSwitcher = ESPUI.addControl(Switcher, "Switcher", "", Wetasphalt, maintab, generalCallback);

	//Sliders default to being 0 to 100, but if you want different limits you can add a Min and Max control
	// mainSlider = ESPUI.addControl(Slider, "Slider", "200", Wetasphalt, maintab, generalCallback);
	// ESPUI.addControl(Min, "", "10", None, mainSlider);
	// ESPUI.addControl(Max, "", "400", None, mainSlider);

	//These are the values for the selector's options. (Note that they *must* be declared static
	//so that the storage is allocated in global memory and not just on the stack of this function.)
	static String modes[] {"Value 1", "Value 2", "Value 3", "Value 4", "Value 5"};
	auto mode_selector = ESPUI.addControl(Select, "Selector", "Mode", Wetasphalt, maintab, generalCallback);
	for(auto const& v : modes) {
		ESPUI.addControl(Option, v.c_str(), v, None, mode_selector);
	}

	//Voltage selector
	auto voltage_selector = ESPUI.addControl(Select, "", "System Voltage", Wetasphalt, mode_selector, generalCallback);
	ESPUI.addControl(Option, "12V", "12V", None, voltage_selector);
	ESPUI.addControl(Option, "24V", "24V", None, voltage_selector);
	ESPUI.addControl(Option, "48V", "48V", None, voltage_selector);

	//Time display
	mainTime = ESPUI.addControl(Time, "Current Time", "", Wetasphalt, maintab, getTimeCallback);


	//Number inputs also accept Min and Max components, but you should still validate the values.
	mainNumber = ESPUI.addControl(Number, "Number Input", "42", Wetasphalt, maintab, generalCallback);
	ESPUI.addControl(Min, "", "10", None, mainNumber);
	ESPUI.addControl(Max, "", "50", None, mainNumber);


	/*
	 * Tab: WiFi Credentials
	 * You use this tab to enter the SSID and password of a wifi network to autoconnect to.
	 *-----------------------------------------------------------------------------------------------------------*/
	auto wifitab = ESPUI.addControl(Tab, "", "WiFi Credentials");
	wifi_ssid_text = ESPUI.addControl(Text, "SSID", "", Alizarin, wifitab, textCallback);
	//Note that adding a "Max" control to a text control sets the max length
	ESPUI.addControl(Max, "", "32", None, wifi_ssid_text);
	wifi_pass_text = ESPUI.addControl(Text, "Password", "", Alizarin, wifitab, textCallback);
	ESPUI.addControl(Max, "", "64", None, wifi_pass_text);
	ESPUI.addControl(Button, "Save", "Save", Peterriver, wifitab, enterWifiDetailsCallback);


	//Finally, start up the UI. 
	//This should only be called once we are connected to WiFi.
	ESPUI.begin(HOSTNAME);
}

// ========== Callbacks ==========
void updateCallback(Control *sender, int type) {
	updates = (sender->value.toInt() > 0);
}

void getTimeCallback(Control *sender, int type) {
	if(type == B_UP) {
		ESPUI.updateTime(mainTime);
	}
}

void graphAddCallback(Control *sender, int type) {
	if(type == B_UP) {
		ESPUI.addGraphPoint(graph, random(1, 50));
	}
}

void graphClearCallback(Control *sender, int type) {
	if(type == B_UP) {
		ESPUI.clearGraph(graph);
	}
}

//Send test data callback
void sendTestPub(Control *sender, int type) {
	Serial.print("CB: id(");
	Serial.print(sender->id);
	Serial.print(") Type(");
	Serial.print(type);
	Serial.print(") '");
	Serial.print(sender->label);
	Serial.print("' = ");
	Serial.println(sender->value);
	String message = ESPUI.getControl(test_message_test)->value;
	std::string msgStr = message.c_str();
	broker.publish("test/topic", msgStr, 0, false);

}

//Most elements in this test UI are assigned this generic callback which prints some
//basic information. Event types are defined in ESPUI.h
void generalCallback(Control *sender, int type) {
	Serial.print("CB: id(");
	Serial.print(sender->id);
	Serial.print(") Type(");
	Serial.print(type);
	Serial.print(") '");
	Serial.print(sender->label);
	Serial.print("' = ");
	Serial.println(sender->value);
}

// Most elements in this test UI are assigned this generic callback which prints some
// basic information. Event types are defined in ESPUI.h
// The extended param can be used to hold a pointer to additional information
// or for C++ it can be used to return a this pointer for quick access
// using a lambda function
void extendedCallback(Control* sender, int type, void* param)
{
    Serial.print("CB: id(");
    Serial.print(sender->id);
    Serial.print(") Type(");
    Serial.print(type);
    Serial.print(") '");
    Serial.print(sender->label);
    Serial.print("' = ");
    Serial.println(sender->value);
    Serial.print("param = ");
    Serial.println((long)param);
}

void setup() {
	Serial.begin(115200);
	while(!Serial);
	if(SLOW_BOOT) delay(5000); //Delay booting to give time to connect a serial monitor
	connectWifi();
	Serial.begin(115200);

    // Display the IP address of the ESP32 (MIGHT BREAK)
    IPAddress IP = WiFi.softAPIP();
    Serial.print("AP IP address: ");
    Serial.println(IP);
   
    const unsigned short mqttPort=9000;
    broker.init(mqttPort);

	WiFi.setSleep(false); //turn off sleeping to increase UI responsivness (at the cost of power use)
	setUpUI();
}

void loop() {
	broker.update(); //update MQTT broker

	static long unsigned lastTime = 0;

	if(millis() > lastTime + 500) {
		
		lastTime = millis();
	}

	//Simple debug UART interface
	if(Serial.available()) {
		switch(Serial.read()) {
			case 'w': //Print IP details
				Serial.println(WiFi.localIP());
				break;
			case 'W': //Reconnect wifi
				connectWifi();
				break;
			case 'C': //Force a crash (for testing exception decoder)
				#if !defined(ESP32)
					((void (*)())0xf00fdead)();
				#endif
				break;
			default:
				Serial.print('#');
				break;
		}
	}
}

// ========== Utility functions ==========
void readStringFromEEPROM(String& buf, int baseaddress, int size) {
	buf.reserve(size);
	for (int i = baseaddress; i < baseaddress+size; i++) {
		char c = EEPROM.read(i);
		buf += c;
		if(!c) break;
	}	
}

void connectWifi() {
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

void enterWifiDetailsCallback(Control *sender, int type) {
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

void textCallback(Control *sender, int type) {
	//This callback is needed to handle the changed values, even though it doesn't do anything itself.
}

void randomString(char *buf, int len) {
	for(auto i = 0; i < len-1; i++) 
		buf[i] = random(0, 26) + 'A';
	buf[len-1] = '\0';
}
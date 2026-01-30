#include <main.h>

#include <string>
#include <iostream>
#include <broker.h>
#include <system.h>

using namespace std;
using namespace constants;


// Event handler methods
void mainEventHandler::notifyMQTT(char* topic, char* message) {
    // Handle MQTT notifications here
}

void mainEventHandler::notifyWebUI(char* topic, char* message) {
    // Handle WebUI notifications here
}


// Create global objects
SystemManager sys; // Create System object
BrokerManager broker; // Create BrokerManager object
WebUI webUI; // Create WebUI object
mqttClientManager client; // Create ClientManager object
mainEventHandler eventHandler; // Create main event handler object


void setup(){
  // Start serial communication for debugging
  Serial.begin(115200);
	while(!Serial);
	if(SLOW_BOOT) delay(5000); //Delay booting to give time to connect a serial monitor

  // Setup debug light pin
  pinMode(DEBUG_LIGHT, OUTPUT);

  // Setup system
  Serial.println("Setting up system...");
  sys.setupSystem();

  //start web ui and MQTT broker
  Serial.println("Setting up Web UI and MQTT...");
  webUI.setupWebConn();
  webUI.setupWebUI();
  broker.setupBroker();
  client.setupClient();

  //register observers
  client.registerObserver(&webUI);
  client.registerObserver(&eventHandler);
  webUI.registerObserver(&eventHandler);
  

  Serial.println("System Booted");
}


// last time system was updated
static long unsigned lastTime = 0;

void loop() {
  Serial.println("Starting Main Loop...");

  //##### UPDATE SYSTEM #####
	if(millis() > lastTime + 500) {
		sys.updateSystem();
		lastTime = millis();
	}

  //##### UPDATE WEB AND BROKER #####
  webUI.updateWeb();
  broker.updateBroker();
  client.updateClient();

  //##### UART SERIAL INTERFACE #####
	if(Serial.available()) {
		switch(Serial.read()) {
			case 'w': //Print IP details
				Serial.println(WiFi.localIP());
				break;
			case 'W': //Reconnect wifi
				webUI.connectWifi();
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

  Serial.println("Main Loop Done!");
}





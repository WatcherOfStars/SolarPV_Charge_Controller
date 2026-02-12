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
    std::cout << "Received MQTT notification for topic: " << topic << ", message: " << message << std::endl;
}

void mainEventHandler::notifyWebUI(char* topic, char* message) {
    // Handle WebUI notifications here
    std::cout << "Received WebUI notification for topic: " << topic << ", message: " << message << std::endl;
    if (strcmp(topic, "test/topic") == 0) {
        // Example: If the topic is "test/topic", print the message
        std::cout << "Handling test/topic with message: " << message << std::endl;
    }
    if (strcmp(topic, "start_client") == 0) {
        // Example: If the topic is "start_client", perform some action
        std::cout << "Handling start_client with message: " << message << std::endl;
        if (strcmp(message, "true") == 0) {
            // Start the MQTT client or perform some related action
            std::cout << "Starting MQTT client..." << std::endl;
            client.setupClient();
        }
    }
}


// Create global objects
//SystemManager sys; // Create System object
BrokerManager broker; // Create BrokerManager object
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
  //sys.setupSystem();

  //start web ui and MQTT broker
  Serial.println("Setting up web connection...");
  webUI.setupWebConn();
  Serial.println("Setting up Web UI");
  webUI.setupWebUI();
  Serial.println("Setting up mqtt broker...");
  broker.setupBroker();
  Serial.println("Setting up mqtt client...");
  webUI.setBroker(&broker);

  // webUI.updateWeb();
  // broker.updateBroker();

  //client.setupClient();

  //register observers
  Serial.println("Registering observers...");
  client.registerObserver(&webUI);
  client.registerObserver(&eventHandler);
  webUI.registerObserver(&eventHandler);
  

  Serial.println("System Booted");
}


// last time system was updated
static long unsigned lastTime = 0;

void loop() {

  //##### UPDATE SYSTEM #####
	if(millis() > lastTime + 5000) {
    //Serial.println("Starting Sys Loop...");

		//sys.updateSystem();
		lastTime = millis();
    //Serial.println("Sys Loop Done!");
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
  delay(10); //Small delay to prevent watchdog timer reset, adjust as needed
}





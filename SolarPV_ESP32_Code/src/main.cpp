#include <main.h>

#include <string>
#include <iostream>

using namespace std;

// Event handler methods
void mainEventHandler::onNotify(const char* topic, const char* message) {
    // Handle WebUI notifications here
    std::cout << "Main received notification for topic: " << topic << ", message: " << message << std::endl;
    if (strcmp(topic, "test/topic") == 0) {
        // Example: If the topic is "test/topic", print the message
        std::cout << "Handling test/topic with message: " << message << std::endl;
    }
    if (strcmp(topic, "Start_Client") == 0) {
        std::cout << "Handling Start_Client with message: " << message << std::endl;
        if (strcmp(message, "1") == 0) {
            // Start the MQTT client or perform some related action
            std::cout << "Starting MQTT client..." << std::endl;
            client.setupClient();
        }
    }
    // if(strcmp(topic, "system_update/data") == 0){
    //   std::cout << "Sending data to MQTT: " << message << std::endl;
    //   client.publishToTopic(topic, message);
    // }
}

mainEventHandler eventHandler; // Create main event handler object


void setup(){
  // Start serial communication for debugging
  Serial.begin(115200);
	while(!Serial);

  // Load configuration from JSON file
  Serial.println("Loading configuration...");
  ConfigManager::getInstance().loadConfig("/config.json");
  ConfigManager::getInstance().printConfig();
  
	if(ConfigManager::getInstance().deviceConfig.slow_boot) delay(5000); //Delay booting to give time to connect a serial monitor

  // Setup system
  Serial.println("Setting up system...");
  sys.setupSystem();

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
  client.registerObserver(&eventHandler); // main event handler observes MQTT client for incoming messages
  webUI.registerObserver(&eventHandler); // main event handler observes WebUI for incoming messages
  webUI.registerObserver(&sys); // System observes WebUI for incoming messages
  sys.registerObserver(&webUI); // WebUI observes System for updates to display on the UI
  

  Serial.println("System Booted");
}


// last time system was updated
static long unsigned lastTime = 0;

void loop() {

  //##### UPDATE SYSTEM #####
	if(millis() > lastTime + 2000) {
    Serial.println("Starting Sys Loop...");

		sys.updateSystem();

    // Serial.print("Config BMS MOSI Pin: ");
    // Serial.println(ConfigManager::getInstance().deviceConfig.bms_mosi_pin);
    // Serial.print("Config BMS MISO Pin: ");
    // Serial.println(ConfigManager::getInstance().deviceConfig.bms_miso_pin);
    // Serial.print("Config BMS CLK Pin: ");
    // Serial.println(ConfigManager::getInstance().deviceConfig.bms_clk_pin);
    // Serial.print("Config BMS CS Pin: ");
    // Serial.println(ConfigManager::getInstance().deviceConfig.bms_cs_pin);

		lastTime = millis();
    //Serial.println("Sys Loop Done!");
	}

  //##### UPDATE WEB AND BROKER #####
  webUI.updateWebUI();
  broker.updateBroker();

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
  delay(10); //Small delay to prevent watchdog timer reset
}



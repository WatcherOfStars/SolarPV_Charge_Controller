#include <Arduino.h>
#include <SPI.h>
#include <iostream>
#include <bitset>
#include <LTC6802.h>

// Define SPI pins, make sure CS is pulled low
#define MOSI 23
#define MISO 19
#define SCK 18
#define CS 5
#define BMS_ADDRESS 0x80

#define TEST_PIN 17

// Instantiate LTC6802 object
//static LTC6802 bms = LTC6802(BMS_ADDRESS, CS);

// Define registers as bitsets for SPI communication
std::bitset<2> cmnd; //command register
std::bitset<7> tmp; //temperature register
std::bitset<20> cvr; //raw cell voltages register
std::bitset<8> cfr; //config read register
std::bitset<8> writecfr; //config write register
std::bitset<13> cv; //calculated cell voltage register

float t1 = 0;
bool write_config = true;

void setup() {
  Serial.begin(115200);
  Serial.println("Starting BMS Initialization...");

  //start SPI
  SPI.begin(SCK, MISO, MOSI);
  SPI.setFrequency(500000); //0.5 MHz

  pinMode(TEST_PIN, OUTPUT);

  // //start BMS connection
  // LTC6802::initSPI(23U, 19U, 18U); //MOSI, MISO, SCK
  // bms.cfgRead();         // Read configuration from chip
  // bms.cfgSetCDC(1);      // Measure mode 13ms
  // bms.cfgSetMCI(0x0fff); // Disable interrupts
  // bms.cfgWrite(false);   // Write configuration back to chip

  Serial.println("Initialized chip");
  delay(1000);
  
  cvr[0]=0;
  cfr[0]=0;
  cfr[1]=2;
  writecfr[0]=0;
  writecfr[1]=1;
  writecfr[2]=97;
  cmnd[0] = 0;

  //start timer
  t1 = millis();

  Serial.println("System Booted");
}

void loop() {
  Serial.println("Starting Main Loop...");
  digitalWrite(TEST_PIN, HIGH); // Set test pin high to indicate loop start

  // bms.cfgWrite(false);          // Write configuration back to chip, because chip resets these every 2.5s when nothing happens on SPI
  // bms.temperatureMeasure();     // Measure temperatures on chip
  // bms.temperatureRead();        // Read temperatures from chip
  // bms.temperatureDebugOutput(); // Send temperatures to serial
  // bms.cellsMeasure();           // Measure cell voltages on chip
  // bms.cellsRead();              // Read cell voltages from chip
  // bms.cellsDebugOutput();       // Send cell voltages to serial

  //reset arrays
  Serial.println("Resetting Arrays...");
  cvr[1]=0x04;
  tmp[1]=0x08;
  // for (int i=2; i<=19; i++){
  //   cvr[i]=0;
  // }

  //conversion commands
  Serial.println("Creating Conversion Commands...");
  cmnd[1]=0x30;
  cfr[1]=0x02;

  // for   // for (int i=2; i<=7; i++){
  //   cfr[i]=0;
  // }(int i=2; i<=7; i++){
  //   cfr[i]=0;
  // }

  //write config register every 10 seconds TODO add contidion for pull up or down
  if(millis() - t1 >= 10000){
    //write config
    Serial.println("10s");
    t1 = millis();
  }

  //print current registers
  Serial.println("-----");
  Serial.print("Command Register: ");
  for (int i=0; i<=1; i++){
    Serial.print(cmnd[i]);
  }
  Serial.println();
  Serial.print("Config Register: ");
  for (int i=0; i<=7; i++){
    Serial.print(cfr[i]);
  }
  Serial.println();
  Serial.print("Temperature Register: ");
  for (int i=0; i<=7; i++){
    Serial.print(tmp[i]);
  }
  Serial.println();
  Serial.print("Cell Voltages Register: ");
  for (int i=0; i<=19; i++){
    Serial.print(cvr[i]);
  }
  Serial.println();
  Serial.println("-----");

  //read spi registers
  Serial.println("Reading SPI Registers...");
  delay(5);
  Serial.println("Temp convertion...");
  SPI.transfer(0x30); //temperature conversion
  delay(22);
  cmnd = 0x10;
  Serial.println("Voltage conversion...");
  SPI.transfer(0x10); //voltage conversion
  delay(22);
  Serial.println("Reading cell voltages...");
  cvr = SPI.transfer(0x04); //read cell voltages
  delay(22);
  Serial.println("Reading temperature...");
  tmp = SPI.transfer(0x08); //read temperature
  delay(2);
  Serial.println("Reading config register...");
  cfr = SPI.transfer(0x02); //read config register
  delay(2);

  //print results
  Serial.print("Cell Voltages: ");
  for (int i=0; i<=19; i++){
    Serial.print(cvr[i]);
  }
  Serial.println();
  Serial.print("Temperature: ");
  for (int i=0; i<=7; i++){
    Serial.print(tmp[i]);
  }
  Serial.println();
  Serial.print("Config Register: ");
  for (int i=0; i<=7; i++){
    Serial.print(cfr[i]);
  }
  Serial.println();
  Serial.println("-----");

  // delay(100);
  // digitalWrite(TEST_PIN, LOW); // Set test pin low to indicate loop end
  sleep(1); //wait 5 seconds before next loop

}

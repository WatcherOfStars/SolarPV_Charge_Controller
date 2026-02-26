#include <Arduino.h>
#include <SPI.h>
#include <iostream>
#include <bitset>
//#include <LTC6802.h>

// Define SPI pins, make sure CS is pulled low
#define MOSI 23
#define MISO 19
#define SCK 18
#define CS 5
#define BMS_ADDRESS 0x80

// Instantiate LTC6802 object
// static LTC6802 bms = LTC6802(BMS_ADDRESS, CS);

// Define registers as bitsets for SPI communication
std::bitset<2> cmnd; //command register
std::bitset<7> tmp; //temperature register
std::bitset<20> cvr; //raw cell voltages register
std::bitset<8> cfr; //config read register
std::bitset<8> writecfr; //config write register
std::bitset<13> cv; //calculated cell voltage register

float t1 = 0;
bool write_config = true;

static const SPISettings spiSettings = SPISettings(1000000, MSBFIRST, SPI_MODE3);


void setup() {
  //start SPI
  SPI.begin(SCK, MISO, MOSI);

  pinMode(CS, OUTPUT);
  digitalWrite(CS, HIGH); //pull CS high to start
  //LTC6802::initSPI(MOSI, MISO, SCK);

  Serial.begin(115200);
  
  cvr[0]=0;
  cfr[0]=0;
  cfr[1]=2;
  writecfr[0]=0;
  writecfr[1]=1;
  writecfr[2]=97;
  cmnd[0] = 0;

  writeConfig();

  //start timer
  t1 = millis();

  Serial.println("System Booted");
}

// Function prototypes
void printCellVoltages();

void printCellVoltages(){
  word cellvolts[12];
  cellvolts[0] = cvr[0] | ((cvr[1] & 0x0F) << 8);
  cellvolts[1] = ((cvr[1] & 0xf0) >> 4) | (cvr[2] << 4);

  cellvolts[2] = cvr[3] | ((cvr[4] & 0x0F) << 8);
  cellvolts[3] = ((cvr[4] & 0xf0) >> 4) | (cvr[5] << 4);

  cellvolts[4] = cvr[6] | ((cvr[7] & 0x0F) << 8);
  cellvolts[5] = ((cvr[7] & 0xf0) >> 4) | (cvr[8] << 4);

  cellvolts[6] = cvr[9] | ((cvr[10] & 0x0F) << 8);
  cellvolts[7] = ((cvr[10] & 0xf0) >> 4) | (cvr[11] << 4);

  cellvolts[8] = cvr[12] | ((cvr[13] & 0x0F) << 8);
  cellvolts[9] = ((cvr[13] & 0xf0) >> 4) | (cvr[14] << 4);

  cellvolts[10] = cvr[15] | ((cvr[16] & 0x0F) << 8);
  cellvolts[11] = ((cvr[16] & 0xf0) >> 4) | (cvr[17] << 4);

  Serial.print(cellvolts[0] * 1.5 / 1000);
  Serial.print(", ");
  Serial.print(cellvolts[1] * 1.5 / 1000);
  Serial.print(", ");
  Serial.print(cellvolts[2] * 1.5 / 1000);
  Serial.print(", ");
  Serial.print(cellvolts[3] * 1.5 / 1000);
  Serial.print(", ");
  Serial.print(cellvolts[4] * 1.5 / 1000);
  Serial.print(", ");
  Serial.print(cellvolts[5] * 1.5 / 1000);
  Serial.print(", ");
  Serial.print(cellvolts[6] * 1.5 / 1000);
  Serial.print(", ");
  Serial.print(cellvolts[7] * 1.5 / 1000);
  Serial.print(", ");
  Serial.print(cellvolts[8] * 1.5 / 1000);
  Serial.print(", ");
  Serial.print(cellvolts[9] * 1.5 / 1000);
  Serial.print(", ");
  Serial.print(cellvolts[10] * 1.5 / 1000);
  Serial.print(", ");
  Serial.println(cellvolts[11] * 1.5 / 1000);
}


void writeConfig() {
  digitalWrite(CS, LOW);
  SPI.beginTransaction(spiSettings);
  //SPI.transfer(0x80); // Write Config Command (address 0x80)
  SPI.transfer(0x01); // PEC
  writecfr[0]=0;
  writecfr[1]=1;
  writecfr[2]=97;
  byte dcc = 0; //discharge cell bitmask
  byte mci = 0; //mask cell interrupts bitmask
  byte vuv = 125; //undervoltage threshold
  byte vov = 167; //overvoltage threshold
  writecfr[3]=(dcc&255);
  writecfr[4]=((dcc>>8)|(mci<<4&240));
  writecfr[5]=(mci>>4);
  writecfr[6]=vuv;
  writecfr[7]=vov;
  for (int i = 0; i < 8; i++)
  {
    SPI.transfer(writecfr[i]);
  }
  SPI.endTransaction();
  digitalWrite(CS, HIGH);
}

void loop() {
  Serial.println("Starting Main Loop...");
  digitalWrite(CS, LOW); //pull CS low to start communication

  //reset arrays
  Serial.println("Resetting Arrays...");
  cvr[1]=0x04;
  tmp[1]=0x08;
  for (int i=2; i<=19; i++){
    cvr[i]=0;
  }

  //conversion commands
  Serial.println("Creating Conversion Commands...");
  cmnd[1]=0x30;
  cfr[1]=0x02;


  for (int i=2; i<=7; i++){
    cfr[i]=0;
  }

  // //write config register every 10 seconds TODO add contidion for pull up or down
  // if(millis() - t1 >= 10000){
  //   //write config
  //   Serial.println("10s");
  //   t1 = millis();

  // }

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
  SPI.beginTransaction(spiSettings);
  sleep(0.0022); //delay to ensure SPI is ready
  Serial.println("Temp convertion...");
  SPI.transfer(0x30); //temperature conversion
  sleep(0.0022);
  Serial.println("Reading temperature...");
  tmp = SPI.transfer(0x08); //read temperature
  sleep(0.002);
  Serial.println("Voltage conversion...");
  SPI.transfer(0x10); //voltage conversion
  sleep(0.0022);
  Serial.println("Reading cell voltages...");
  cvr = SPI.transfer(0x04); //read cell voltages
  sleep(0.0022);
  Serial.println("Reading config register...");
  cfr = SPI.transfer(0x02); //read config register
  sleep(0.002);
  digitalWrite(CS, HIGH); //pull CS high to end communication
  SPI.endTransaction();

  //print results
  Serial.print("Cell Voltages: ");
  for (int i=0; i<=19; i++){
    Serial.print(cvr[i]);
    printCellVoltages();
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

  delay(1000); //wait 1 second before next loop

}


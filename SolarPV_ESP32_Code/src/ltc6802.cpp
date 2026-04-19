#include <Arduino.h>
#include <SPI.h>
#include <iostream>
#include <bitset>
#include "ltc6802.h"

using namespace constants;

static int BMS_MOSI = 23;
static int BMS_MISO = 19;
static int BMS_SCK = 18;
static int BMS_CS = 5;

// Define registers for SPI communication
byte cmnd[2]; // command register
byte tmp[7]; // temperature register
byte cvr[20]; // raw cell voltages register
byte cfr[6]; // config read register
byte writecfr[6]; // config write register
byte cv[13]; // calculated cell voltage register

byte dcc = 0; // discharge cell tribble
byte mci = 0; // mask cell interrupts
byte vuv = 125; // undervoltage config flag
byte vov = 167; // overvoltage config flag


float t1 = 0;

static const SPISettings spiSettings = SPISettings(1000000, MSBFIRST, SPI_MODE3);


int setupLTC6802() {
    //get config values from config manager
    BMS_CS = ConfigManager::getInstance().deviceConfig.bms_cs_pin;
    BMS_MOSI = ConfigManager::getInstance().deviceConfig.bms_mosi_pin;
    BMS_MISO = ConfigManager::getInstance().deviceConfig.bms_miso_pin;
    BMS_SCK = ConfigManager::getInstance().deviceConfig.bms_clk_pin;

    //start SPI
    SPI.begin(BMS_SCK, BMS_MISO, BMS_MOSI);
    Serial.print("Initialized SPI with SCK: ");
    Serial.print(BMS_SCK);
    Serial.print(", MISO: ");
    Serial.print(BMS_MISO);
    Serial.print(", MOSI: ");
    Serial.println(BMS_MOSI);

    pinMode(BMS_CS, OUTPUT);
    digitalWrite(BMS_CS, HIGH); //pull CS high to start
    delay(10); // allow the LTC6802 supply and reference to settle before SPI traffic
    
    cvr[0]=0;
    cfr[0]=0;
    cfr[1]=2;
    cmnd[0] = 0;

    writeLTCConfig();

    //start timer
    t1 = millis();

    Serial.println("LTC6802 Setup Complete");
    return 1; // Return success code
}

void writeLTCConfig() {
    // Writes config registers per guidelines in datasheet page 27
    // Set configuration register values
    writecfr[0]=0x61;
    writecfr[1]=(dcc&255);
    writecfr[2]=((dcc>>8)|(mci<<4&240));
    writecfr[3]=(mci>>4);
    writecfr[4]=vuv;
    writecfr[5]=vov;
    // Serial.print("Config Register to Write: ");
    // for (int i=0; i<6; i++){
    //     Serial.print(writecfr[i], HEX);
    //     Serial.print(" ");
    // }
    //Serial.println();

    SPI.beginTransaction(spiSettings);
    digitalWrite(BMS_CS, LOW); //pull CS low to start communication

    SPI.transfer(0x01); //send WRCFG byte

    for (int i = 0; i < 6; i++) //6 config registers
    {
        SPI.transfer(writecfr[i]); //send config register bytes
    }

    digitalWrite(BMS_CS, HIGH); //pull CS high to end communication
    SPI.endTransaction();

}

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

uint8_t* getCellVoltages() {
    static uint8_t cellVolts[12];
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

    for (int i = 0; i < 12; i++) {
        cellVolts[i] = (uint8_t)(cellvolts[i] * 1.5 / 1000);
    }

    return cellVolts;
}

void startLTC6802Conversion(const uint8_t cmd)
{
  // sends a single byte command to LTC6802 to start conversion

  SPI.beginTransaction(spiSettings);
  digitalWrite(BMS_CS, LOW);

  SPI.transfer(cmd);

  digitalWrite(BMS_CS, HIGH);
  SPI.endTransaction();

}

void readLTC6802(const uint8_t cmd, const uint8_t numOfRegisters, uint8_t *const arr)
{
  // reads values from specified register into provided array. Will retry if communication error.
  Serial.print("Reading command: 0x");
  Serial.println(cmd, HEX);

  do
  {
    SPI.beginTransaction(spiSettings);
    digitalWrite(BMS_CS, LOW);

    SPI.transfer(cmd); // Send the command to read the specified registers
    
    for (int i = 0; i < numOfRegisters; ++i) // Read the specified number of registers into the provided array
    {
      arr[i] = SPI.transfer(cmd); // should this be 0x00 instead of cmd?
    }
    (void)SPI.transfer(cmd); // Read and intentionally discard PEC byte

    digitalWrite(BMS_CS, HIGH);
    SPI.endTransaction();

    for (int i = 0; i < numOfRegisters; ++i) // Print the read values for debugging
    {
      Serial.print(" 0x");
      Serial.print(arr[i], HEX);
    }
    Serial.println();

  }
  while (arr[0] == 0xff); // Retry if the first byte is 0xFF, which may indicate a communication error
}

void displayLTC6802Config()
{
  const byte expectedCfg0 = writecfr[0];
  const byte readCfg0 = cfr[0];

  Serial.println("Decoded CFG0:");
  Serial.print("  expected raw: 0x");
  Serial.print(expectedCfg0, HEX);
  Serial.print("  read raw: 0x");
  Serial.println(readCfg0, HEX);

  Serial.print("  expected masked (bit7 ignored): 0x");
  Serial.print(expectedCfg0 & CFG0_WDT_INVMSK, HEX);
  Serial.print("  read masked: 0x");
  Serial.println(readCfg0 & CFG0_WDT_INVMSK, HEX);

  Serial.print("  WDT status bit (read only status): ");
  Serial.println((readCfg0 & CFG0_WDT_MSK) ? 1 : 0);

  Serial.print("  GPIO2=");
  Serial.print((readCfg0 & CFG0_GPIO2_MSK) ? 1 : 0);
  Serial.print(" GPIO1=");
  Serial.print((readCfg0 & CFG0_GPIO1_MSK) ? 1 : 0);
  Serial.print(" LVLPL=");
  Serial.print((readCfg0 & CFG0_LVLPL_MSK) ? 1 : 0);
  Serial.print(" CELL10=");
  Serial.print((readCfg0 & CFG0_CELL10_MSK) ? 1 : 0);
  Serial.print(" CDC=");
  Serial.println(readCfg0 & CFG0_CDC_MSK);

  bool cfg0MatchIgnoringWdt = ((expectedCfg0 & CFG0_WDT_INVMSK) == (readCfg0 & CFG0_WDT_INVMSK));
  bool cfg1to5Match = true;
  for (int i = 1; i < 6; ++i)
  {
    if (writecfr[i] != cfr[i])
    {
      cfg1to5Match = false;
      break;
    }
  }

  Serial.print("  CFG0 match ignoring WDT: ");
  Serial.println(cfg0MatchIgnoringWdt ? "YES" : "NO");
  Serial.print("  CFG1..CFG5 exact match: ");
  Serial.println(cfg1to5Match ? "YES" : "NO");
}


void updateLTC6802() {

  //write config register every 2 seconds TODO add contidion for pull up or down
//   if(millis() - t1 >= 2000){
//     writeLTCConfig();
//     Serial.println("Wrote CFR");
//     t1 = millis();
//   }
  writeLTCConfig();

  Serial.println("Reading SPI Registers...");
  // voltage conversion and reading
  startLTC6802Conversion(0x10); //start voltage conversion (all cells)
  delay(10); //delay to ensure conversion is complete before reading
  readLTC6802(0x04, 20, cvr); //read cell voltages

  //temperature conversion and reading
  // measure(0x30); //start temperature conversion (all temps)
  // delay(10); //delay to ensure conversion is complete before reading
  // readValues(0x08, 7, tmp); //read temperatures

  //config register reading
  readLTC6802(0x02, 6, cfr); //read config registers


  //print results
  Serial.print("Cell Voltages: ");
  printCellVoltages();

  // Serial.println();
  // Serial.print("Temperature: ");
  // for (int i=0; i<=6; i++){
  //   Serial.print(tmp[i]);
  // }

  Serial.println();
  Serial.print("Config Register: ");
  for (int i=0; i<6; i++){
    Serial.print(cfr[i], HEX);
    Serial.print(" ");
  }
  Serial.println();
  //printDecodedConfigView();
  Serial.println("-----");

}
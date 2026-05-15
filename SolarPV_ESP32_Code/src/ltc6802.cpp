#include <Arduino.h>
#include <SPI.h>
#include <iostream>
#include <bitset>
#include "ltc6802.h"

using namespace constants;

// Define registers for SPI communication
byte cmnd[2]; // command register
byte tmp[7]; // temperature register
byte cvr[20]; // raw cell voltages register
byte cfr[6]; // config read register
byte writecfr[6]; // config write register
byte cv[13]; // calculated cell voltage register

byte maxdcc = 0; // max discharge value based on number of cells
uint16_t dcc = 0; // discharge cell tribble
byte mci = 0; // mask cell interrupts
byte vuv = 125; // undervoltage config flag
byte vov = 167; // overvoltage config flag


float t1 = 0;

static const SPISettings spiSettings = SPISettings(1000000, MSBFIRST, SPI_MODE3);

DeviceConfig device;

static int BMS_CS; // chip select pin for LTC6802, set in config


int setupLTC6802() {
  device = ConfigManager::getInstance().deviceConfig;
  BMS_CS = device.bms_cs_pin;

  maxdcc = (1 << device.num_cells) - 1; // calculate max discharge value based on number of cells

  //start SPI
  SPI.begin(device.bms_clk_pin, device.bms_miso_pin, device.bms_mosi_pin);

  pinMode(BMS_CS, OUTPUT);
  digitalWrite(BMS_CS, HIGH); //pull CS high to start
  delay(10); // allow the LTC6802 supply and reference to settle before SPI traffic

  Serial.println("SPI pins: CLK: " + String(device.bms_clk_pin) + ", MISO: " + String(device.bms_miso_pin) + ", MOSI: " + String(device.bms_mosi_pin) + ", CS: " + String(BMS_CS));

  writeLTCConfig();

  //return fail code (-1) if config readback doesn't match what was written
  if(readLTC6802(0x02, 6, cfr) == -1){
    Serial.println("Failed to read back LTC6802 config registers");
    return -1; // read config registers into tmp buffer
  }
  for (int i=1; i<6; i++){ // exclude index 0
    if (cfr[i] != writecfr[i]){
      Serial.print("LTC6802 config readback mismatch at byte ");      Serial.print(i);
      Serial.print(": expected ");      Serial.print(writecfr[i], HEX);
      Serial.print(", got ");      Serial.println(cfr[i], HEX);
      return -1;
    }
  }

  //start timer
  //t1 = millis();

  Serial.println("LTC6802 Setup Successful");
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
    Serial.print("Config Register to Write: ");
    for (int i=0; i<6; i++){
        Serial.print(writecfr[i], HEX);
        Serial.print(" ");
    }
    Serial.println();

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

float* getCellVoltages() {
    static float cellVolts[12];
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
        cellVolts[i] = (float)(cellvolts[i] * 1.5 / 1000);
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

int readLTC6802(const uint8_t cmd, const uint8_t numOfRegisters, uint8_t *const arr)
{
  // reads values from specified register into provided array. Will retry if communication error.
  // Returns -1 if timeout occurs, 1 on success
  // Serial.print("Reading command: 0x");
  // Serial.println(cmd, HEX);

  unsigned long startTime = millis();
  const unsigned long timeout = 1000; // 1 second timeout

  do
  {
    if (millis() - startTime >= timeout) {
      Serial.println("ERROR: readLTC6802 timeout");
      return -1;
    }

    SPI.beginTransaction(spiSettings);
    digitalWrite(BMS_CS, LOW);

    SPI.transfer(cmd); // Send the command to read the specified registers
    
    for (int i = 0; i < numOfRegisters; ++i) // Read the specified number of registers into the provided array
    {
      arr[i] = SPI.transfer(0x00); // Read register data
    }
    (void)SPI.transfer(cmd); // Read and intentionally discard PEC byte

    digitalWrite(BMS_CS, HIGH);
    SPI.endTransaction();

  }
  while (arr[0] == 0xff); // Retry if the first byte is 0xFF, which may indicate a communication error

  return 1; // Success
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


int updateLTC6802() {
  // Safety check: ensure num_cells is configured
  if (device.num_cells == 0) {
      Serial.println("ERROR: num_cells not configured! Config may not have loaded properly.");
      return -1;
  }

  //write config register every 2 seconds TODO add contidion for pull up or down
  // if(millis() - t1 >= 2000){
  //   writeLTCConfig();
  //   Serial.println("Wrote CFR");
  //   t1 = millis();
  // }
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
  // for (int i=1; i<6; i++){ // exclude index 0
  //   if (cfr[i] != writecfr[i]){
  //     Serial.print("LTC6802 config readback mismatch at byte ");      Serial.print(i);
  //     Serial.print(": expected ");      Serial.print(writecfr[i], HEX);
  //     Serial.print(", got ");      Serial.println(cfr[i], HEX);
  //     return -1;
  //   }
  // }

  //print results
  Serial.print("Cell Voltages: ");
  printCellVoltages();

  // Serial.println();
  // Serial.print("Temperature: ");
  // for (int i=0; i<=6; i++){
  //   Serial.print(tmp[i]);
  // }

  // Serial.print("Config Register: ");
  // for (int i=0; i<6; i++){
  //   Serial.print(cfr[i], HEX);
  //   Serial.print(" ");
  // }
  // Serial.println();
  //printDecodedConfigView();
  return 1; // Return success code
}

String pullDownBalance(float *cellVoltages, float *packAverage){
  String vstr;
  dcc = 0;
  // pulls down high cells
  for (int cell = 0; cell < device.num_cells; cell++){ // for each cell
    float bufferCV = cellVoltages[cell] - 0.02; // add small buffer to prevent rapid toggling
    if (bufferCV > *packAverage && cellVoltages[cell] > device.cell_balance_start){ // if cell is above average and above balance start threshold
      dcc |= (1 << cell); // set bit for this cell to pull down
      vstr += "d";
    }
    else if (cellVoltages[cell] <= *packAverage){ // if cell is at or below average, stop pulling down
      //dcc &= maxdcc - (1 << cell); // clear bit for this cell to stop pulling down
      dcc &= ~(1 << cell); // clear bit for this cell to stop pulling down
      vstr += "w";
    }
    else vstr += "_";

  }
  Serial.println();
  Serial.print("DCC after pull down balance: ");
  Serial.print(dcc, BIN);
  Serial.print(";   ");
  Serial.println(vstr);
  return vstr;
}
String pullUpBalance(float *cellVoltages, float *packAverage, int *minCellIndex){
  String vstr;
  dcc = 0;
  // pulls up low cells
  for (int cell = 0; cell < device.num_cells; cell++){ // for each cell
    //
    if (cell == *minCellIndex || cellVoltages[cell] <= device.cell_balance_start){ // if cell is above balance start threshold and not the minimum cell
      dcc &= ~(1 << cell); // set bit for this cell to pull down
      vstr += "w";
      //Serial.print("Cell "); Serial.print(cell); Serial.print(" pulled low. DCC: "); Serial.println(dcc, BIN);
    }
    else{
      dcc |= (1 << cell); // clear bit for this cell to stop pulling down
      vstr += "d";
      //Serial.print("Cell "); Serial.print(cell); Serial.print(" pulled high DCC: "); Serial.println(dcc, BIN);
    }
  }
  Serial.print("DCC after pull up balance: ");
  Serial.print(dcc, BIN);
  Serial.print(";   ");
  Serial.println(vstr);
  return vstr;
}
void stopBalance(){
  dcc = 0; // clear all bits to stop balancing
}

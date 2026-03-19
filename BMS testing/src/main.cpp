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

// Config register bitmasks and bit positions
static const byte CFG0_WDT_BIT    = 7;    /**< Configuration register 0 watchdog timer bit. */
static const byte CFG0_GPIO2_BIT  = 6;    /**< Configuration register 0 GPIO2 bit. */
static const byte CFG0_GPIO1_BIT  = 5;    /**< Configuration register 0 GPIO1 bit. */
static const byte CFG0_LVLPL_BIT  = 4;    /**< Configuration register 0 level polling mode bit. */
static const byte CFG0_CELL10_BIT = 3;    /**< Configuration register 0 10-cell mode bit. */
static const byte CFG0_WDT_MSK    = 0x80; /**< Configuration register 0 watchdog timer bitmask. */
static const byte CFG0_GPIO2_MSK  = 0x40; /**< Configuration register 0 GPIO2 bitmask. */
static const byte CFG0_GPIO1_MSK  = 0x20; /**< Configuration register 0 GPIO1 bitmask. */
static const byte CFG0_LVLPL_MSK  = 0x10; /**< Configuration register 0 level polling bitmask. */
static const byte CFG0_CELL10_MSK = 0x08; /**< Configuration register 0 10-cell mode bitmask. */
static const byte CFG0_CDC_MSK    = 0x07; /**< Configuration register 0 comparator duty cycle bitmask. */
static const byte CFG1_DCC_MSK    = 0xff; /**< Configuration register 1 discharge cell bitmask. */
static const byte CFG2_DCC_MSK    = 0x0f; /**< Configuration register 2 discharge cell bitmask. */
static const byte CFG2_MCI_MSK    = 0xf0; /**< Configuration register 2 mask cell interrupts bitmask. */
static const byte CFG3_MCI_MSK    = 0xff; /**< Configuration register 3 mask cell interrupts bitmask. */
static const byte CFG0_WDT_INVMSK    = 0x7f; /**< Configuration register 0 watchdog timer inverse bitmask. */
static const byte CFG0_GPIO2_INVMSK  = 0xbf; /**< Configuration register 0 GPIO2 inverse bitmask. */
static const byte CFG0_GPIO1_INVMSK  = 0xdf; /**< Configuration register 0 GPIO1 inverse bitmask. */
static const byte CFG0_LVLPL_INVMSK  = 0xef; /**< Configuration register 0 level polling mode inverse bitmask. */
static const byte CFG0_CELL10_INVMSK = 0xf7; /**< Configuration register 0 10-cell mode inverse bitmask. */
static const byte CFG0_CDC_INVMSK    = 0xf8; /**< Configuration register 0 comparator duty cycle inverse bitmask. */
static const byte CFG1_DCC_INVMSK    = 0x00; /**< Configuration register 1 discharge cell inverse bitmask. */
static const byte CFG2_DCC_INVMSK    = 0xf0; /**< Configuration register 2 discharge cell inverse bitmask. */
static const byte CFG2_MCI_INVMSK    = 0x0f; /**< Configuration register 2 mask cell interrupts inverse bitmask. */
static const byte CFG3_MCI_INVMSK    = 0x00; /**< Configuration register 3 mask cell interrupts inverse bitmask. */

// Instantiate LTC6802 object
//static LTC6802 bms = LTC6802(BMS_ADDRESS, CS);

// Define registers as bitsets for SPI communication
// std::array<uint8_t, 2> cmnd; //command register
// std::array<uint8_t, 7> tmp; //temperature register
// std::array<uint8_t, 20> cvr; //raw cell voltages register
// std::array<uint8_t, 8> cfr; //config read register
// std::array<uint8_t, 8> writecfr; //config write register
// std::array<uint8_t, 13> cv; //calculated cell voltage register
byte cmnd[2]; // command register
byte tmp[7]; // temperature register
byte cvr[20]; // raw cell voltages register
byte cfr[8]; // config read register
byte writecfr[6]; // config write register
byte cv[13]; // calculated cell voltage register

byte dcc = 0; // discharge cell tribble
byte mci = 0; // mask cell interrupts
byte vuv = 125; // undervoltage config flag
byte vov = 167; // overvoltage config flag


float t1 = 0;
bool write_config = true;

static const SPISettings spiSettings = SPISettings(1000000, MSBFIRST, SPI_MODE3);

void writeConfig();

void writeConfig() {
  // Writes config registers per guidelines in datasheet page 27
  
  // Set configuration register values
  writecfr[0]=1;
	writecfr[1]=97;
	writecfr[2]=(dcc&255);
	writecfr[3]=((dcc>>8)|(mci<<4&240));
	writecfr[4]=(mci>>4);
	writecfr[5]=vuv;
	writecfr[6]=vov;

  digitalWrite(CS, LOW); //pull CS low to start communication
  SPI.beginTransaction(spiSettings);
  SPI.transfer(0x01); //send WRCFG byte

  for (int i = 0; i < 6; i++) //6 config registers
  {
    SPI.transfer(writecfr[i]); //send config register bytes
  }

  SPI.endTransaction();
  digitalWrite(CS, HIGH); //pull CS high to end communication
}

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

void measure(const byte cmd); // call before reading values to start conversion
void measure(const byte cmd)
{
  SPI.beginTransaction(spiSettings);
  digitalWrite(CS, LOW);

  SPI.transfer(cmd);

  digitalWrite(CS, HIGH);
  SPI.endTransaction();
  sleep(0.0022); //delay to ensure measurement is complete before reading
}

void readValues(const byte cmd, const byte numOfRegisters, byte *const arr); // call after measure to read values from specified registers
void readValues(const byte cmd, const byte numOfRegisters, byte *const arr)
{
  do
  {
    SPI.beginTransaction(spiSettings);
    digitalWrite(CS, LOW);

    SPI.transfer(cmd); // Send the command to read the specified registers
    Serial.print("Reading command: 0x");
    Serial.println(cmd, HEX);

    for (int i = 0; i < numOfRegisters; ++i) // Read the specified number of registers into the provided array
    {
      arr[i] = SPI.transfer(0x00); // should this be 0x00 instead of cmd?
      Serial.print("  Register ");      Serial.print(i);
      Serial.print(": 0x");      Serial.print(arr[i], HEX);
    }
    (void)SPI.transfer(cmd); // Read and intentionally discard PEC byte
    Serial.println("  (PEC byte discarded)");

    digitalWrite(CS, HIGH);
    SPI.endTransaction();
  }
  while (arr[0] == 0xff); // Retry if the first byte is 0xFF, which may indicate a communication error
  sleep(0.0022);
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
  for (int i=0; i<=6; i++){
    Serial.print(tmp[i]);
  }
  Serial.println();

  Serial.print("Cell Voltages Register: ");
  for (int i=0; i<=19; i++){
    Serial.print(cvr[i]);
  }
  Serial.println();
  Serial.println("-----");

  // //read spi registers
  // Serial.println("Reading SPI Registers...");
  // SPI.beginTransaction(spiSettings);
  // digitalWrite(CS, LOW); //pull CS low to start communication
  // sleep(0.0022); //delay to ensure SPI is ready
  // Serial.println("Temp convertion...");
  // SPI.transfer(0x30); //temperature conversion
  // sleep(0.0022);
  // Serial.println("Reading temperature...");
  // tmp = SPI.transfer(0x08); //read temperature
  // sleep(0.002);
  // Serial.println("Voltage conversion...");
  // SPI.transfer(0x10); //voltage conversion
  // sleep(0.0022);
  // Serial.println("Reading cell voltages...");
  // cvr = SPI.transfer(0x04); //read cell voltages
  // sleep(0.0022);
  // Serial.println("Reading config register...");
  // cfr = SPI.transfer(0x02); //read config register
  // sleep(0.002);
  // digitalWrite(CS, HIGH); //pull CS high to end communication
  // SPI.endTransaction();

  Serial.println("Reading SPI Registers...");
  Serial.println("Temp conversion...");
  measure(0x30); //temperature conversion
  Serial.println("Reading temperature...");
  readValues(0x08, 8, tmp); //read temperature
  Serial.println("Voltage conversion...");
  measure(0x10); //voltage conversion
  Serial.println("Reading cell voltages...");
  readValues(0x04, 20, cvr); //read cell voltages
  Serial.println("Reading config register...");
  readValues(0x02, 8, cfr); //read config register

  //print results
  Serial.print("Cell Voltages: ");
  printCellVoltages();
  // for (int i=0; i<=19; i++){
  //   Serial.print(cvr[i]);
    
  // }
  Serial.println();
  Serial.print("Temperature: ");
  for (int i=0; i<=6; i++){
    Serial.print(tmp[i]);
  }
  Serial.println();
  Serial.print("Config Register: ");
  for (int i=0; i<=7; i++){
    Serial.print(cfr[i]);
  }
  Serial.println();
  Serial.println("-----");

  delay(5000); //wait 5 seconds before next loop

}


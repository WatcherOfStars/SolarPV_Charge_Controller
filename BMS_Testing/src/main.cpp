#include <Arduino.h>
#include <SPI.h>
#include <iostream>
#include <bitset>

// Define SPI pins, make sure CS is pulled low
#define MOSI 8
#define MISO 7
#define SCK 6

// Define registers as bitsets for SPI communication
std::bitset<2> cmnd; //command register
std::bitset<7> tmp; //temperature register
std::bitset<20> cvr; //raw cell voltages register
std::bitset<8> cfr; //config read register
std::bitset<8> writecfr; //config write register
std::bitset<13> cv; //calculated cell voltage register


void setup() {
  //start SPI
  SPI.begin(SCK, MISO, MOSI);

  Serial.begin(115200);
  Serial.println("System Booted");
  
  cvr[0]=0;
  cfr[0]=0;
  cfr[1]=2;
  writecfr[0]=0;
  writecfr[1]=1;
  writecfr[2]=97;
  cmnd[0] = 0;
}

void loop() {
  //reset arrays
  cvr[1]=0x04;
  tmp[1]=0x08;
  for (int i=2; i<=19; i++){
    cvr[i]=0;
  }

  //temperature conversion
  cmnd[1]=0x30;
  cfr[1]=2;

  for (int i=2; i<=7; i++){
    cfr[i]=0;
  }

  if(millis()%10000==0){
    //write config
  }

  //read spi registers
  sleep(5);
  SPI.transfer(cmnd.to_ulong());
  sleep(22);
  cmnd = 0x10;
  SPI.transfer(cmnd.to_ulong());
  sleep(22);
  SPI.transfer(cvr.to_ulong());
  sleep(22);
  SPI.transfer(cvr.to_ulong());
  sleep(2);
  SPI.transfer(tmp.to_ulong());
  sleep(2);
  SPI.transfer(cfr.to_ulong());
  sleep(2);

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
  delay(1000);

}

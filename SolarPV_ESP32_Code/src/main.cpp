#include <Arduino.h>
#include <vars.h>
#include <Wire.h>
#include <INA226.h>
#include <DS3231-RTC.h>

INA226 ina(0x40); // Create an INA226 object with the default I2C address
DS3231 rtc_clock; // create clock object, see https://github.com/hasenradball/DS3231-RTC

void setup(){
  Wire.begin(); // Initialize I2C

  Serial.begin(115200);
  Serial.println("System Booted");

  pinMode(DEBUG_LIGHT, OUTPUT);

  //check for ina226 shunt
  if (!ina.begin()) {
    Serial.println("INA226 not found!");
    while (1);
  }
  // Configure the INA226 (e.g., calibration, averaging)
  //ina.setCalibration(0.01, 10.0); // Example: 0.01 Ohm shunt, 10A max current
  ina.setMaxCurrentShunt(60); //60A max current
  ina.setAverage(4); // Set averaging to 4 samples

  //setup real time clock
}

void loop() {
  digitalWrite(DEBUG_LIGHT, HIGH);
  Serial.println("ON");
  delay(500);
  digitalWrite(DEBUG_LIGHT, LOW);
  Serial.println("OFF");
  delay(500);

  update_shunt();

}

void update_shunt(){
  float busVoltage = ina.getBusVoltage_mV();
  float shuntVoltage = ina.getShuntVoltage_mV();
  float current = ina.getCurrent_mA();
  float power = ina.getPower_mW();

  Serial.print("Bus Voltage: ");
  Serial.print(busVoltage);
  Serial.println(" V");

  Serial.print("Shunt Voltage: ");
  Serial.print(shuntVoltage);
  Serial.println(" mV");

  Serial.print("Current: ");
  Serial.print(current);
  Serial.println(" mA");

  Serial.print("Power: ");
  Serial.print(power);
  Serial.println(" mW");
}

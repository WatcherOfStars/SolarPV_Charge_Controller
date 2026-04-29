# SolarPV_Charge_Controller

## Project Overview
This is the firmware for a low-cost DC solar charge controller running on an esp32-wroom. Our custom PCB and circuit can be found here: . Electrical components include:
- (1x) ESP32-WROOM devboard
- (2x) INA226 current monitoring shunts
- (1x) Donated magna e-car LiPo cell supervision unit. Any cell supervisor circuit using the LTC6802G-1 chip will work.
- (1x) Real Time Clock module
- (1x) 24-48V to 5V buck converter

## Installation & Use
### Assembly
### 

## Safety Testing Procedures

### Disconnections
- Solar shunt
- Load shunt
- Cell Supervisor
- RTC
- Panels
- Batteries
- Temperature

### Overcurrent
### FET failures
### Battery undercharge/overcharge
### High / low temperatures
### WiFi communication failure
### Shorts
### Surges
### Full shutdown
### Complex Failures
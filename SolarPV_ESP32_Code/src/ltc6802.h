#ifndef LTC6802_H
#define LTC6802_H

#include <Arduino.h>
#include <config.h>

int setupLTC6802();
void updateLTC6802();
void writeLTCConfig();
void printCellVoltages();
void displayLTC6802Config();
void startLTC6802Conversion(const byte command);
void readLTC6802(const byte command, const int numBytes, byte* buffer);
uint8_t* getCellVoltages();

namespace constants {
    inline const int BMS_MOSI = ConfigManager::getInstance().deviceConfig.bms_mosi_pin;
    inline const int BMS_MISO = ConfigManager::getInstance().deviceConfig.bms_miso_pin;
    inline const int BMS_SCK = ConfigManager::getInstance().deviceConfig.bms_clk_pin;
    inline const int BMS_CS = ConfigManager::getInstance().deviceConfig.bms_cs_pin;

    // Config register bitmasks and bit positions
    inline const byte CFG0_WDT_BIT    = 7;    /**< Configuration register 0 watchdog timer bit. */
    inline const byte CFG0_GPIO2_BIT  = 6;    /**< Configuration register 0 GPIO2 bit. */
    inline const byte CFG0_GPIO1_BIT  = 5;    /**< Configuration register 0 GPIO1 bit. */
    inline const byte CFG0_LVLPL_BIT  = 4;    /**< Configuration register 0 level polling mode bit. */
    inline const byte CFG0_CELL10_BIT = 3;    /**< Configuration register 0 10-cell mode bit. */
    inline const byte CFG0_WDT_MSK    = 0x80; /**< Configuration register 0 watchdog timer bitmask. */
    inline const byte CFG0_GPIO2_MSK  = 0x40; /**< Configuration register 0 GPIO2 bitmask. */
    inline const byte CFG0_GPIO1_MSK  = 0x20; /**< Configuration register 0 GPIO1 bitmask. */
    inline const byte CFG0_LVLPL_MSK  = 0x10; /**< Configuration register 0 level polling bitmask. */
    inline const byte CFG0_CELL10_MSK = 0x08; /**< Configuration register 0 10-cell mode bitmask. */
    inline const byte CFG0_CDC_MSK    = 0x07; /**< Configuration register 0 comparator duty cycle bitmask. */
    inline const byte CFG1_DCC_MSK    = 0xff; /**< Configuration register 1 discharge cell bitmask. */
    inline const byte CFG2_DCC_MSK    = 0x0f; /**< Configuration register 2 discharge cell bitmask. */
    inline const byte CFG2_MCI_MSK    = 0xf0; /**< Configuration register 2 mask cell interrupts bitmask. */
    inline const byte CFG3_MCI_MSK    = 0xff; /**< Configuration register 3 mask cell interrupts bitmask. */
    inline const byte CFG0_WDT_INVMSK    = 0x7f; /**< Configuration register 0 watchdog timer inverse bitmask. */
    inline const byte CFG0_GPIO2_INVMSK  = 0xbf; /**< Configuration register 0 GPIO2 inverse bitmask. */
    inline const byte CFG0_GPIO1_INVMSK  = 0xdf; /**< Configuration register 0 GPIO1 inverse bitmask. */
    inline const byte CFG0_LVLPL_INVMSK  = 0xef; /**< Configuration register 0 level polling mode inverse bitmask. */
    inline const byte CFG0_CELL10_INVMSK = 0xf7; /**< Configuration register 0 10-cell mode inverse bitmask. */
    inline const byte CFG0_CDC_INVMSK    = 0xf8; /**< Configuration register 0 comparator duty cycle inverse bitmask. */
    inline const byte CFG1_DCC_INVMSK    = 0x00; /**< Configuration register 1 discharge cell inverse bitmask. */
    inline const byte CFG2_DCC_INVMSK    = 0xf0; /**< Configuration register 2 discharge cell inverse bitmask. */
    inline const byte CFG2_MCI_INVMSK    = 0x0f; /**< Configuration register 2 mask cell interrupts inverse bitmask. */
    inline const byte CFG3_MCI_INVMSK    = 0x00; /**< Configuration register 3 mask cell interrupts inverse bitmask. */
}



#endif
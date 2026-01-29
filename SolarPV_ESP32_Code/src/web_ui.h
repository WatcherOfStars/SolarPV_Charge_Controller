#ifndef WEB_UI_H
#define WEB_UI_H

//consts
// namespace constants { inline constexpr double PI = 3.14; }
namespace constants {
    inline constexpr int SLOW_BOOT = 0; //Delay booting to give time to connect a serial monitor
    inline constexpr char* HOSTNAME = "ESPUITest";
    inline constexpr int FORCE_USE_HOTSPOT = 0;
}

// Forward declaration
class Control;

//function prototypes
void connectWifi();
void setUpUI();
void enterWifiDetailsCallback(Control *sender, int type);
void textCallback(Control *sender, int type);
void sendTestPub(Control *sender, int type);
void generalCallback(Control *sender, int type);
void updateCallback(Control *sender, int type);
void getTimeCallback(Control *sender, int type);
void graphAddCallback(Control *sender, int type);
void graphClearCallback(Control *sender, int type);
void extendedCallback(Control* sender, int type, void* param);



//class definitions

#endif
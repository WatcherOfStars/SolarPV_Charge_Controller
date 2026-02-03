#include<Arduino.h>
#define light 2
#define switch 0



void setup(){
    Serial.begin(115200);
    pinMode(light, OUTPUT);
    pinMode(switch, OUTPUT);

}
void loop(){
    digitalWrite(light, HIGH);
    digitalWrite(switch, HIGH);
    delay(10000);
    digitalWrite(light, LOW);
    digitalWrite(switch, LOW);
}
#ifndef OBSERVER_H
#define OBSERVER_H

#include <Arduino.h>

//observer and subject class prototypes
class observer{ //todo: do we need aliases for topic and message types?
public:
    virtual void onNotify(const char* topic, const char* message) = 0; //function to notify observers of incoming messages. 
    virtual ~observer() = default; //destructor
};

class subject{
public:
    virtual void registerObserver(observer* obs) = 0;
    virtual void removeObserver(observer* obs) = 0;
    virtual void notifyObservers(const char* topic, const char* message) = 0;
    virtual ~subject() = default; //destructor
};

#endif
// valves.h

// definition of public and private objects
#ifndef valves_h
#define valves_h

#include <Arduino.h>

class valves {
  private:
    int pwm= 0;
    int directionPin = 13;
    int pwmPin = 11;
    int brakePin = 8;
  public:
    valves();
    void turnON();
    void turnOFF();
    int getState();
    char valvesState;

};

#endif
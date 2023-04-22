// pump.h

// definition of public and private objects
#ifndef pump_h
#define pump_h

#include <Arduino.h>

class pump {
  private:
    int pwm;
    
    int directionPin = 12;
    int pwmPin = 3;
    int brakePin = 9;
  public:
    char pump State; // debugger helps see in whitch state the pump is
    unsigned char pumpROT; // set the direction of the pump rotation
    pump(int pwm);
    void turnON(char pumpROT);
    void turnOFF();
    int getState();

};

#endif
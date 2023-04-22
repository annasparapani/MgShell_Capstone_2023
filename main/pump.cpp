// pump.cpp

// definition of methods

#include "pump.h"

pump::pump(int pwm) {


  Pumppwm = pwm; // initialize pwm
  
  pumpState = 'OFF' // Initially the pump is off untill it's activated

  // ho riportato di nuovo i valori dei pin (potrebbe non essere utile perchè li ho)
  int directionPin = 12; 
  int pwmPin = 3;
  int brakePin = 9;
  pinMode(directionPin, OUTPUT);
  pinMode(pwmPin, OUTPUT);
  pinMode(brakePin, OUTPUT);  
}

void pump::turnON(char pumpROT) {
  ROT = pumpROT;
  pumpState= 'ON';
  // Set pump direction --> activate pump
  digitalWrite(directionPin,ROT);

  //release breaks
  digitalWrite(brakePin, LOW);

  //set work duty for the motor
  analogWrite(pwmPin, pwm);


}

void pump::turnOFF() {

  pumpState='OFF';
  //activate breaks
  digitalWrite(brakePin, HIGH);

  //set work duty for the motor to 0 (off)
  analogWrite(pwmPin, 0);
}

int pump::getState() {
  return pumpState;
}

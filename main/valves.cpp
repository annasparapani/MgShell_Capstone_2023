// valves.cpp

// definition of methods

#include "valves.h"

valves::valves() {
  
  char valvesState = 'CLOSED'
  // ho riportato di nuovo i valori dei pin (potrebbe non essere utile perchè li ho)
  int pwm= 100;
  int directionPin = 13;
  int pwmPin = 11;
  int brakePin = 8;
  pinMode(directionPin, OUTPUT);
  pinMode(pwmPin, OUTPUT);
  pinMode(brakePin, OUTPUT);  
}

void valves::turnON() {

  valvesState = 'OPEN';
  // Set pump direction --> activate pump
  digitalWrite(directionPin, HIGH);

  //release breaks
  digitalWrite(brakePin, LOW);

  //set work duty for the motor
  analogWrite(pwmPin, pwm);



}

void valves::turnOFF() {
  
  valvesState = 'CLOSED';
  //activate breaks
  digitalWrite(brakePin, HIGH);

  //set work duty for the motor to 0 (off)
  analogWrite(pwmPin, 0);
}

int valves::getState() {
  return valvesState;
}

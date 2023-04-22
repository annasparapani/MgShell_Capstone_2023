// main code: ArduinoCode.ino
#include "pump.h"
#include "valves.h"
// set tuning parameters from here

// pwm for the pump
int pwm = 10;

// activation & sleep time for the whole system
int activation_time = 2000;
int measurement_time= 3000; 
int sleep_time = 10000;


pump p_pump(pwm); // create a pump object  with a set pwm
valves e_valves(); // create a valves object


void setup() {
  Serial.begin(9600);
}

void loop() {

  // FILL  THE CHAMBER

  e_valves.turnON(); // activate the valves

  delay(2000); //wait untill the valves are fully opened

  p_pump.turnON('HIGH'); // activate the rotation in one direction
  delay(activation_time)) // wait untill filled
  p_pump.turnOFF();

  delay(2000); // wait untill the pump has fully stopped

  e_valves.turnOFF() // turn off the valve

  // DEACTIVATE WHILE MEASURING PH
  delay(measurement_time) // wait untill the pH sensor has finished the measurement

  // EMPTY THE CHAMBER
  p_pump.turnON('LOW'); // activate the rotation in the opposite direction
  delay(activation_time)) // wait untill emptied
  p_pump.turnOFF();

  delay(2000); // wait untill the pump has fully stopped
  e_valves.turnOFF() // close the valves

  //SLEEP
  delay(sleep_time) // wait untill the next measurement phase

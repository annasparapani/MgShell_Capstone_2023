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
bool valves_linked = true;

pump p_pump(pwm); // create a pump object  with a set pwm
valves e_valves(); // create a valves object


void setup() {
  Serial.begin(9600);
}

void loop() {

  // FILL  THE CHAMBER
  if (valves_linked) {

    e_valves.turnON(); // activate the valves

    delay(2000); //wait untill the valves are fully opened
  };

  p_pump.turnON('HIGH'); // activate the rotation in one direction
  delay(activation_time)) // wait untill filled
  p_pump.turnOFF();

  delay(2000); // wait untill the pump has fully stopped
  if (valves_linked) {
    e_valves.turnOFF(); // close the circuit to hold the liquid level
  };
   // turn off the valve

  // DEACTIVATE WHILE MEASURING PH
  delay(measurement_time);// wait untill the pH sensor has finished the measurement

  // PH MEASUREMENT PHASE

  // add here the code to measure the ph with the probe (build a proper PH measurement function or class)

  // EMPTY THE CHAMBER

  if (valves_linked) {

    e_valves.turnON(); // activate the valves

    delay(2000); //wait untill the valves are fully opened
  };

  p_pump.turnON('LOW'); // activate the rotation in the opposite direction
  delay(activation_time);// wait untill emptied
  p_pump.turnOFF();

  if (valves_linked) {
      delay(2000); // wait untill the pump has fully stopped
      e_valves.turnOFF() // close the valves
  };

  //SLEEP
  delay(sleep_time) // wait untill the next measurement phase

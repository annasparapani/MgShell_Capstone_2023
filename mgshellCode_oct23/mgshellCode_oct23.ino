/* MgShell incubator code 
  pH measuring trial and GUI setup 

  ****** PIN CONNECTIONS **********
  Rx EZO - 10 (connected to the TX of the ph EZO!) the UART protocol requires the crossing of RX and TX
  Tx EZO - 11 (connected to the TX of the ph EZO!)

*/
#include <SoftwareSerial.h>

// Definitions for pH sensor
#define rx_pH 10 // on port 3 
#define tx_pH 11 // on port 4 
SoftwareSerial ph_serial(rx_pH, tx_pH); // creates serial connection between pH sensor and Arduino

// GLOBAL VARIABLES
String ph_string=""; 
bool ph_string_complete=false;

// INTERRUPTS managing the reception form pH ezo and the usb 
void serialEvent() {
  // This function will be called when data is available on the PC serial port (interrupt triggered)
  while (Serial.available() > 0) {
    char incomingByte = (char)Serial.read(); // Read the incoming byte
    ph_serial.write(incomingByte); // Forward the data to the pH sensor
  }
  ph_serial.write('\r');
}

//------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------

void setup() {
  Serial.begin(9600); //open serial connection with computer's USB port
  Serial.print("\n Hello! I'm setting up, wait a moment please \n");

  ph_serial.begin(9600); //open serial connection with pH sensor
  ph_serial.write('C,0');
}

void loop() {

  if(ph_serial.available()>0){
    char inchar = (char)ph_serial.read();         //get the char we just received
    ph_string += inchar;                          //add the char to the var called sensorstring
    if (inchar == '\r') {                         //if the incoming character is a <CR>
      ph_string_complete = true;                  //set the flag
  }
    if(ph_string_complete==true){
      Serial.print("Arduino received char from sensor\n");
      Serial.print("value received : ");
      Serial.println(ph_string);   
      ph_string=""; 
      ph_string_complete=false; //lower the flag to 0
    }
  }
}
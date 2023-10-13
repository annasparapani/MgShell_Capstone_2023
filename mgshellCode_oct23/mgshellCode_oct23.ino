/* FILE: MGSHELL INCUBATOR CODE
  AUTHOR: Anna Sparapani, Gianluca De Francheschi
  DATE: 10-23

  TO DO: 
  [ ] add the SD saving part (take from incubator_redone_2.0.ino)
  [ ] add the CO2 solenoid control (take from incubator_redone_2.0.ino)
      build a function to keep the main cleaner than in take from incubator_redone_2.0.ino
  [ ] if hardware avaiable, write code for displaying measures on the LCD display (currently
      displaying on serial monitor but needs a computer connected)
  [ ] change the forwarding in SerialEvent() so that the characters coming from the USB serial (PC)
      are forwared to the ph_serial only if the first character is not an 'A' (because when it's an 'A')
      we are using the new added protocol and switching on / off the automatic reading instead of 
      communicating with the pH EZO 

  **************  CODE DESCRITPION **********************
  This is an edited code, which improves the code developed within the Capstone Projects (@PoliMi) for MgShell.
  MgShell provided its previous version which did not include pH sensing, the code developed during the project 
  and submitted in June 2023 added the pH sensing and the current code (October 2023) is an update which improves
  the communication between the PC and the pH sensor, and introduces: 
  1. communication with the pH sensor (pH EZO by Atlas Scientific) with forwarding of eveything
     that is sent to Arduino through the PC (USB) serial port to the pH sensor. In this way the code allows to 
     communicate with the pH sensor following its protocol (defined in the pH EZO board datasheet). 
     >> calibration can be performed with this code, single readings be taken when needed
  2. switching on and off the automatic measure every "interval"ms and setting of the measure interval. 
     this is a custom protocol that'a been added to the code to allow more interaction. It includes: 
     'A,0' switches off the automatic reading
     'A,1' switches on the automatic reading
     'A,2,ms' switches on the automatic reading and sets the "interval" to a duration of "ms" milliseconds
      > be careful not to set a too short reading interval, the printing on the serial monitor won't keep up otherwise.

  ****** PIN CONNECTIONS **********
  Arduino MEGA2560 Pin Connections:

  Pin 12,13: Tx/Rx from CO2 Sensor
  Pin #2: Temperature Sensors (T and humidity)
  Pin #8: Solenoid Relay    
  --- for pH sensing WE ADD  
  pin #10: Rx from pH (connected to the TX of the ph EZO!) the UART protocol requires the crossing of RX and TX
  pin #11: Tx from pH (connected to the TX of the ph EZO!)

  --- for SD we add
  pin #50 for MISO
  pin #51 for MOSI 
  pin #52 for SCK
  pin #53 FOR CS

*/
#include <cozir.h> // management of CO2 sensor, which is a COZIR sensor 
#include <SoftwareSerial.h> // serial communication with LCD screen and pH sensor EVO board
#include <DHT.h> // management of DHT series sensors 
#include <DHT_U.h>
#include <LiquidCrystal_I2C.h> // manages communication with LCD crystal that uses I2C protocol
#include <SPI.h> // communication with the SD card
#include <SD.h> // SD card management

/* ----------------------------------------------
--------  PORTS DEFINITIONS  --------------------
-------------------------------------------------
*/
// Definitions for CO2 sensor
#define ONE_WIRE_BUS 2 
SoftwareSerial c_serial(12, 13); // creates serial connection between CO2 sensor and Arduino on ports 12 and 13
COZIR czr(&c_serial);

// Defintion for DHT sensor (humidity and temperature)
DHT dht(2, DHT11); // on port 2 

// Definitions for pH sensor
#define rx_pH 10 // on port 3 
#define tx_pH 11 // on port 4 
SoftwareSerial ph_serial(rx_pH, tx_pH); // creates serial connection between pH sensor and Arduino

// Definition of solenoid port and on times
#define solenoid 8 
#define solenoid_ON_time_2 5000
#define TWO_HOURS 20*60*60000

/* ----------------------------------------------
--------  GLOBAL VARIABLES DEFINITIONS ----------
-------------------------------------------------
*/
String USB_string="";
// pH monitoring 
String ph_string=""; 
bool ph_string_complete=false;
float pH; 
bool pHready_flag=0; 
// CO2 control 
float threshold = 4.90; 
float CO2Threshold = 0.60; // threshold used to move to the stepping control of the solenoid
float multiplier = 0.001; // conversion in ppm
int count=0; 

// CO2, temperature and humidity variables
float c=0; 
float t=0; 
int h=0; 

File datafile; 

// Time management for automatic measures 
bool automatic_measure=1; // automatic measure can be switched on and off
unsigned long previousReadingTime = 0;
unsigned long readingInterval = 10000;  // how often are automatic measures, 
                                              //1800000 = 30 minutes interval (in milliseconds)

/* ----------------------------------------------
--------  SERIAL CONNECTION FUNCTIONS -----------
-------------------------------------------------
*/

// Serial Event coming from USB (port 1)
void serialEvent() {
  while (Serial.available() > 0) {
    char incomingByte = (char)Serial.read(); // Read the incoming byte
    ph_serial.write(incomingByte);
    USB_string+=incomingByte;
  }
  // add automatic measure switching on and off and control of reading interval
  if(USB_string[0]=='A'){
    USB_communication(); 
  }
  ph_serial.write('\r');
  USB_string="";
}

// Serial Event coming from phEZO (port 10)
void ph_serialEvent(){
  char inchar = (char)ph_serial.read();         //get the char we just received
    ph_string += inchar;                          //add the char to the var called sensorstring
    if (inchar == '\r') {                         //if the incoming character is a <CR>
      ph_string_complete = true;                  //set the flag
    }
    if(ph_string_complete==true){
      Serial.print("Arduino received from pH sensor: ");
      Serial.println(ph_string);   
      if(isdigit(ph_string[0])){
        pH = ph_string.toFloat(); 
        pHready_flag=1; 
      }
      ph_string=""; 
      ph_string_complete=false; //lower the flag to 0
    }
}

void USB_communication(){
  if (USB_string.length() >= 3) {
    char command = USB_string[2];
    switch (command) {
      case '0':
        automatic_measure=0; 
        Serial.print((String)"Automatic measuring every "+readingInterval+" ms switched off\n"); 
        break;
      case '1': 
        automatic_measure=1; 
        Serial.print((String)"Automatic measuring every "+readingInterval+" ms switched on\n"); 
        break;
      case'2':
        automatic_measure=1;
        String intervalString = USB_string.substring(4);
        readingInterval = intervalString.toInt();
        Serial.print("Reading interval set to " + String(readingInterval) + " ms\n");
        break;

  }} else Serial.print("Command not recognized\n");
}

/* ----------------------------------------------
--------  READING FROM SENSORS FUNCTIONS --------
-------------------------------------------------
*/

void read_CO2(){
  c=czr.CO2()*multiplier; // multiplier to convert in ppm
}
void read_humidity(){
  h=dht.readHumidity();
}
void read_temperature(){
  t=dht.readTemperature(); 
}
void read_pH(){
  ph_serial.write("R"); // tell the phEZO to take a single reading
  ph_serial.write("\r"); // add terminator character needed by phEZO communication protocol
}

void print_measures_USB(){ //prints the values obtained from the sensors on the USB serial
  Serial.print("  CO2  | Temp | Humid | pH \n");        
  Serial.println((String)" "+c+"  | "+t+" |  "+h+"    | "+pH+" "); 
}


//-------------------------------------------------------------------
//---------------------- MAIN ---------------------------------------
//-------------------------------------------------------------------

void setup() {
  Serial.begin(9600); //open serial connection with computer's USB port
  Serial.print("\n Hello! I'm setting up, wait a moment please \n");

  ph_serial.begin(9600); //open serial connection with pH sensor
  ph_serial.write("C,0\r");
  
  automatic_measure=1; //switch on the automatic measure option
}

void loop() {

  // always check if pH EZO has sent something (this would be better done with an interrupt
  // but unfortunately they don't seem to work well for serial connection in Arduino)
  if(ph_serial.available()>0){
    ph_serialEvent();
  }

  // Time management for automatic measure and saving every "readingInterval"
  long currentMillis=millis(); 
  if((currentMillis - previousReadingTime >= readingInterval)&&automatic_measure){
    Serial.print("Time to make a measurement, let's wake up the sensors!\n ");
    previousReadingTime=currentMillis; 
    read_CO2(); 
    read_humidity(); 
    read_temperature(); 
    read_pH(); 
  }
  if (pHready_flag){ //pH value has been received and saved in the variable pH
     pHready_flag=0; 
     print_measures_USB(); 
  }
}
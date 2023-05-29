/* FILE: MgShell Incubator - Arduino control code
   AUTHOR: Anna 
   DATE: 04-23
   This is an edited code by L. Bontempi, G. Noya, G. De Franceschi, M.Marras C. Russo and A. Sparapani 
   within the scope of the Capstone Projects (PoliMi, 2023, professor Jacchetti). 
   We were asked by MgShell to include a pH sensor in the current incubator setup, we are adding a
   pH microprobe by Atlas Scientific, and reading it through Arduino. Our code simply adds the reading 
   of this latter sensor to the LCD display

  ***************** CODE DESCRIPTION ********************
  This is the code used for the pH Atlas Scientific functional testing. It takes a pH measure every 30 
  minutes and saves the value in the SD memory card

  ************ PIN CONNECTIONS ***************************
  Arduino MEGA2560 Pin Connections:
  NB switching to Arduino UNO might require to change some pins connections 
  
  ---- for pH sensing WE ADD  
   pin #10: Rx from pH Ezo Board
   pin #11: Tx from pH Ezo Board
 
  --- for SD we add
  pin# 50 for MISO
  pin# 51 for MOSI 
  pin# 52 for SCK
  pin# 53 FOR CS
*/

//************ LIBREARIES AND DEFINITIONS ******************
#include <SoftwareSerial.h> // serial communication with LCD screen and pH sensor EVO board
#include <LiquidCrystal_I2C.h> // manages communication with LCD crystal that uses I2C protocol
#include <SD.h>

// Definitions for pH sensor
#define rx_pH 10 // on port 10 rx from pH 
#define tx_pH 11 // on port 11 tx from pH 
SoftwareSerial ph_serial(rx_pH, tx_pH); // creates serial connection between pH sensor and Arduino


// Definitions for LCD display 
LiquidCrystal_I2C lcd(0x27, 16, 2); // creates the display variable with its dimensions

// *************** GLOBAL VARIABLES ****************
int count=0; 
String input_string_pH = "";
String sensor_string_pH = "";
volatile boolean input_string_pH_complete=false; 
boolean sensor_string_pH_complete=false; 
char inchar="Z"; 
float pH; 
int sec_on=0;
int min_on=0;
int hr_on=0; 
bool measure_flag=0; 
File datafile;

//*********** FUNCTIONS DEFINITIONS *****************

void read_pH(){
    pH = sensor_string_pH.toFloat(); 
    measure_flag=0;
    Serial.print("pH value saved: ");
    Serial.print(pH); 
    Serial.print("\n");
    Serial.print("pH values read and flag lowered\n");
    sensor_string_pH="";
}

void serialEvent() { // COMMUNICATION with PC Serial
  input_string_pH = Serial.readStringUntil(13);           //read the string until we see a <CR>
  input_string_pH_complete = true;                       //set the flag used to tell if we have received a completed string from the PC
  Serial.print("Received character from pc serial, will be sent to sensor:  ");
  Serial.print(input_string_pH);
  Serial.print("\n");
}

void print_measures(){
  Serial.print("pH: ");
  Serial.print(pH);
  Serial.print("\n");       
}

void save_measures(){
  datafile=SD.open("pH.txt",FILE_WRITE);            
  if (datafile) {
    datafile.println(pH);
    datafile.close();
  }
}

//********** SETUP AND LOOP **************
void setup(){
  // SERIAL CONNECTIONS INITIALIZATION
  Serial.begin(9600);
  delay(1000);
  Serial.print("Hello! I'm setting up, wait a moment please\n "); 
  ph_serial.begin(9600);// open serial connection with pH sensor, baudrate = 9600
  input_string_pH.reserve(10); // reserves some memory for the string, this prevents errors 
  sensor_string_pH.reserve(30);
  //attachInterrupt(digitalPinToInterrupt(10), pH_rx_serialInterrupt, FALLING); // attach interrupt to the RX coming from the sensor

  // pH SENSOR INITIALIZATION
  ph_serial.print("C,0\r");// turn off continuos measuring of the sensor 
   
  // SD INITIALIZATION
  //test if wiring is correct
  if (!SD.begin(53)) {
    Serial.println("SD card initialization failed (There may be no card inserted or wrong connections)");
    while (1);
  } else{ 
    Serial.println("Initialization ok \n");
    }
  Serial.print("I have finished setting up! ready to go \n ");

  delay(5000); // leave some time before getting into the code, so that the user can read info on the LCD
}

void loop() {
  // put your main code here, to run repeatedly:

  // ***************** TIME ON EVALUATION *****************
  long filling_time=millis(); // returns the number of milliseconds since the arduino board was powered or reset, 32-bit unsigned int (enough for our times)
  sec_on++;  
  if(sec_on >= 60){  // time variables to keep track of for how long we've been on 
    min_on++;
    sec_on=0;
    // every minute prints how long it's been on for
    Serial.print((String) "Hello! I have been on for: "+(int)hr_on+" hr "+(int)min_on+" min \n");
    if(min_on==60){
      hr_on++;
      min_on=0; 
    }
  }
  
  //******************** SENSORS READINGS & SAVINGS *****************
  // SENDING FROM ARDUINO TO pH EZO BOARD
  if (input_string_pH_complete == true) {              //if a string from the PC has been received in its entirety
      ph_serial.print(input_string_pH);                //send that string to the Atlas Scientific product
      if(input_string_pH[0]=='R' || input_string_pH[0]=='r'){
        measure_flag=1; 
      }
      ph_serial.print('\r');                           //add a <CR> to the end of the string
      input_string_pH = "";                            //clear the string
      input_string_pH_complete = false;                //reset the flag used to tell if we have received a completed string from the PC
      Serial.print("Arduino sent character to pH EZO, measure flag: ");
      Serial.print(measure_flag);
      Serial.print("\n");
  }

  // ARDUINO RECEIVE DATA FROM pH EZO
  if (ph_serial.available() > 0) {                     //if we see that the Atlas Scientific product has sent a character
    char inchar = (char)ph_serial.read();              //get the char we just received
    sensor_string_pH += inchar;                       //add the char to the var called sensorstring
    if (inchar == '\r') {                             //if the incoming character is a <CR>
      sensor_string_pH_complete = true;               //set the flag
    }
  }

  if (sensor_string_pH_complete) {
    Serial.print("Received data from pH EZO: ");
    Serial.print(sensor_string_pH);
    Serial.print("\n");
    sensor_string_pH_complete=0; 
    if(measure_flag && (isdigit(sensor_string_pH[0]))){
      read_pH(); 
    } else{
      sensor_string_pH="";
    }

  }

  if ( min_on == 30 || min_on == 60 ){ // every 30 minutes does an automatic measure and saves the values
    measure_flag=1; 
    ph_serial.print('R\r');   //tell sensor to take a single measurement
    delay(3000);
    save_measures();
    print_measures(); 
  }
  delay(1000); 
}
/* FILE: MGSHELL INCUBATOR CODE
  AUTHOR: Anna Sparapani, Gianluca De Francheschi
  DATE: 10-23
  ********************************************************
  TO DO: 
  [ ] change the forwarding in SerialEvent() so that the characters coming from the USB serial (PC)
      are forwared to the ph_serial only if the first character is not an 'A' (because when it's an 'A')
      we are using the new added protocol and switching on / off the automatic reading instead of 
      communicating with the pH EZO 

  **************  CODE DESCRITPION **********************
  This is an edited code, which improves the code developed within the Capstone Projects (@PoliMi) for MgShell.
  MgShell provided its previous version which did not include pH sensing, the code developed during the project 
  and submitted in June 2023 added the pH sensing. While the current code (October 2023) is an update which improves
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
#include <TimeLib.h>   // Include the TimeLib library if you're using an RTC module

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

// Definitions for LCD display 
LiquidCrystal_I2C lcd(0x27, 16, 2); // creates the display variable with its dimensions


// Definitions for pH sensor
#define rx_pH 10 // on port 3 
#define tx_pH 11 // on port 4 
SoftwareSerial ph_serial(rx_pH, tx_pH); // creates serial connection between pH sensor and Arduino

// Definition of LEDs ports 
#define led_red 26
#define led_green 28

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
boolean saving_pH=false; // allow not to perform double savings of other values when we want to save the pH
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
bool measure_flag=0;

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
        measure_flag=1;
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
--------------------  LCD AND SD ----------------
-------------------------------------------------
*/

void LCD_setup(){
  // displays starting information on the LCD display
  lcd.backlight();
  delay(500);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("SOGLIA CO2: ");
  lcd.print(threshold);
  lcd.setCursor(0, 1);
  lcd.print("THRESHOLD:");
  lcd.print(CO2Threshold);
  delay(2000);
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("INIZIO");
  delay(1000);
  lcd.clear();
}

void save_measures(){
  Serial.print("Saving measures on SD\n");

  // TAKE A TIME STAMP //
  time_t currentT = now(); // Get the current time
  tmElements_t tm; // Create a tmElements_t struct to hold the date and time
  breakTime(currentT, tm); // Break down the time_t value into its components
  String currentDate = String(tm.Day)+ "/" + tm.Month + "/" + tm.Year + 2023;
  String currentTime = String(tm.Hour) + ":" + tm.Minute + ":" + tm.Second;
  String dataString = currentDate + "; " + currentTime + "; " ;

  // SAVE DATA INTO THE PROPER FILES //
 datafile = SD.open("CO2.txt",FILE_WRITE);
    if (datafile && !saving_pH ) {
      datafile.println(dataString + String(c));
      datafile.close();
    }
  datafile = SD.open("Humidity.txt",FILE_WRITE);
    if (datafile && !saving_pH ) {
      datafile.println(dataString + String(h));
      datafile.close();
    }
  datafile = SD.open("Temperature.txt",FILE_WRITE);
    if (datafile && !saving_pH ) {
      datafile.println(dataString + String(t));
      datafile.close();
    }
  datafile = SD.open("pH.txt",FILE_WRITE);           
  if (datafile) {
    if (measure_flag && (isdigit(ph_string[0]))){
      datafile.println(dataString + String(pH));
      saving_pH=false;
    }
    datafile.close();
  }
}




/* ----------------------------------------------
--------  READING FROM SENSORS FUNCTIONS --------
-------------------------------------------------
*/

void read_CO2(){
  c = czr.CO2()*multiplier;
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("CO2:");
  lcd.setCursor(5, 0); 
  lcd.print(c);
  lcd.print("%");
  lcd.setCursor(11, 0); 
  lcd.print(threshold);
  lcd.print("%");  
}

void read_humidity(){
  h = dht.readHumidity();
  lcd.setCursor(10, 1);
  lcd.print("U:"); 
  lcd.setCursor(12, 1);  
  lcd.print(h);
  lcd.print("%");
  // don't know if these can stay here 
  digitalWrite(led_red,LOW);
  digitalWrite(led_green,LOW);
  digitalWrite(solenoid, LOW);
}

void read_temperature(){
  t = dht.readTemperature(); 
  lcd.setCursor(0, 1);
  lcd.print("T:"); 
  lcd.setCursor(2, 1);  
  lcd.print(t);
  lcd.print(" C");
}

void read_pH(){
  ph_serial.write("R"); // tell the phEZO to take a single reading
  ph_serial.write("\r"); // add terminator character needed by phEZO communication protocol
  saving_pH = true;
  measure_flag=0;
}

void print_measures_USB(){ //prints the values obtained from the sensors on the USB serial
  Serial.print("  CO2  | Temp | Humid | pH \n");        
  Serial.println((String)" "+c+"  | "+t+" |  "+h+"    | "+pH+" "); 
}



//-------------------------------------------------------------------
//---------------------- MAIN ---------------------------------------
//-------------------------------------------------------------------

void setup() {

// SERIAL CONNECTIONS INITIALIZATION
  Serial.begin(9600); //open serial connection with computer's USB port
  Serial.print("\n Hello! I'm setting up, wait a moment please \n");

  c_serial.begin(9600); // open serial connection with CO2 sensor, baudrate = 9600

  ph_serial.begin(9600); //open serial connection with pH sensor
  ph_serial.write("C,0\r");
  
  automatic_measure=1; //switch on the automatic measure option

// SENSORS INITALIZATION
  czr.init();
  dht.begin();
  
  pinMode(led_red, OUTPUT);     // LED PINS INITIALIZATION
  pinMode(led_green, OUTPUT);
  pinMode(solenoid, OUTPUT);    // SOLENOID PIN INITIALZIATION

// LCD INITIALIZATION 
  lcd.init();
  LCD_setup();

// SD INITIALIZATION
   Serial.begin(9600);

  Serial.print("SD Card init... \n");
  //test if wiring is correct
  if (!SD.begin(53)) {
    Serial.println("init failed.. (there may be no card inserted or wrong connections)");
    while (1);
  }
  Serial.println("init ok");

  Serial.print("I have finished setting up! ready to go \n ");

  delay(5000); // leave some time before getting into the code, so that the user can read info on the LCD

    // put your setup code here, to run once:
 


}

void loop() {

  digitalWrite(10,HIGH); 
  delay(500);
  digitalWrite(10, LOW); 

  // put your main code here, to run repeatedly:
  long filling_time=millis(); // returns the number of milliseconds since the arduino board was powered or reset, 32-bit unsigned int (enough for our times)

  /*Serial.print("Hello! I have been on for: "); 
  Serial.print(filling_time); 
  Serial.print("ms \n"); 
  */

  // always check if pH EZO has sent something (this would be better done with an interrupt
  // but unfortunately they don't seem to work well for serial connection in Arduino)
  if(ph_serial.available()>0){
    ph_serialEvent();
  }
  
//******************** SENSORS READINGS & SAVINGS *****************

  // Time management for automatic measure and saving every "readingInterval"
  long currentMillis=millis(); 
  if((currentMillis - previousReadingTime >= readingInterval)&&automatic_measure){
    Serial.print("Time to make a measurement, let's wake up the sensors!\n ");
    previousReadingTime=currentMillis; 
    read_CO2(); 
    read_humidity(); 
    read_temperature(); 
    read_pH(); 
    save_measures();

  }
  if (pHready_flag){ //pH value has been received and saved in the variable pH
     pHready_flag=0; 
     print_measures_USB(); 
  }


/************************ CO2 CONTROL ***************************************
  CO2 levels are controlled by a solenoid, which when open lets more 
  CO2 in, and when closed blocks the input. This is controlled by the following 
  NESTED if series -> could be improved with a PID Controller */

  if(c!=0){
    // ******** Initial Filling in ******** 
    //  -> until the CO2 level (c) is below a certain threshold we keep the solenoid open
    // LED red on, LED green off 
    if(c<threshold && c<CO2Threshold*threshold){
      digitalWrite(led_red, HIGH); // switch on red led
      digitalWrite(solenoid, HIGH);  
      digitalWrite(led_green,LOW);

    } else if (c<threshold && c>=CO2Threshold*threshold){ // ***** Reached the thres. value ****
    // -> switch off the solenoid for the interval "Solenoid on Time 2" = 5000 ms, then back on
    // LED ren on, LED grenn off (?)  
      digitalWrite(solenoid, LOW); 
      delay(solenoid_ON_time_2); 
      digitalWrite(solenoid, HIGH); 
      digitalWrite(led_red, HIGH); 
    } else if (c>=threshold){ // ***** Breached over the threshold ***** 
    // -> swtch the solenoid off 
      digitalWrite(solenoid, LOW); 
    }
    if(filling_time>=TWO_HOURS && digitalRead(solenoid)==HIGH){
      digitalWrite(solenoid, LOW); // after two hours clos the solenoid, no matter what the c value is
    }
  } else if (c==0 && digitalRead(solenoid)==HIGH){
    count++; 
    Serial.print("zeri = ");
    Serial.println(count); 
  }

  delay(1000); 




}
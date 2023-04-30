/* FILE: MgShell Incubator - Arduino control code
   AUTHOR: Anna 
   DATE: 04-23
   This is an edited code by L. Bontempi, G. Noya, G. De Franceschi, M.Marras C. Russo and A. Sparapani 
   within the scope of the Capstone Projects (PoliMi, 2023, professor Jacchetti). 
   We were asked by MgShell to include a pH sensor in the current incubator setup, we are adding a
   pH microprobe by Atlas Scientific, and reading it through Arduino. Our code simply adds the reading 
   of this latter sensor to the LCD display

  ***************** CODE DESCRIPTION ********************
  This Arduino code is for controlling an incubator with controlled CO2 and temperature. 
  The code initializes various libraries and defines global variables such as the threshold 
  for CO2, the solenoid relay pin, and the pins for the keypad. In the setup function, the 
  code initializes various sensors and sets up the LCD display. In the loop function, the code 
  reads data from the CO2 sensor, DHT11 temperature and humidity sensor, and updates the LCD 
  display with the readings. The code also checks if the CO2 level is below a certain threshold 
  and turns on the solenoid relay to add more CO2 if necessary. The code also uses a keypad to
  update the CO2 threshold and other settings. The code also includes a timer to keep track of 
  the time since the Arduino was turned on.

  ************ PIN CONNECTIONS ***************************
  Arduino MEGA2560 Pin Connections:

  Pin 12,13: Tx/Rx from CO2 Sensor
  Pin #2: Temperature Sensors (T and humidity)
  Pin #8: Solenoid Relay    
  Pin # 20 e 21 SCL e  SCA 
  Pin # 26 :led rosso
  Pin # 28 :led verde
  Pin [ROWS] # 33, 35, 37,39 : connect to the row pinouts of the keypad
  Pin [COLS] # 41, 43, 45 : connect to the column pinouts of the keypad
  rowPins_m[4] # 33, 35, 37,39 : connect to the row pinouts of the keypad
  colPins_m[1] # 47: connect to the column pinouts of the keypad
  ---- for pH sensing WE ADD  
 pin #3: Rx from pH 
 pin #4: Tx from pH 
*/

//************ LIBREARIES AND DEFINITIONS ******************
#include <cozir.h> // management of CO2 sensor, which is a COZIR sensor 
#include <SoftwareSerial.h> // serial communication with LCD screen and pH sensor EVO board
#include <DHT.h> // management of DHT series sensors 
#include <DHT_U.h>
#include <LiquidCrystal_I2C.h> // manages communication with LCD crystal that uses I2C protocol
#include <SPI.h>
#include <SD.h>

// Definitions for CO2 sensor
#define ONE_WIRE_BUS 2 
SoftwareSerial c_serial(12, 13); // creates serial connection between CO2 sensor and Arduino 
                                 // on ports 12 and 13
COZIR czr(&c_serial);

// Definitions for pH sensor
#define rx_pH 3 // on port 3 
#define tx_pH 4 // on port 4 
SoftwareSerial ph_serial(rx_pH, tx_pH); // creates serial connection between pH sensor and Arduino

// Degintion for DHT sensor (humidity and temperature)
DHT dht(2, DHT11); // on port 2 

// Definitions for LCD display 
LiquidCrystal_I2C lcd(0x27, 16, 2); // creates the display variable with its dimensions

// Definition of LEDs ports 
#define led_red 26
#define led_green 28

// Definition of solenoid port and on times
#define solenoid 8

// ********** DEFINES **********
#define solenoid_ON_time_2 5000
#define TWO_HOURS 20*60*60000

// *************** GLOBAL VARIABLES ****************
float threshold = 4.90; 
float CO2Threshold = 0.60; // threshold used to move to the stepping control of the solenoid
float multiplier = 0.001; // conversion in ppm
int count=0; 
String input_string_pH = "";
String sensor_string_pH = "";
boolean input_string_pH_complete=false; 
boolean sensor_string_pH_complete=false; 
char inchar="Z"; 
float c=0; 
float t=0; 
int h=0; 
float pH; 

// Skipping the definition of the buttons and keys -> not used in the code

//*********** FUNCTIONS DEFINITIONS *****************
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

void serialEvent_pH(){
  input_string_pH = Serial.readStringUntil(13); 
  input_string_pH_complete=true; 
}


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

  datafile=SD.open("CO2.txt",FILE_WRITE)
    if (datafile) {
      datafile.println(c);
      datafile.close();
    }
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

    datafile=SD.open("Humidity.txt",FILE_WRITE)
    if (datafile) {
      datafile.println(h);
      datafile.close();
    }
}

void read_temperature(){
  t = dht.readTemperature(); 
  lcd.setCursor(0, 1);
  lcd.print("T:"); 
  lcd.setCursor(2, 1);  
  lcd.print(t);
  lcd.print(" C");
    datafile=SD.open("Temperature.txt",FILE_WRITE)
    if (datafile) {
      datafile.println(t);
      datafile.close();
    }
}


void read_pH(){
  if (ph_serial.available() > 0) {                //if we see that the Atlas Scientific product has sent a character
    /* since we have a delay in the main loop, we cannot rely on the original code version
    provided by Atlas scientfic that makes one read of the char at each iteration. We
    open a loop that reads all the characters until the terminator character is received, 
    while reading we save the characters in the string (sensorstring) and then raise a flag
    when the terminator is received */
    do{
      char inchar = (char)ph_serial.read();
      sensor_string_pH += inchar; 
      delay(2); // 2 ms delay so that the next character has time to come in
      } while (inchar !='\r');                                 
      sensor_string_pH_complete = true;             
  }

  if (sensor_string_pH_complete == true) {               //if a string from the Atlas Scientific product has been received in its entirety
      Serial.println(sensor_string_pH);                   //send that string to the PC's serial monitor
      if (isdigit(sensor_string_pH[0])) {                 //if the first character in the string is a digit, we convert it to a float 
        
        pH = sensor_string_pH.toFloat(); 
        lcd.setCursor(0, 2);
        lcd.print("pH:"); 
        lcd.setCursor(5, 2);  
        lcd.print(pH);
        
        datafile=SD.open("pH.txt",FILE_WRITE)             // if the string is recieved we also save it in the SD
        if (datafile) {
          datafile.println(pH);
          datafile.close();
        }        
      }

  }
}

void print_measures(){
  Serial.print("  CO2  | Temp | Humid | pH \n");        
  Serial.println((String)" "+c+"  | "+t+" |  "+h+"    | "+pH+" "); 


  /*Serial.print(" Temp= ");
  Serial.print(t);

  Serial.print(" Umid= ");
  Serial.println(h);

  Serial.print(" pH= ");
  Serial.println(pH);*/
}

//********** SETUP AND LOOP **************
void setup() {
  // put your setup code here, to run once:
  // SERIAL CONNECTIONS INITIALIZATION
  Serial.begin(9600);
  Serial.print("Hello! I'm setting up, wait a moment please\n "); 
  c_serial.begin(9600); // open serial connection with CO2 sensor, baudrate = 9600
  ph_serial.begin(9600);// open serial connection with pH sensor, baudrate = 9600
  input_string_pH.reserve(10); // reserves some memory for the string, this prevents errors 
  sensor_string_pH.reserve(30);

  // SENSORS INITALIZATION
  czr.init();
  dht.begin();
  
  pinMode(led_red, OUTPUT);     // LED PINS INITIALIZATION
  pinMode(led_green, OUTPUT);
  pinMode(solenoid, OUTPUT);    // SOLENOID PIN INITIALZIATION

  // LCD INITIALIZATION 
  lcd.init();
  LCD_setup(); 

  Serial.print("I have finished setting up! ready to go \n ");

  delay(5000); // leave some time before getting into the code, so that the user can read info on the LCD
}

void loop() {
  digitalWrite(10,HIGH); 
  delay(500);
  digitalWrite(10, LOW); 

  // put your main code here, to run repeatedly:
  long filling_time=millis(); // returns the number of milliseconds since the arduino board was powered or reset, 32-bit unsigned int (enough for our times)

  Serial.print("Hello! I have been on for: "); 
  Serial.print(filling_time); 
  Serial.print("ms \n"); 
  //******************** SENSORS READINGS & SAVINGS *****************
  read_CO2();
  read_humidity(); 
  read_temperature(); 
  read_pH(); 
  print_measures(); 

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

  // aggiungere: lettura altri caratteri da pH EZO 
}

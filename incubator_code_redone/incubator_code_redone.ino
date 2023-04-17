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
  Pin #2: Temperature Sensors
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

// Definitions for CO2 sensor
SoftwareSerial c_serial(12, 13); // creates serial connection between CO2 sensor and Arduino 
COZIR c_serial(&sws);

// Definitions for pH sensor
#define rx_pH 3
#define tx_pH 4
SoftwareSerial ph_serial(rx_pH, tx_pH); // creates serial connection between pH sensor and Arduino

// Degintion for DHT sensor (humidity and temperature)
DHT dht(2, DHT11);

// Definitions for LCD display 
#include <LiquidCrystal_I2C.h> // manages communication with LCD crystal that uses I2C protocol
LiquidCrystal_I2C lcd(0x27, 16, 2); // creates the display variable with its dimensions

// *************** GLOBAL VARIABLES ****************
float threshold_CO2 = 4.90; 
float CO2Threshold = 0.60; // threshold used to move to the stepping control of the solenoid
float multiplies = 0.001; // conversion in ppm
int solenoid = 8;
int two_hours=20*60*60000; // was float originally, check if it works this way 
String input_string_pH = "";
String sensor_string_pH = "";
boolean input_string_pH_complete=false; 
booleas sensor_string_pH_complete=false; 
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
  lcd.print(soglia);
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
  inputstring_pH = Serial.readStringUntil(13); 
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
  lcd.print(soglia);
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
  digitalWrite(pin_r,LOW);
  digitalWrite(pin_v,LOW);
  digitalWrite(solenoide, LOW);
}

void read_T(){
  t = dht.readTemperature(); 
  lcd.setCursor(0, 1);
  lcd.print("T:"); 
  lcd.setCursor(2, 1);  
  lcd.print(t);
  lcd.print(" C");
}


void read_pH(){
  if (myserial.available() > 0) {                //if we see that the Atlas Scientific product has sent a character
      char inchar = (char)myserial.read();       //get the char we just received
      sensorstring += inchar;                    //add the char to the var called sensorstring
      if (inchar == '\r') {                      //if the incoming character is a <CR>
      sensor_string_complete = true;             //set the flag
      }
  }

  if (sensor_string_complete == true) {               //if a string from the Atlas Scientific product has been received in its entirety
      Serial.println(sensorstring);                   //send that string to the PC's serial monitor
      if (isdigit(sensorstring[0])) {                 //if the first character in the string is a digit, we convert it to a float 
      pH = sensorstring.toFloat(); 
      lcd.setCursor(0, 2);
      lcd.print("pH:"); 
      lcd.setCursor(5, 2);  
      lcd.print(pH);
    }
  }
}

//********** SETUP AND LOOP **************
void setup() {
  // put your setup code here, to run once:
  // SERIAL CONNECTIONS INITIALIZATION
  Serial.begin(9600); 
  c_serial.begin(9600); // open serial connection with CO2 sensor, baudrate = 9600
  pH_serial.begin(9600);// open serial connection with pH sensor, baudrate = 9600
  input_string_pH.reserve(10); // reserves some memory for the string, this prevents errors 
  sensor_string_pH.reserve(30);

  // SENSORS INITALIZATION
  czr.init();
  dht.begin();

  // SOLENOID PINS INITIALZIATION
  pinMode(pin_r, OUTPUT);
  pinMode(pin_v, OUTPUT);
  pinMode(solenoide, OUTPUT);

  // LCD INITIALIZATION 
  lcd.init();
  LCD_setup(); 

  delay(5000); // leave some time before getting into the code, so that the user can read info on the LCD
}

void loop() {
  // put your main code here, to run repeatedly:
  filling_time=millis(); // returns the number of milliseconds since the arduino board was powered or reset, 32-bit unsigned int (enough for our times)

  read_CO2();
  read_humidity(); 
  read_temperature(); 
  read_pH(); 

  // aggiungere : check on CO2, lettura altri caratteri da pH EZO 





}

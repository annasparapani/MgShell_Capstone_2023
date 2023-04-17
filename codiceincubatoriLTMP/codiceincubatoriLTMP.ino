//  FILE: Codice Incubatore: CO2 e  Temperatura controllata
//  AUTHOR: Federica 
// 
//  DATE: 07-22
/* This is an edited code by L. Bontempi, G. Noya, G. De Franceschi, M.Marras C. Russo and A. Sparapani 
   within the scope of the Capstone Projects (PoliMi, 2023, professor Jacchetti). 
   We were asked by MgShell to include a pH sensor in the current incubator setup, we are adding a
   pH microprobe by Atlas Scientific, and reading it through Arduino. Our code simply adds the reading 
   of this latter sensor to the LCD display
*/

/* ***************** CODE DESCRIPTION ********************

This Arduino code is for controlling an incubator with controlled CO2 and temperature. The code initializes various libraries and defines global 
variables such as the threshold for CO2, the solenoid relay pin, and the pins for the keypad. In the setup function, the code initializes various 
sensors and sets up the LCD display. In the loop function, the code reads data from the CO2 sensor, DHT11 temperature and humidity sensor, 
and updates the LCD display with the readings. The code also checks if the CO2 level is below a certain threshold and turns on the solenoid relay 
to add more CO2 if necessary. The code also uses a keypad to update the CO2 threshold and other settings. The code also includes a timer to keep 
track of the time since the Arduino was turned on.


*/
/*
*Arduino MEGA2560 Pin Connections:

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
---- WE ADD 

pin #3: Rx from pH 
pin #4: Tx from pH 

*/

// Librerie 

#include <cozir.h> // management of CO2 sensor, which is a COZIR sensor 
#include <SoftwareSerial.h> // serial communication with LCD screen and pH sensor EVO board
 
#define ONE_WIRE_BUS 2
// CO2 sensor 
SoftwareSerial sws(12, 13);
COZIR czr(&sws);

// pH sensor 
#define rx_pH 3
#define tx_pH 4
SoftwareSerial myserial(rx_pH, tx_pH); 


#include <LiquidCrystal_I2C.h> // manages communication with LCD crystal that uses I2C protocol (has lcd variable type already defined inside)
LiquidCrystal_I2C lcd(0x27, 16, 2); // NB LCD display communicates with Arduino with I2C protocol! 
#include <Keypad.h>

#include <DHT.h> // management of DHT series sensors 
#include <DHT_U.h>
DHT dht(2, DHT11);


/*GLOBAL VARIABLES:*/
float soglia = 4.90; // what is this referring to? 
float CO2Threshold = 0.60; // Threshold per passare al controllo stepping in % / 100
float multiplier = 0.001;// Converione ppm-->% (10/10000) (Hardware multiplier/conversione ppm).
int SolenoidOnTime = 1000;
int SolenoidOnTime2 = 5000;
int solenoide = 8;
int pin_r = 26;
int pin_v = 28;
float tempo;
int count;
float tempo_riempimento; //definisco la variabile che mi serve per contare il tempo dall'accensione di arduino

// rows and columns of the LCD display
const byte ROWS = 4; // numero di righe
const byte COLS = 3; // numero di colonne


char hexaKeys[ROWS][COLS] = { 
{'1','2','3'},
{'4','5','6'},
{'7','8','9'},
{'.','0','#'}
};

char hexamenu[4][1] = { 
{'A'},
{'B'},
{'C'},
{'D'}
};

byte rowPins[ROWS] = {33, 35, 37,39}; 
byte colPins[COLS] = {41, 43, 45}; 
byte rowPins_m[4] = {33, 35, 37,39}; 
byte colPins_m[1] = {47}; 


// Inizializzazione variabili della classe Keypad
Keypad customKeypad = Keypad( makeKeymap(hexamenu), rowPins_m, colPins_m, 4, 1); 
Keypad valoriKeypad = Keypad( makeKeymap(hexaKeys), rowPins, colPins, ROWS,COLS);
// Inizializzazione variabili char matrice Keypad
char menu = customKeypad.getKey();
char valori = valoriKeypad.getKey();



bool x; // VARIABILE BOOLEANA PER ATTIVARE LA CONDIZIONE CHE PERMETTE DI AGGIORNARE LA SOGLIA DI CO2.
bool streaming ;// USO UNA VARIABILE BOOLEANA PER ATTIVARE LA CONDIZIONE DI STERAMING DATI.
int i; //USO VARIALE INTERA COME CONTEGGIO
int soglia_n=0;
int CO2Threshold_n=0;
float dueore=2*60*60000; //limite di due ore da collegare al conteggio della funzione millis()
String intput_string_pH = "";
String sensor_string_pH = "";
boolean input_string_pH_complete=false; 
booleas sensor_string_pH_complete=false; 
float pH; 

void setup() {
  Serial.begin(9600); // baudrate a 9600
  myserial.begin(9600); // pH sensor begins with baudrate 9600
  sws.begin(9600); // CO2 sensor begins with baudrate 9600 
  inputstring.reserve(10);     //set aside some bytes for receiving data from the PC
  sensorstring.reserve(30);     //set aside some bytes for receiving data from Atlas Scientific product
  czr.init();
  lcd.init();
  lcd.backlight();
  dht.begin();
  pinMode(pin_r, OUTPUT);
  pinMode(pin_v, OUTPUT);
  pinMode(solenoide, OUTPUT);
  delay(1000);
  lcd.noAutoscroll();
  lcd.print("INCUBATORE CO2");
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
  // put your setup code here, to run once:

}

void serialEvent_pH(){
  inputstring_pH = Serial.readStringUntil(13); 
  input_string_pH_complete=true; 
}

void loop() {

  tempo_riempimento = millis(); //inizio il conteggio del tempo , returns the number of milliseconds since the arduino board was powered or reset, 32-bit unsigned int (enough for our times)
  
  /* QUESTION 
  it's doing the reading twise, once in this loop function, and 
  another time in the LCD function - is it necessary? can't we just pass the parameters
  to the LCD function? */ 

    float c = czr.CO2()*multiplier;
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print("CO2:");
          lcd.setCursor(5, 0); 
          lcd.print(c);
          lcd.print("%");
          lcd.setCursor(11, 0); 
          lcd.print(soglia);
          lcd.print("%"); 
                  
    //float t=sensors.getTempCByIndex(0);
    float t = dht.readTemperature(); 
              lcd.setCursor(0, 1);
              lcd.print("T:"); 
              lcd.setCursor(2, 1);  
              lcd.print(t);
              lcd.print(" C");

    int h = dht.readHumidity();
              lcd.setCursor(10, 1);
              lcd.print("U:"); 
              lcd.setCursor(12, 1);  
              lcd.print(h);
              lcd.print("%");

            digitalWrite(pin_r,LOW);
            digitalWrite(pin_v,LOW);
            digitalWrite(solenoide, LOW);
  
  /*****************************************
  *********** EZO pH board reading**********
  *******************************************/ 
  // **** pH value reading ****
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
  // **** reading of some other information coming from the EXO board ****
  if(input_string_pH_complete=true){
      myserial.print(input_string_pH); // print the pH value on the serial terminal (our computer)
      myserial.print('\r'); 
      input_string_pH=""; 
      input_string_pH_complete=false;
    }

    if (myserial.available() > 0) {           //if we see that the Atlas Scientific product has sent a character
    char inchar = (char)myserial.read();      //get the char we just received
    sensorstring_pH += inchar;                //add the char to the var called sensorstring
    if (inchar == '\r') {                     //if the incoming character is a <CR>
      sensor_string_complete_pH = true;       //set the flag
    }
  }

    
    if(c!=0){
      if (c < soglia && c < CO2Threshold*soglia) {   //*********  Riempimento iniziale - misuro la CO2  ***************//
          digitalWrite(pin_r,HIGH);                    
          digitalWrite(solenoide,HIGH);//aperto           
          digitalWrite(pin_v,LOW);  
          // digitalWrite(solenoide, LOW);                //*********   Apertura solo per 'SolenoidOnTime'   ***************//
          //delay(SolenoidOnTime);
          // digitalWrite(solenoide, HIGH);
          //digitalWrite(pin_r,HIGH);
      } 

      if (c < soglia && c >= CO2Threshold*soglia) {  //*********************    Soglia minore   ***********************//
            digitalWrite(solenoide, LOW);                //*********   Apertura solo per 'SolenoidOnTime'   ***************//
            delay(SolenoidOnTime2);
            digitalWrite(solenoide, HIGH);
            digitalWrite(pin_r,HIGH);
      } 

      if (c >= soglia){                            //*********************    Soglia maggiore   *********************//
             digitalWrite(pin_r,LOW); 
             digitalWrite(solenoide, LOW);//chiuso
             digitalWrite(pin_v,HIGH);
      }

      if (c!=0 && tempo_riempimento >= dueore && c <= 3.5) {
              digitalWrite(solenoide, LOW);//chiuso
      }
    }
    if (c==0 && digitalRead(solenoide)==HIGH) {
        count++;
        Serial.print(" zeri= ");
        Serial.println(count);
    }
    //delay(1000);// one reading each second -> in this way reading a serial UART 
    // becomes complicated, the delay can be moved to the update_LCD function 
    
    // put your main code here, to run repeatedly:  
  updateLCD();
}

// Funzione per aggiornare il display LCD
void updateLCD() {
          float c = czr.CO2()*multiplier;
                    lcd.clear();
                    lcd.setCursor(0, 0);
                    lcd.print("CO2:");
                    lcd.setCursor(5, 0); 
                    lcd.print(c);
                    lcd.print("%");
                    lcd.setCursor(11, 0); 
                    lcd.print(soglia);
                    lcd.print("%"); 
                            
          //float t=sensors.getTempCByIndex(0);
          float t = dht.readTemperature(); 
                    lcd.setCursor(0, 1);
                    lcd.print("T:"); 
                    lcd.setCursor(2, 1);  
                    lcd.print(t);
                    lcd.print(" C");

          int h = dht.readHumidity();
                    lcd.setCursor(10, 1);
                    lcd.print("U:"); 
                    lcd.setCursor(12, 1);  
                    lcd.print(h);
                    lcd.print("%");


          /* Print results also on the serial monitor for debugging purposes! */
                  //*********************          CO2              *********************
              
                  Serial.print(" CO2 = ");        
                  Serial.print(c);
                  //*********************          TEMPERATURA               *********************

                  Serial.print(" Temp= ");
                  Serial.print(t);

                  Serial.print(" Umid= ");
                  Serial.println(h);

                  Serial.print(" pH= ");
                  Serial.println(pH);


    delay(1000); // moved here the delay
}
           

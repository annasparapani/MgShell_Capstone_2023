

//
//  FILE: Codice Incubatore: CO2 e  Temperatura controllata
//  AUTHOR:Federica 
// 
//    DATE: 07-22
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
*/


// Librerie sensori 

#include <cozir.h>
#include <SoftwareSerial.h>
 

#define ONE_WIRE_BUS 2
SoftwareSerial sws(12, 13);
COZIR czr(&sws);


#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27, 16, 2);
#include <Keypad.h>

#include <DHT.h>
#include <DHT_U.h>
DHT dht(2, DHT11);


//Variabili globali:

float soglia = 4.90;
float CO2Threshold = 0.60; // Threshold per passare al controllo stepping in% / 100
float multiplier = 0.001;// Converione ppm-->% (10/10000) (Hardware multiplier/conversione ppm).
int SolenoidOnTime = 1000;
int SolenoidOnTime2 = 5000;
int solenoide = 8;
int pin_r = 26;
int pin_v = 28;
float tempo;
int count;
float tempo_riempimento; //definisco la variabile che mi serve per contare il tempo dall'accensione di arduino


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


void setup() {
   Serial.begin(9600);
  sws.begin(9600);
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

void loop() {

  tempo_riempimento = millis(); //inizio il conteggio del tempo
 
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
        
         
         if(c!=0)
          {

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
             delay(1000);
          
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
          
//}

//void SerialPrintResults() {

        //*********************          CO2              *********************
    
        Serial.print(" CO2 = ");        
        Serial.print(c);
        //*********************          TEMPERATURA               *********************

          Serial.print(" Temp= ");
          Serial.print(t);

           Serial.print(" Umid= ");
           Serial.println(h);
           }
           

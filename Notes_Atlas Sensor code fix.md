## Atlas Sensor code fix 
1. Check sending of data 
    - [ ] why is LED command working?
    - [ ] try sending commands in I2C (change electrical connections?)
    - [ ] try to use ArduinoUNO

2. Check receiving of data 
    - [ ] is it ever responding? 
    - [ ] read on the interrupt instead of with the if
        ```c 
        #include <SoftwareSerial.h>
        const int rxPin = 2;  // RX pin of the Arduino connected to the TX pin of the Atlas Scientific product
        const int txPin = 3;  // TX pin of the Arduino connected to the RX pin of the Atlas Scientific product

        SoftwareSerial mySerial(rxPin, txPin);  // Create a SoftwareSerial instance

        String sensorString;              // String to store incoming data
        volatile bool sensorStringComplete = false;  // Flag to indicate if a complete string has been received

        void setup() {
        Serial.begin(9600);       // Initialize the serial monitor
        mySerial.begin(9600);     // Initialize the software serial communication

        attachInterrupt(digitalPinToInterrupt(rxPin), receiveData, FALLING);
        // Attach the interrupt to the RX pin, triggering on the falling edge
        }

        void loop() {
        if (sensorStringComplete) {
            Serial.println("Arduino received a string from pH EZO: " + sensorString);
            // Process the received string as needed

            sensorString = "";        // Clear the string for new data
            sensorStringComplete = false;  // Reset the flag
        }

        // Other code in the loop
        }

        void receiveData() {
        while (mySerial.available() > 0) {
            char inChar = (char)mySerial.read();  // Get the character received
            sensorString += inChar;               // Add the character to the sensorString

            if (inChar == '\r') {                  // If the incoming character is a carriage return
            sensorStringComplete = true;         // Set the flag to indicate a complete string
            }
        }
        }
    ```


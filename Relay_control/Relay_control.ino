int relay1 = 2;
int relay2 = 3;
#define BUTTON_PIN 52

bool isRotatingCW = true; // Variable to track the direction of rotation

void setup() {
  pinMode(relay1, OUTPUT); // set pin as output for relay 1
  pinMode(relay2, OUTPUT); // set pin as output for relay 2

  // keep the motor off by keeping both HIGH
  digitalWrite(relay1, HIGH);
  digitalWrite(relay2, HIGH);

  pinMode(BUTTON_PIN, INPUT_PULLUP);

  Serial.begin(9600); // initialize serial monitor with 9600 baud
  Serial.println("Motor Direction of Rotation");
  Serial.println("Using 2 Relays");
  delay(2000);
}

void loop() {
  Serial.println(digitalRead(BUTTON_PIN));

  if (digitalRead(BUTTON_PIN) == LOW) {
    // Reverse the direction of rotation when the button is pressed
    isRotatingCW = !isRotatingCW;
    delay(100); // Debounce delay to avoid multiple toggles with a single button press
  }

  if (isRotatingCW) {
    // Rotate in CW direction
    digitalWrite(relay1, HIGH);  // turn relay 1 OFF
    digitalWrite(relay2, LOW);   // turn relay 2 ON
    Serial.println("Rotating in CW");
  } else {
    // Rotate in CCW direction
    digitalWrite(relay1, LOW);   // turn relay 1 ON
    digitalWrite(relay2, HIGH);  // turn relay 2 OFF
    Serial.println("Rotating in CCW");
  }

  delay(3000); // wait for 3 seconds

  // Stop the motor
  digitalWrite(relay1, HIGH); // turn relay 1 OFF
  digitalWrite(relay2, HIGH); // turn relay 2 OFF
  Serial.println("Stopped");
  //delay(2000); // stop for 2 seconds

  Serial.println("===============");
}

// PH Reading trial 

// Pin assignments
const int pHPin1 = A0;
const int pHPin2 = A1;

// Calibration values (to be determined during calibration)
const float calibrationOffset = 0.0;  // Calibration offset value
const float calibrationFactor = 1.0;  // Calibration factor value

void setup() {
  Serial.begin(9600);
}

void loop() {
  // Read pH sensor values
  int pHValue1 = analogRead(pHPin1);
  int pHValue2 = analogRead(pHPin2);

  // Calculate differential pH value
  float pHDiff = (pHValue1 - pHValue2) * calibrationFactor + calibrationOffset;
  float pHDiffMillivolts = pHDiff * 4.88;  // Convert to millivolts

  Serial.print("pH (mV): ");
  Serial.println(pHDiffMillivolts);


  delay(1000);  // Delay for stability
}


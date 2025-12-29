
void setup() {

  Serial.begin(9600); 
  Serial.println("Hardware Serial Ready");
  Serial1.begin(9600); 
  Serial1.println("Hardware Serial 1 Ready");
}

void loop() {

  if (Serial.available()) {
    String dataFromSerial = Serial.readStringUntil('\n'); 
    Serial1.println(dataFromSerial); 
  }

  if (Serial1.available()) {
    String dataFromSerial1 = Serial1.readStringUntil('\n'); 
    Serial.println(dataFromSerial1); 
  }
}
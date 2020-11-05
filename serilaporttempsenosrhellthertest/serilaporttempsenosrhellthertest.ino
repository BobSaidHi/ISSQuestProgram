void setup() {
  Serial.begin(19200);
}

void loop() {
  Serial.print("Hello");
  Serial.println(" There");
  delay(1000);
}

void setup() {
  //run once- setup code
  pinMode(9, OUTPUT); //output mode
  pinMode(12, INPUT); //output mode
}

void loop() {
  //main loop
  if(digitalRead(13)== LOW) {
    digitalWrite(9, HIGH);  //on
    delay(1000);  //delay
  }
  digitalWrite(9, LOW); //offf
  delay(1000);  //dealy
}

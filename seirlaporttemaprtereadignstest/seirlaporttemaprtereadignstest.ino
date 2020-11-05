#include <SHT3x.h>

SHT3x sensor;
void setup() {
  // put your setup code here, to run once:
  //Make sure to set montior to 19200 buad
  Serial.begin(19200);
  sensor.Begin();
}

void loop() {
  // put your main code here, to run repeatedly:
  Serial.print("Temperature (\xC2\xB0 C)");
  Serial.println(sensor.GetTemperature());
  Serial.print("Temperature (\xC2\xB0 F)");
  Serial.println(sensor.GetTemperature(SHT3x::Far));
  Serial.print("Temperature (K)");
  Serial.println(sensor.GetTemperature(SHT3x::Kel));
  delay(1000);
  Serial.println("");
}

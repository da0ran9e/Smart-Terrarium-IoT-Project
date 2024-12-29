#include "SoilSensor_Controller.h"

void setup() {
  Serial.begin(115200);
}

void loop() {
  // sensorVal = analogRead(SensorPin);
  // Serial.print("Moisture Value: ");
  // Serial.println(sensorVal);
  //Serial.println(getMoisture());
  PrintAnalog();
  delay(100);
}


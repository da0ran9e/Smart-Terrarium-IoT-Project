#define SensorPin       34  //D34 
int     sensorVal;

void getMoisture(){
  sensorVal = analogRead(SensorPin);

  if (sensorVal > (wetSoilVal - 100) && sensorVal < (drySoilVal + 100) ){
    moisturePercentage = map(sensorVal ,drySoilVal, wetSoilVal, 0, 100);
    // Print result to serial monitor
    Serial.print("Moisture Percentage: ");
    Serial.print(moisturePercentage);
    Serial.println(" %");
  }
  else{
    Serial.println(sensorVal);
  }
  delay(100);
}

void setup() {
  Serial.begin(115200);
}

void loop() {
  // sensorVal = analogRead(SensorPin);
  // Serial.print("Moisture Value: ");
  // Serial.println(sensorVal);
  getMoisture();
}


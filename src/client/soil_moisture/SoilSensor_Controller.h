#define SensorPin       5  
int wetSoilVal = 930 ;  //min value when soil is wet
int drySoilVal = 3000 ;  //max value when soil is dry

void getMoisture(){
  int sensorVal = analogRead(SensorPin);

  if (sensorVal > (wetSoilVal - 100) && sensorVal < (drySoilVal + 100) ){
    int moisturePercentage = map(sensorVal ,drySoilVal, wetSoilVal, 0, 100);
    Serial.print("Moisture Percentage: ");
    Serial.print(moisturePercentage);
    Serial.println(" %");
  }
  else{
    Serial.println(sensorVal);
  }
  delay(100);
}

void PrintAnalog(){
    int sensorVal = analogRead(SensorPin);
    Serial.println(sensorVal);
}

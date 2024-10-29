#define SensorPin       5  
int     sensorVal;
int     moisturePercentage;
int wetSoilVal = 930 ;  //min value when soil is wet
int drySoilVal = 3000 ;  //max value when soil is dry

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


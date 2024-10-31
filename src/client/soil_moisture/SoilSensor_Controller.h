#define SensorPin       A0  
int wetSoilVal = 500 ;  //min value when soil is wet
int drySoilVal = 714 ;  //max value when soil is dry

int getMoisture(){
  int sensorVal = analogRead(SensorPin);

  if (sensorVal > (wetSoilVal - 20) && sensorVal < (drySoilVal + 20) ){
    return map(sensorVal ,drySoilVal, wetSoilVal, 0, 100); 
  }
  else{
    return -1;
  }
}

void PrintAnalog(){
    int sensorVal = analogRead(SensorPin);
    Serial.println(sensorVal);
}

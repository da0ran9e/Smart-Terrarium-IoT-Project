#include "DHT_Controller.h"

void setup()
{
  Serial.begin(115200);
  delay(10);
  DHTSetup();
}

void loop()
{
  PrintDHTData();
  delay(500);
}
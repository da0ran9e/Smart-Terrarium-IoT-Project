#include "wifi.h"

const char* ssid     = "O R A N G E"; 
const char* password = "1234567tam";

void setup()
{
  Serial.begin(115200);
  //WiFiSetup();
  ConnectWiFi(ssid, password);
}

void loop()
{
  //WiFiScan();
}
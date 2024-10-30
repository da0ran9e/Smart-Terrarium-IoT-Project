#include "WiFi_Controller.h"

const char* ssid     = "Tung home"; 
const char* password = "0963617074";

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
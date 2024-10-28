#ifndef wifi_h
#define wifi_h

#include "ESP8266WiFi.h"

void WiFiSetup(){
  Serial.println();
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);
}

void WiFiScan(){
  Serial.print("Scan start ... ");
  int n = WiFi.scanNetworks();
  Serial.print(n);
  Serial.println(" network(s) found");
  for (int i = 0; i < n; i++)
  {
    Serial.println(WiFi.SSID(i));
  }
  Serial.println();

  delay(5000);
}

void ConnectWiFi(const char* ssid, const char* password){
  Serial.println('\n');
  
  WiFi.begin(ssid, password);             
  Serial.print("Connecting to ");
  Serial.print(ssid); Serial.println(" ...");

  int i = 0;
  while (WiFi.status() != WL_CONNECTED) { 
    delay(1000);
    Serial.print(++i); 
    Serial.print(' ');
  }

  Serial.println('\n');
  Serial.println("Connection established!");  
  Serial.print("IP address:\t");
  Serial.println(WiFi.localIP()); 
}

#endif
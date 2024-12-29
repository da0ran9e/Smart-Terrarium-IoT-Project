#include <WiFiManager.h> 

// WiFi and MQTT settings
const char* AP_NAME     = "8266AP"; 
const char* AP_PASSWORD = "password";

#define BLINK 4

WiFiClient espClient;

void setup() {
    pinMode(BLINK, OUTPUT);
    digitalWrite(BLINK, LOW);
    Serial.begin(115200);
    
    WiFiManager wm;
    bool res;
    res = wm.autoConnect(AP_NAME, AP_PASSWORD); 
    if(!res) {
        Serial.println("Failed to connect");
        digitalWrite(BLINK, HIGH);
        delay(25);
        digitalWrite(BLINK, LOW);
        delay(25);
        digitalWrite(BLINK, HIGH);
        delay(25);
        digitalWrite(BLINK, LOW);
        delay(25);
    } 
    else {
        Serial.println("connected...yeey :)");
        digitalWrite(BLINK, HIGH);
        delay(25);
    }
}

void loop() {
    
}
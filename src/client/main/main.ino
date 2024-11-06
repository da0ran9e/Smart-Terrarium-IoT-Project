#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include "MQTT_Handler.h"

// Time interval to publish sensor data (in milliseconds)
const unsigned long PUBLISH_INTERVAL = 5000;  // e.g., publish every 5 seconds
unsigned long lastPublishTime = 0;
unsigned long activeTime = 0;

void setup() {
    Serial.begin(115200);
    Serial.println("Starting setup...");

    mqttSetup();
    Serial.println("MQTT setup complete.");

    DHTSetup();
    Serial.println("DHT setup complete.");
}

void loop() {
    // Ensure MQTT connection and handle incoming messages
    mqttLoop();
    delay(10);

    // Publish sensor data at the specified interval
    unsigned long currentTime = millis();
    if (currentTime - lastPublishTime >= PUBLISH_INTERVAL) {
        publishSensorData();  // Function to read and publish temperature & moisture
        lastPublishTime = currentTime;
    }
}

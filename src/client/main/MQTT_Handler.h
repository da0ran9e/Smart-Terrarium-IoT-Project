#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include "Sensor_Handler.h"  // Include Sensor_Handler for access to GetDHTData and getMoisture

const char* SSID     = "Tung home"; 
const char* PASSWORD = "0963617074";

const char *MQTT_BROKER = "broker.hivemq.com";
const char *MQTT_TOPIC = "ict66/smarterra/sensors/";
const int MQTT_PORT = 1883;

WiFiClient espClient;
PubSubClient MQTTClient(espClient);

// Updated SensorData structure to include moisture
struct SensorData {
    float temperature;
    float humidity;
    int moisture;

    void toJson(char *jsonBuffer, size_t bufferSize) const {
        StaticJsonDocument<200> jsonDoc;
        jsonDoc["temperature"] = temperature;
        jsonDoc["humidity"] = humidity;
        jsonDoc["moisture"] = moisture;
        serializeJson(jsonDoc, jsonBuffer, bufferSize);
    }
};

// ControlMessage structure for pump control messages
struct ControlMessage {
    bool pump;
    unsigned int duration;

    bool fromJson(const char *jsonBuffer, size_t length) {
        StaticJsonDocument<200> jsonDoc;
        DeserializationError error = deserializeJson(jsonDoc, jsonBuffer, length);
        if (error) {
            Serial.print("deserializeJson() failed: ");
            Serial.println(error.c_str());
            return false;
        }
        pump = jsonDoc["pump"] | false;
        duration = jsonDoc["duration"] | 0;
        return true;
    }
};

void connectToWiFi();
void mqttCallback(char *topic, byte *payload, unsigned int length);
void connectToMQTTBroker();
void mqttPublishMessage(const char *topic, const SensorData &data);
void publishSensorData();  // New function to publish sensor data

void mqttSetup() {
    connectToWiFi();
    MQTTClient.setServer(MQTT_BROKER, MQTT_PORT);
    MQTTClient.setCallback(mqttCallback);
    connectToMQTTBroker();
}

void connectToWiFi() {
    WiFi.begin(SSID, PASSWORD);
    Serial.print("Connecting to WiFi");
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\nConnected to the WiFi network");
}

void mqttPublishMessage(const char *topic, const SensorData &data) {
    char message[256];
    data.toJson(message, sizeof(message));
    Serial.print("Sending to topic: ");
    Serial.println(topic);
    Serial.print("Message: ");
    Serial.println(message);
    MQTTClient.publish(topic, message);
    Serial.println("Sent~");
}

void mqttCallback(char *topic, byte *payload, unsigned int length) {
    Serial.print("Message received on topic: ");
    Serial.println(topic);

    ControlMessage controlMsg;
    if (controlMsg.fromJson((char*)payload, length)) {
        if (controlMsg.pump) {
            Serial.print("Turning pump ON for ");
            Serial.print(controlMsg.duration);
            Serial.println(" seconds");
            // Code to turn the pump on
            digitalWrite(PUMP_PIN, HIGH); // Assume PUMP_PIN is the GPIO controlling the pump

            delay(controlMsg.duration * 1000); // Run pump for 'duration' seconds
            
            // Code to turn the pump off after duration
            digitalWrite(PUMP_PIN, LOW);
            Serial.println("Turning pump OFF after duration");
        } else {
            Serial.println("Turning pump OFF");
            // Code to turn the pump off immediately
            digitalWrite(PUMP_PIN, LOW);
        }
    } else {
        Serial.println("Failed to parse control message.");
    }
}

void connectToMQTTBroker() {
    while (!MQTTClient.connected()) {
        Serial.println("Attempting to connect to MQTT broker...");
        String client_id = "esp8266-client-" + String(ESP.getChipId(), HEX);
        Serial.printf("Connecting as %s\n", client_id.c_str());
        if (MQTTClient.connect(client_id.c_str())) {
            Serial.println("Connected to MQTT broker!");
            MQTTClient.subscribe(MQTT_TOPIC);
        } else {
            Serial.print("Failed to connect, rc=");
            Serial.print(MQTTClient.state());
            Serial.println(" Trying again in 5 seconds.");
            delay(5000);
        }
    }
}

void mqttLoop() {
    if (!MQTTClient.connected()) {
        connectToMQTTBroker();
    }
    MQTTClient.loop();
}

// Function to read sensor data and publish it to MQTT
void publishSensorData() {
    // Get DHT and moisture data
    DHTData dhtData = GetDHTData();
    int moisture = getMoisture();

    // Prepare sensor data for MQTT
    SensorData sensorData;
    sensorData.temperature = dhtData.temperature;
    sensorData.humidity = dhtData.humidity;
    sensorData.moisture = moisture;

    // Publish sensor data
    mqttPublishMessage(MQTT_TOPIC, sensorData);
}

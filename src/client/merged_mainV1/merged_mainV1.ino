// Combined and optimized version of main.ino, MQTT_Handler.h, and Sensor_Handler.h

#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <DHT.h>

// WiFi and MQTT settings
const char* SSID     = ""; 
const char* PASSWORD = "";
const char *MQTT_BROKER = "broker.hivemq.com";
const char *MQTT_TOPIC = "ict66/smarterra/sensors/";
const int MQTT_PORT = 1883;

// Pin and DHT settings
#define DHT_PIN 1
#define DHTTYPE DHT11
#define SENSOR_PIN A0
#define PUMP_PIN 5
const int WET_VAL = 500;  // Wet soil threshold
const int DRY_VAL = 714;  // Dry soil threshold

// Time interval to publish sensor data (in milliseconds)
const unsigned long PUBLISH_INTERVAL = 5000;
unsigned long lastPublishTime = 0;

WiFiClient espClient;
PubSubClient MQTTClient(espClient);
DHT dht(DHT_PIN, DHTTYPE);

// SensorData structure to hold DHT and moisture data
struct SensorData {
    float temperature;
    float humidity;
    int moisture;

    void toJson(char *jsonBuffer, size_t bufferSize) const {
        StaticJsonDocument<150> jsonDoc;  // Reduced from 200 to 150
        jsonDoc["temperature"] = temperature;
        jsonDoc["humidity"] = humidity;
        jsonDoc["moisture"] = moisture;
        serializeJson(jsonDoc, jsonBuffer, bufferSize);
    }
};

// ControlMessage structure for pump control
struct ControlMessage {
    bool pump;
    unsigned int duration;

    bool fromJson(const char *jsonBuffer, size_t length) {
        StaticJsonDocument<100> jsonDoc;  // Reduced from 200 to 100
        auto error = deserializeJson(jsonDoc, jsonBuffer, length);
        if (error) return false;
        pump = jsonDoc["pump"] | false;
        duration = jsonDoc["duration"] | 0;
        return true;
    }
};

// WiFi connection setup
void connectToWiFi() {
    WiFi.begin(SSID, PASSWORD);
    Serial.print("Connecting to WiFi");
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\nConnected to the WiFi network");
}

// MQTT connection setup
void connectToMQTTBroker() {
    while (!MQTTClient.connected()) {
        Serial.println("Connecting to MQTT broker...");
        if (MQTTClient.connect("esp8266-client")) {
            Serial.println("Connected to MQTT broker!");
            MQTTClient.subscribe(MQTT_TOPIC);
        } else {
            delay(5000);
        }
    }
}

// MQTT publish message
void mqttPublishMessage(const char *topic, const SensorData &data) {
    char message[128];  // Reduced buffer size
    data.toJson(message, sizeof(message));
    MQTTClient.publish(topic, message);
}

// Handle incoming MQTT messages
void mqttCallback(char *topic, byte *payload, unsigned int length) {
    ControlMessage controlMsg;
    if (controlMsg.fromJson((char*)payload, length)) {
        digitalWrite(PUMP_PIN, controlMsg.pump ? HIGH : LOW);
        if (controlMsg.pump) delay(controlMsg.duration * 1000);
        digitalWrite(PUMP_PIN, LOW);
    }
}

// MQTT initialization
void mqttSetup() {
    connectToWiFi();
    MQTTClient.setServer(MQTT_BROKER, MQTT_PORT);
    MQTTClient.setCallback(mqttCallback);
    connectToMQTTBroker();
}

// Setup DHT and pump pins
void DHTSetup() {
    dht.begin();
    pinMode(PUMP_PIN, OUTPUT);
    digitalWrite(PUMP_PIN, LOW);
}

// Function to get DHT data
SensorData GetSensorData() {
    SensorData data;
    data.humidity = dht.readHumidity();
    float temp = dht.readTemperature(true);
    data.temperature = isnan(temp) ? 0 : temp;
    data.moisture = map(analogRead(SENSOR_PIN), DRY_VAL, WET_VAL, 0, 100);
    return data;
}

// Publish sensor data to MQTT
void publishSensorData() {
    SensorData sensorData = GetSensorData();
    mqttPublishMessage(MQTT_TOPIC, sensorData);
}

void setup() {
    Serial.begin(115200);
    DHTSetup();
    mqttSetup();
}

void loop() {
    if (!MQTTClient.connected()) connectToMQTTBroker();
    MQTTClient.loop();

    unsigned long currentTime = millis();
    if (currentTime - lastPublishTime >= PUBLISH_INTERVAL) {
        publishSensorData();
        lastPublishTime = currentTime;
    }
    delay(10);
}

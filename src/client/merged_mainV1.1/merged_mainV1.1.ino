#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <DHT.h>

// WiFi and MQTT settings
const char* SSID = "oản"; 
const char* PASSWORD = "1234567tam";

const char* MQTT_BROKER = "broker.emqx.io";
const char* SENSOR_TOPIC = "ict66/smarterra/sensors/";
const char* QUERY_TOPIC = "ict66/smarterra/queries/";
const char* KEEPALIVE_TOPIC = "ict66/smarterra/keepalive/";
const int MQTT_PORT = 1883;
const char* MQTT_USERNAME = "";
const char* MQTT_PASSWORD = "";

// Pin and DHT settings
#define DHT_PIN 1
#define SENSOR_PIN A0
#define PUMP_PIN 5
#define FAN_PIN 4
#define WIFI_BLINK 14
#define MQTT_BLINK 12
#define DHT_BLINK 13
#define SOIL_BLINK 15

#define DHTTYPE DHT11

const int WET_VAL = 500;  // Wet soil threshold
const int DRY_VAL = 714;  // Dry soil threshold

// Time intervals (in milliseconds)
const unsigned long SENSOR_INTERVAL = 5 * 60 * 1000; // 5 minutes for sensor data
const unsigned long KEEPALIVE_INTERVAL = 5 * 1000;  // 30 seconds for keep-alive

unsigned long lastSensorTime = 0;
unsigned long lastKeepAliveTime = 0;

WiFiClient espClient;
PubSubClient MQTTClient(espClient);
DHT dht(DHT_PIN, DHTTYPE);

// SensorData structure to hold DHT and moisture data
struct SensorData {
    float temperature;
    float humidity;
    int moisture;

    void toJson(char *jsonBuffer, size_t bufferSize) const {
        StaticJsonDocument<150> jsonDoc;
        jsonDoc["temperature"] = temperature;
        jsonDoc["humidity"] = humidity;
        jsonDoc["moisture"] = moisture;
        serializeJson(jsonDoc, jsonBuffer, bufferSize);
    }
};

// ControlMessage structure for pump control
struct ControlMessage {
    bool pump;
    bool fan;
    unsigned int duration;

    bool fromJson(const char *jsonBuffer, size_t length) {
        StaticJsonDocument<100> jsonDoc;
        auto error = deserializeJson(jsonDoc, jsonBuffer, length);
        if (error) return false;
        pump = jsonDoc["pump"] | false;
        fan = jsonDoc["fan"] | false;
        duration = jsonDoc["duration"] | 0;
        return true;
    }
};

// WiFi connection setup
void connectToWiFi() {
    WiFi.begin(SSID, PASSWORD);
    digitalWrite(WIFI_BLINK, LOW);
    while (WiFi.status() != WL_CONNECTED) {
        delay(250);
        digitalWrite(WIFI_BLINK, LOW);
        delay(250);
        digitalWrite(WIFI_BLINK, HIGH);
    }
    digitalWrite(WIFI_BLINK, HIGH);
}

// MQTT connection setup
void connectToMQTTBroker() {
    while (!MQTTClient.connected()) {
        String client_id = "esp8266-client-" + String(ESP.getChipId(), HEX);
        digitalWrite(MQTT_BLINK, HIGH);
        if (MQTTClient.connect(client_id.c_str(), MQTT_USERNAME, MQTT_PASSWORD)) {
            digitalWrite(MQTT_BLINK, HIGH);
            // Subscribe to control topic
            MQTTClient.subscribe(QUERY_TOPIC);
        } else {
            delay(5000); // Retry after 5 seconds
        }
    }
}

// MQTT publish message
void mqttPublishMessage(const char *topic, const SensorData &data) {
    char message[128];
    data.toJson(message, sizeof(message));
    MQTTClient.publish(topic, message);
}

void mqttPublishKeepAlive() {
    MQTTClient.publish(KEEPALIVE_TOPIC, "ESP8266 is online");
}

// Handle incoming MQTT messages
void mqttCallback(char *topic, byte *payload, unsigned int length) {
    if (strcmp(topic, QUERY_TOPIC) == 0) {
        ControlMessage controlMsg;
        if (controlMsg.fromJson((char*)payload, length)) {
            if (controlMsg.pump) {
                //Serial.print("Turning pump ON for ");
                //Serial.print(controlMsg.duration);
                //Serial.println(" seconds");
                digitalWrite(PUMP_PIN, HIGH);
                delay(controlMsg.duration * 1000); // Blocking delay
                digitalWrite(PUMP_PIN, LOW);
            }
            if (controlMsg.fan) {
                //Serial.print("Turning fan ON for ");
                //Serial.print(controlMsg.duration);
                //Serial.println(" seconds");
                digitalWrite(FAN_PIN, HIGH);
                delay(controlMsg.duration * 1000); // Blocking delay
                digitalWrite(FAN_PIN, LOW);
            }
        }
    }
}

// MQTT initialization
void mqttSetup() {
    MQTTClient.setServer(MQTT_BROKER, MQTT_PORT);
    MQTTClient.setCallback(mqttCallback);
    connectToWiFi();
    connectToMQTTBroker();
}

// Setup DHT and pump pins
void DHTSetup() {
    dht.begin();
    digitalWrite(DHT_BLINK, HIGH);
}

// Function to get DHT data
SensorData GetSensorData() {
    SensorData data;
    data.humidity = dht.readHumidity();
    data.temperature = dht.readTemperature();
    data.moisture = map(analogRead(SENSOR_PIN), DRY_VAL, WET_VAL, 0, 100);
    return data;
}

// Publish sensor data to MQTT
void publishSensorData() {
    SensorData sensorData = GetSensorData();
    mqttPublishMessage(SENSOR_TOPIC, sensorData);
}

void setup() {
    pinMode(PUMP_PIN, OUTPUT);
    pinMode(FAN_PIN, OUTPUT);
    pinMode(WIFI_BLINK, OUTPUT);
    pinMode(MQTT_BLINK, OUTPUT);
    pinMode(DHT_BLINK, OUTPUT);
    pinMode(SOIL_BLINK, OUTPUT);

    digitalWrite(PUMP_PIN, LOW);
    digitalWrite(FAN_PIN, LOW);

    //Serial.begin(115200);
    DHTSetup();
    mqttSetup();
}

void loop() {
    if (!MQTTClient.connected()) connectToMQTTBroker();
    MQTTClient.loop();

    unsigned long currentTime = millis();

    // Publish sensor data every 5 minutes
    if (currentTime - lastSensorTime >= SENSOR_INTERVAL) {
        publishSensorData();
        lastSensorTime = currentTime;
    }

    // Publish keep-alive message every 30 seconds
    if (currentTime - lastKeepAliveTime >= KEEPALIVE_INTERVAL) {
        mqttPublishKeepAlive();
        lastKeepAliveTime = currentTime;
    }

    delay(10); // Small delay to avoid watchdog resets
}

#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <DHT.h>
#include <Arduino_FreeRTOS.h>

// WiFi and MQTT settings
const char* SSID = "Tung home";
const char* PASSWORD = "0963617074";

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

#define DHTTYPE DHT11

const int WET_VAL = 500;
const int DRY_VAL = 714;

WiFiClient espClient;
PubSubClient MQTTClient(espClient);
DHT dht(DHT_PIN, DHTTYPE);

// Sensor data structure
struct SensorData {
    float temperature;
    float humidity;
    int moisture;

    void toJson(char* jsonBuffer, size_t bufferSize) const {
        StaticJsonDocument<150> jsonDoc;
        jsonDoc["temperature"] = temperature;
        jsonDoc["humidity"] = humidity;
        jsonDoc["moisture"] = moisture;
        serializeJson(jsonDoc, jsonBuffer, bufferSize);
    }
};

// Control message structure
struct ControlMessage {
    bool pump;
    bool fan;
    unsigned int duration;

    bool fromJson(const char* jsonBuffer, size_t length) {
        StaticJsonDocument<100> jsonDoc;
        auto error = deserializeJson(jsonDoc, jsonBuffer, length);
        if (error) return false;
        pump = jsonDoc["pump"] | false;
        fan = jsonDoc["fan"] | false;
        duration = jsonDoc["duration"] | 0;
        return true;
    }
};

// FreeRTOS task handles
TaskHandle_t sensorTaskHandle;
TaskHandle_t mqttTaskHandle;
TaskHandle_t keepAliveTaskHandle;

// WiFi connection setup
void connectToWiFi() {
    WiFi.begin(SSID, PASSWORD);
    while (WiFi.status() != WL_CONNECTED) {
        digitalWrite(WIFI_BLINK, LOW);
        delay(250);
        digitalWrite(WIFI_BLINK, HIGH);
        delay(250);
    }
    digitalWrite(WIFI_BLINK, HIGH);
}

// MQTT connection setup
void connectToMQTTBroker() {
    while (!MQTTClient.connected()) {
        String client_id = "esp8266-client-" + String(ESP.getChipId(), HEX);
        if (MQTTClient.connect(client_id.c_str(), MQTT_USERNAME, MQTT_PASSWORD)) {
            MQTTClient.subscribe(QUERY_TOPIC);
            digitalWrite(MQTT_BLINK, HIGH);
        } else {
            delay(5000);
        }
    }
}

// MQTT publish message
void mqttPublishMessage(const char* topic, const SensorData& data) {
    char message[128];
    data.toJson(message, sizeof(message));
    MQTTClient.publish(topic, message);
}

void mqttPublishKeepAlive() {
    MQTTClient.publish(KEEPALIVE_TOPIC, "ESP8266 is online");
}

// Handle incoming MQTT messages
void mqttCallback(char* topic, byte* payload, unsigned int length) {
    if (strcmp(topic, QUERY_TOPIC) == 0) {
        ControlMessage controlMsg;
        if (controlMsg.fromJson((char*)payload, length)) {
            if (controlMsg.pump) {
                digitalWrite(PUMP_PIN, HIGH);
                delay(controlMsg.duration * 1000);
                digitalWrite(PUMP_PIN, LOW);
            }
            if (controlMsg.fan) {
                digitalWrite(FAN_PIN, HIGH);
                delay(controlMsg.duration * 1000);
                digitalWrite(FAN_PIN, LOW);
            }
        }
    }
}

// Get sensor data
SensorData getSensorData() {
    SensorData data;
    data.humidity = dht.readHumidity();
    data.temperature = dht.readTemperature();
    data.moisture = map(analogRead(SENSOR_PIN), DRY_VAL, WET_VAL, 0, 100);
    return data;
}

// FreeRTOS tasks
void sensorTask(void* parameter) {
    while (true) {
        SensorData sensorData = getSensorData();
        mqttPublishMessage(SENSOR_TOPIC, sensorData);
        vTaskDelay(pdMS_TO_TICKS(5 * 60 * 1000)); // 5 minutes
    }
}

void mqttTask(void* parameter) {
    while (true) {
        if (!MQTTClient.connected()) connectToMQTTBroker();
        MQTTClient.loop();
        vTaskDelay(pdMS_TO_TICKS(10)); // Small delay to avoid watchdog resets
    }
}

void keepAliveTask(void* parameter) {
    while (true) {
        mqttPublishKeepAlive();
        vTaskDelay(pdMS_TO_TICKS(30 * 1000)); // 30 seconds
    }
}

void setup() {
    pinMode(PUMP_PIN, OUTPUT);
    pinMode(FAN_PIN, OUTPUT);
    pinMode(WIFI_BLINK, OUTPUT);
    pinMode(MQTT_BLINK, OUTPUT);

    digitalWrite(PUMP_PIN, LOW);
    digitalWrite(FAN_PIN, LOW);

    Serial.begin(115200);
    dht.begin();

    connectToWiFi();

    MQTTClient.setServer(MQTT_BROKER, MQTT_PORT);
    MQTTClient.setCallback(mqttCallback);

    // Create FreeRTOS tasks
    xTaskCreate(sensorTask, "SensorTask", 1024, NULL, 1, &sensorTaskHandle);
    xTaskCreate(mqttTask, "MQTTTask", 1024, NULL, 1, &mqttTaskHandle);
    xTaskCreate(keepAliveTask, "KeepAliveTask", 1024, NULL, 1, &keepAliveTaskHandle);
}

void loop() {
    // FreeRTOS handles the tasks; nothing to do here.
}

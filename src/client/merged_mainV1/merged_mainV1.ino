#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <DHT.h>

// WiFi and MQTT settings
const char* SSID     = "Tung home"; 
const char* PASSWORD = "0963617074";

const char *MQTT_BROKER = "broker.emqx.io";
const char *MQTT_TOPIC = "ict66/smarterra/sensors/";
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
    bool fan;
    unsigned int duration;

    bool fromJson(const char *jsonBuffer, size_t length) {
        StaticJsonDocument<100> jsonDoc;  // Reduced from 200 to 100
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
    //Serial.print("Connecting to WiFi");
    digitalWrite(WIFI_BLINK, LOW);
    while (WiFi.status() != WL_CONNECTED) {
        delay(250);
        digitalWrite(WIFI_BLINK, LOW);
        delay(250);
        //Serial.print("."); 
        digitalWrite(WIFI_BLINK, HIGH);
    }
    digitalWrite(WIFI_BLINK, HIGH);
    //Serial.println("\nConnected to the WiFi network");
}

// MQTT connection setup
void connectToMQTTBroker() {
    while (!MQTTClient.connected()) {
        String client_id = "esp8266-client-" + String(ESP.getChipId(), HEX);
        //Serial.println("Connecting to MQTT broker...");
        digitalWrite(MQTT_BLINK, HIGH);
        if (MQTTClient.connect(client_id.c_str(), MQTT_USERNAME, MQTT_PASSWORD)) {
            //Serial.println("Connected to MQTT broker!");
            digitalWrite(MQTT_BLINK, HIGH);
            MQTTClient.subscribe(MQTT_TOPIC);
        } else {
            //Serial.print("Failed to connect to MQTT broker, rc=");
            //Serial.println(MQTTClient.state());
            //Serial.println("Trying again in 5 seconds.");
            delay(1000);
            digitalWrite(MQTT_BLINK, LOW);
            delay(1000);
            digitalWrite(MQTT_BLINK, HIGH);
            delay(1000);
            digitalWrite(MQTT_BLINK, LOW);
            delay(1000);
            digitalWrite(MQTT_BLINK, HIGH);
            delay(1000);
            digitalWrite(MQTT_BLINK, LOW);
            delay(1000);
        }
    }
}

// MQTT publish message
void mqttPublishMessage(const char *topic, const SensorData &data) {
    char message[128];  // Reduced buffer size
    data.toJson(message, sizeof(message));
    MQTTClient.publish(topic, message);
}

void mqttCallback(char *topic, byte *payload, unsigned int length) {
    ControlMessage controlMsg;
    if (controlMsg.fromJson((char*)payload, length)) {
        if (controlMsg.pump) {
            digitalWrite(PUMP_PIN, HIGH);
            delay(controlMsg.duration * 1000); // Consider using millis() for non-blocking
            digitalWrite(PUMP_PIN, LOW);
        }
        if (controlMsg.fan) {
            digitalWrite(FAN_PIN, HIGH);
            delay(controlMsg.duration * 1000); // Consider using millis() for non-blocking
            digitalWrite(FAN_PIN, LOW);
        }
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
    digitalWrite(DHT_BLINK, HIGH);
}

// Function to get DHT data
SensorData GetSensorData() {
    SensorData data;
    data.humidity = dht.readHumidity();
    float temp = dht.readTemperature();
    data.temperature = isnan(temp) ? 0 : temp;
    data.moisture = map(analogRead(SENSOR_PIN), DRY_VAL, WET_VAL, 0, 100);
    if(data.humidity != 0 && data.temperature != 0){
        digitalWrite(DHT_BLINK, HIGH);
    } else {
        digitalWrite(DHT_BLINK, LOW);
    }
    if(data.moisture <= 100) {
        digitalWrite(SOIL_BLINK, HIGH);
    } else {
        digitalWrite(SOIL_BLINK, LOW);
    }
    return data;
}

// Publish sensor data to MQTT
void publishSensorData() {
    SensorData sensorData = GetSensorData();
    mqttPublishMessage(MQTT_TOPIC, sensorData);
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
    digitalWrite(WIFI_BLINK, LOW);
    digitalWrite(MQTT_BLINK, LOW);
    digitalWrite(DHT_BLINK, LOW);
    digitalWrite(SOIL_BLINK, LOW);

    //Serial.begin(115200);
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

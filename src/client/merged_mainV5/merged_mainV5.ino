#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <DHT.h>
#include <time.h>

// WiFi and MQTT settings
const char* SSID     = "Tung home"; 
const char* PASSWORD = "0963617074";

const char* MQTT_BROKER = "broker.emqx.io";
const char* SENSOR_TOPIC = "ict66/smarterra/sensors/";
const char* QUERY_TOPIC = "ict66/smarterra/commands/";
const char* KEEPALIVE_TOPIC = "ict66/smarterra/keepalive/";
const int MQTT_PORT = 1883;
const char* MQTT_USERNAME = "";
const char* MQTT_PASSWORD = "";

// NTP Server and Timezone
const char* NTP_SERVER = "pool.ntp.org";
const long GMT_OFFSET_SEC = 7 * 3600; // Vietnam GMT+7
const int DAYLIGHT_OFFSET_SEC = 0;

// Pin and DHT settings
#define LED1 2
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
const unsigned long KEEPALIVE_INTERVAL = 3000;
unsigned long lastPublishTime = 0;
unsigned long lastKeepAliveTime = 0;
unsigned long pumpStartTime = 0;
unsigned long fanStartTime = 0;
unsigned long pumpDuration = 0;
unsigned long fanDuration = 0;
bool isPumpOn = false;
bool isFanOn = false;

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

// Scheduling structure
struct Schedule {
    time_t timestamp;
    unsigned int duration;
    bool isPump; // true for pump, false for fan
};

std::vector<Schedule> schedules;

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

// Sync time using NTP
void syncTime() {
    configTime(GMT_OFFSET_SEC, DAYLIGHT_OFFSET_SEC, NTP_SERVER);
    while (time(nullptr) < 1609459200) { // Wait until time is synchronized
        delay(100);
    }
}

// Parse "HH:MM" into time_t for the current day
time_t parseTime(const char* timeStr) {
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo)) return 0;

    int hour, minute;
    sscanf(timeStr, "%d:%d", &hour, &minute);

    timeinfo.tm_hour = hour;
    timeinfo.tm_min = minute;
    timeinfo.tm_sec = 0;

    return mktime(&timeinfo);
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
            MQTTClient.subscribe(QUERY_TOPIC);
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
    char message[128]; 
    data.toJson(message, sizeof(message));
    MQTTClient.publish(topic, message);
}

void mqttPublishKeepAlive() {
    MQTTClient.publish(KEEPALIVE_TOPIC, "ESP8266 is online");
}

void handlePumpFan() {
    if (isPumpOn && millis() - pumpStartTime >= pumpDuration * 1000) {
        digitalWrite(PUMP_PIN, LOW);
        isPumpOn = false;
    }
    if (isFanOn && millis() - fanStartTime >= fanDuration * 1000) {
        digitalWrite(FAN_PIN, LOW);
        isFanOn = false;
    }
}

// Check and execute scheduled tasks
void handleSchedules() {
    time_t now = time(nullptr);
    for (auto it = schedules.begin(); it != schedules.end(); ) {
        if (now >= it->timestamp && now < it->timestamp + it->duration) {
            if (it->isPump) {
                digitalWrite(PUMP_PIN, HIGH);
                pumpStartTime = millis();
                pumpDuration = it->duration;
                isPumpOn = true;
            } else {
                digitalWrite(FAN_PIN, HIGH);
                fanStartTime = millis();
                fanDuration = it->duration;
                isFanOn = true;
            }
            it = schedules.erase(it);
        } else {
            ++it;
        }
    }
}

void mqttCallback(char *topic, byte *payload, unsigned int length) {
    StaticJsonDocument<512> jsonDoc;
    DeserializationError error = deserializeJson(jsonDoc, payload, length);
    if (error) {
        return; // Handle parsing error
    }

    // Manual controls
    bool pump = jsonDoc["pump"] | false;
    bool fan = jsonDoc["fan"] | false;
    unsigned int duration = jsonDoc["duration"] | 0;

    if (pump) {
        digitalWrite(PUMP_PIN, HIGH);
        pumpStartTime = millis();
        pumpDuration = duration;
        isPumpOn = true;
    }
    if (fan) {
        digitalWrite(FAN_PIN, HIGH);
        fanStartTime = millis();
        fanDuration = duration;
        isFanOn = true;
    }

    // Parse and schedule tasks
    const char* scheduleStr = jsonDoc["schedule"] | "";
    if (strlen(scheduleStr) > 0) {
        char scheduleCopy[128];
        strncpy(scheduleCopy, scheduleStr, sizeof(scheduleCopy));
        char* token = strtok(scheduleCopy, "&");
        while (token != nullptr) {
            char timeStr[6];
            unsigned int schedDuration;
            bool isPumpTask = strstr(token, "pump") != nullptr;
            sscanf(token, "%s %u", timeStr, &schedDuration);

            time_t timestamp = parseTime(timeStr);
            if (timestamp > time(nullptr)) {
                schedules.push_back({timestamp, schedDuration, isPumpTask});
            }
            token = strtok(nullptr, "&");
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
    mqttPublishMessage(SENSOR_TOPIC, sensorData);
}

void staticBlink() {
  //digitalWrite(LED2, !digitalRead(LED2));
  publishSensorData();
}

void scheduledBlink() {
  //digitalWrite(LED3, !digitalRead(LED2));
  mqttPublishKeepAlive();
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
  syncTime();
}

void loop() {
  if (!MQTTClient.connected()) connectToMQTTBroker();
    MQTTClient.loop();

    // Publish sensor data at intervals
    if (millis() - lastPublishTime >= PUBLISH_INTERVAL) {
        publishSensorData();
        lastPublishTime = millis();
    }

    // Send keep-alive message at intervals
    if (millis() - lastKeepAliveTime >= KEEPALIVE_INTERVAL) {
        mqttPublishKeepAlive();
        lastKeepAliveTime = millis();
    }
    handleSchedules();
    handlePumpFan();
    delay(10);
}

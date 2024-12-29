#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <DHT.h>
#include <time.h>

// WiFi and MQTT settings remain unchanged
const char* SSID     = "Tung home"; 
const char* PASSWORD = "0963617074";
const char* MQTT_BROKER = "broker.emqx.io";
const char* SENSOR_TOPIC = "ict66/smarterra/sensors/";
const char* QUERY_TOPIC = "ict66/smarterra/commands/";
const char* KEEPALIVE_TOPIC = "ict66/smarterra/keepalive/";
const int MQTT_PORT = 1883;
const char* MQTT_USERNAME = "";
const char* MQTT_PASSWORD = "";

// NTP Server and Timezone remain unchanged
const char* NTP_SERVER = "pool.ntp.org";
const long GMT_OFFSET_SEC = 7 * 3600; // Vietnam GMT+7
const int DAYLIGHT_OFFSET_SEC = 0;

// Pin and DHT settings remain unchanged
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

const int WET_VAL = 120;  // Wet soil threshold
const int DRY_VAL = 170;  // Dry soil threshold

// Time intervals in milliseconds remain unchanged
const unsigned long PUBLISH_INTERVAL = 5000;
const unsigned long KEEPALIVE_INTERVAL = 3000;
unsigned long lastPublishTime = 0;
unsigned long lastKeepAliveTime = 0;
unsigned long pumpStartTime = 0;
unsigned long fanStartTime = 0;
unsigned long pumpDuration = 0;
unsigned long fanDuration = 0;
unsigned long realCurrentTime = 0;
bool isPumpOn = false;
bool isFanOn = false;

WiFiClient espClient;
PubSubClient MQTTClient(espClient);
DHT dht(DHT_PIN, DHTTYPE);

// SensorData structure remains unchanged
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

// ControlMessage structure remains unchanged
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

// Scheduling structure with `unsigned long`
struct Schedule {
    unsigned long timestamp;
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

// Sync time function with unsigned long
void syncTime() {
    configTime(GMT_OFFSET_SEC, DAYLIGHT_OFFSET_SEC, NTP_SERVER);

    unsigned long now = static_cast<unsigned long>(time(nullptr));
    unsigned long startAttemptTime = millis();

    while (now < 1609459200 && (millis() - startAttemptTime) < 10000) {
        delay(500);
        now = static_cast<unsigned long>(time(nullptr));
    }

    if (now < 1609459200) {
        // Handle failure
    } else {
      realCurrentTime = now;
      time_t tnow = time(nullptr);
      char *onlineMessage = "ESP8266 is online at: ";
      char *readableTime = ctime(&tnow);
      strcat(onlineMessage, readableTime);
      MQTTClient.publish(KEEPALIVE_TOPIC, onlineMessage);
      delay(100);
    }
}

// Parse datetime function with unsigned long
unsigned long parseDateTime(const char* dateTimeStr) {
    struct tm timeinfo = {0};
    const char* format = "%m/%d/%Y %H:%M";

    if (strptime(dateTimeStr, format, &timeinfo) == nullptr) {
        return 0;
    }

    unsigned long rawTime = static_cast<unsigned long>(mktime(&timeinfo));
    if (rawTime == static_cast<unsigned long>(-1)) {
        return 0;
    }

    return rawTime;
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
    char keepAliveMessage[25] = "ESP8266 is online at: ";
    char readableTime[15];
    unsigned long nowTime = realCurrentTime + millis()/1000;
    ultoa(nowTime, readableTime, 10);
    strcat(keepAliveMessage, readableTime);
    MQTTClient.publish(KEEPALIVE_TOPIC, keepAliveMessage);
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

// Handle schedules with unsigned long
void handleSchedules() {
    unsigned long now = static_cast<unsigned long>(time(nullptr));
    for (auto it = schedules.begin(); it != schedules.end();) {
        if (now >= it->timestamp && now < it->timestamp + it->duration) {
            if (it->isPump) {
                digitalWrite(PUMP_PIN, HIGH);
                pumpStartTime = millis();
                pumpDuration = it->duration;
                isPumpOn = true;
            } 
            it = schedules.erase(it);
        } else {
            ++it;
        }
    }
}

// MQTT Callback function remains unchanged, except for Schedule timestamp handling
void mqttCallback(char *topic, byte *payload, unsigned int length) {
    StaticJsonDocument<512> jsonDoc;
    DeserializationError error = deserializeJson(jsonDoc, payload, length);
    if (error) {
        return;
    }

    bool pump = jsonDoc["pump"] | false;
    bool fan = jsonDoc["fan"] | false;
    unsigned int duration = jsonDoc["duration"] | 0;

    if (pump) {
        digitalWrite(PUMP_PIN, HIGH);
        pumpStartTime = millis();
        pumpDuration = duration;
        isPumpOn = true;
    }

    const char* scheduleStr = jsonDoc["schedule"] | "";
    //ParSchedule(scheduleStr);
}

// Parsing schedule strings with unsigned long timestamps
void ParSchedule(const char* scheduleStr) {
    if (strlen(scheduleStr) > 0) {
        char scheduleCopy[128];
        strncpy(scheduleCopy, scheduleStr, sizeof(scheduleCopy));
        char* token = strtok(scheduleCopy, "&");

        while (token != nullptr) {
            unsigned long timestamp;
            unsigned int schedDuration;

            sscanf(token, "%lu %u", &timestamp, &schedDuration);

            unsigned long now = realCurrentTime + millis();
            if (timestamp > now) {
                schedules.push_back({timestamp, schedDuration});
            }

            // char printTime[7] = "Time: ";
            // char printTime2[20];
            // char printTime3[20];
            // ltoa(timestamp,printTime2,10);
            // ltoa(now,printTime3,10);
            // strcat(printTime, printTime2);
            // strcat(printTime, " : ");
            // strcat(printTime, printTime3);
            // MQTTClient.publish(KEEPALIVE_TOPIC, printTime);

            token = strtok(nullptr, "&");
        }
    }
}

// MQTT initialization
void mqttSetup() {
    connectToWiFi();
    syncTime();
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

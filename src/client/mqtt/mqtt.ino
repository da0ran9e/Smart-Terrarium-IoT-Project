#include <WiFi.h>
extern "C" {
  #include "freertos/FreeRTOS.h"
  #include "freertos/timers.h"
}
#include <AsyncMqttClient.h>
#include <ArduinoJson.h>

#define WIFI_SSID "xxxxxxxx"
#define WIFI_PASSWORD "xxxxxxxx"

#define MQTT_HOST "broker.hivemq.com"
#define MQTT_PORT 1883
#define MQTT_PUB_TEMP "ict66/smarterra/sensors/"
#define MQTT_USER ""
#define MQTT_PASSWORD ""

AsyncMqttClient mqttClient;
TimerHandle_t mqttReconnectTimer;
TimerHandle_t wifiReconnectTimer;

// Variables to hold sensor readings
float temp;
float hum;

unsigned long previousMillis = 0;   // Stores last time temperature was published
const long interval = 10000;        // Interval at which to publish sensor readings

void connectToWifi() {
    Serial.println("Connecting to Wi-Fi...");
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
}

void connectToMqtt() {
    Serial.println("Connecting to MQTT...");
    mqttClient.connect();
}

void WiFiEvent(WiFiEvent_t event) {
    Serial.printf("[WiFi-event] event: %d\n", event);
    switch (event) {
        case SYSTEM_EVENT_STA_GOT_IP:
            Serial.println("WiFi connected");
            Serial.println("IP address: ");
            Serial.println(WiFi.localIP());
            connectToMqtt();
            break;
        case SYSTEM_EVENT_STA_DISCONNECTED:
            Serial.println("WiFi lost connection");
            xTimerStop(mqttReconnectTimer, 0); // ensure we don't reconnect to MQTT while reconnecting to Wi-Fi
            xTimerStart(wifiReconnectTimer, 0);
            break;
    }
}

void onMqttConnect(bool sessionPresent) {
    Serial.println("Connected to MQTT.");
    Serial.print("Session present: ");
    Serial.println(sessionPresent);
    // pin 13
    uint16_t packetIdSub = mqttClient.subscribe("640d4c75a2b2ba23e07ecb4e/devices/640dbdb6a9e8e10e90f5e1e1", 0);

    // pin 12
    uint16_t packetIdSub1 = mqttClient.subscribe("640d4c75a2b2ba23e07ecb4e/devices/640dbdcba9e8e10e90f5e1e2", 0);

}

void onMqttDisconnect(AsyncMqttClientDisconnectReason reason) {
    Serial.println("Disconnected from MQTT.");
    if (WiFi.isConnected()) {
        xTimerStart(mqttReconnectTimer, 0);
    }
}

void onMqttMessage(char *topic, char *payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index,
                   size_t total) {
      Serial.println("tesst topic+payload:....\n");
      Serial.println(payload);
    DynamicJsonDocument doc(1024);
    deserializeJson(doc, payload);
    const char *pin = doc["pin"];
    Serial.println(pin);
    const char *status = doc["data"]["status"];
    Serial.println(status);
    if(strcmp(pin,"13") == 0){
        if (strcmp(status, "ON") == 0) {
            Serial.println("test 13");
            digitalWrite(LED_1, HIGH);
        } else {
            digitalWrite(LED_1, LOW);
        }
    }
    if(strcmp(pin,"12") == 0){
        if (strcmp(status, "ON") == 0) {
            Serial.println("test 12");
            digitalWrite(LED_2, HIGH);
        } else {
            digitalWrite(LED_2, LOW);
        }
    }
}

void onMqttSubscribe(uint16_t packetId, uint8_t qos) {
    Serial.println("Subscribe acknowledged.");
    Serial.print("  packetId: ");
    Serial.println(packetId);
    Serial.print("  qos: ");
    Serial.println(qos);
}

void onMqttUnsubscribe(uint16_t packetId) {
    Serial.println("Unsubscribe acknowledged.");
    Serial.print("  packetId: ");
    Serial.println(packetId);
}

void onMqttPublish(uint16_t packetId) {
    Serial.print("Publish acknowledged.");
    Serial.print("  packetId: ");
    Serial.println(packetId);
}

void setup() {
    Serial.begin(115200);
    Serial.println();
    pinMode(LED_1, OUTPUT);
    pinMode(LED_2, OUTPUT);
    dht.begin();

    mqttReconnectTimer = xTimerCreate(
            "mqttTimer", pdMS_TO_TICKS(2000), pdFALSE, (void *) 0,
            reinterpret_cast<TimerCallbackFunction_t>(connectToMqtt));
    wifiReconnectTimer = xTimerCreate(
            "wifiTimer", pdMS_TO_TICKS(2000), pdFALSE, (void *) 0,
            reinterpret_cast<TimerCallbackFunction_t>(connectToWifi));

    WiFi.onEvent(WiFiEvent);

    mqttClient.onConnect(onMqttConnect);
    mqttClient.onDisconnect(onMqttDisconnect);
    mqttClient.onMessage(onMqttMessage);
    mqttClient.onPublish(onMqttPublish);
    mqttClient.setServer(MQTT_HOST, MQTT_PORT);
    mqttClient.setCredentials(MQTT_USER, MQTT_PASSWORD);

    connectToWifi();
}

void loop() {
    unsigned long currentMillis = millis();
    digitalWrite(LED_1, HIGH);

    if (currentMillis - previousMillis >= interval) {
        previousMillis = currentMillis;

        DynamicJsonDocument doc(256);
        doc["device"] = "111";
        doc["pin"] = 1;
        doc["time"] = time(nullptr);
        doc["data"]["temperature"] = temp;
        doc["data"]["humidity"] = hum;

        char payload[256];
        serializeJson(doc, payload);
        uint16_t packetIdPub1 = mqttClient.publish(MQTT_PUB_TEMP, 1, true, payload);

        Serial.println(time(nullptr));
        Serial.printf("Publishing on topic %s at QoS 1, packetId: %i ", MQTT_PUB_TEMP, packetIdPub1);
        Serial.printf("Message: %s \n", payload);        
    }
}
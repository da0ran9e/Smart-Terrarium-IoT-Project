#include <AsyncMqttClient.h>
#include <ArduinoJson.h>

#define MQTT_HOST "broker.hivemq.com"
#define MQTT_PORT 1883
#define MQTT_PUB_TEMP "ict66/smarterra/sensors/"
#define MQTT_USER ""
#define MQTT_PASSWORD ""

AsyncMqttClient mqttClient;

// Variables to hold sensor readings
float temp;
float hum;

unsigned long previousMillis = 0;   // Stores last time temperature was published
const long interval = 10000;        // Interval at which to publish sensor readings

void connectToMqtt() {
    Serial.println("Connecting to MQTT...");
    mqttClient.connect();
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
            //digitalWrite(LED_1, HIGH);
        } else {
            //digitalWrite(LED_1, LOW);
        }
    }
    if(strcmp(pin,"12") == 0){
        if (strcmp(status, "ON") == 0) {
            Serial.println("test 12");
            //digitalWrite(LED_2, HIGH);
        } else {
            //digitalWrite(LED_2, LOW);
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

void MQTTSetup(){
  mqttClient.onConnect(onMqttConnect);
  mqttClient.onDisconnect(onMqttDisconnect);
  mqttClient.onMessage(onMqttMessage);
  mqttClient.onPublish(onMqttPublish);
  mqttClient.setServer(MQTT_HOST, MQTT_PORT);
  mqttClient.setCredentials(MQTT_USER, MQTT_PASSWORD);
  Serial.println("MQTT Setup successfully!");
}

void SendDataToBroker(){
  unsigned long currentMillis = millis();
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
  delay(10000);
}

#include <ESP8266WiFi.h>
#include <PubSubClient.h>

// MQTT Broker settings
const char *MQTT_BROKER = "broker.hivemq.com";
const char *MQTT_TOPIC = "ict66/smarterra/sensors/"; 
const char *MQTT_USERNAME = ""; 
const char *MQTT_PASSWORD = ""; 
const int MQTT_PORT = 1883; 

PubSubClient MQTTClient(espClient);

void mqttCallback(char *topic, byte *payload, unsigned int length);

void connectToMQTTBroker();

void mqttPublishMessage(const char *topic, const char *message);

void mqttSetup(){
    MQTTClient.setServer(MQTT_BROKER, MQTT_PORT);
    MQTTClient.setCallback(mqttCallback);
    connectToMQTTBroker();
}

void mqttPublishMessage(const char *topic, const char *message){
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
   Serial.print("Message:");
   for (unsigned int i = 0; i < length; i++) {
     Serial.print((char) payload[i]);
    }
   Serial.println();
   Serial.println("-----------------------");

   // Publish message upon successful connection
  mqttPublishMessage(MQTT_TOPIC, "Hi EMQX I'm ESP8266 ^^");
}

void connectToMQTTBroker() {
   while (!MQTTClient.connected()) {
     String client_id = "esp8266-client-" + String(WiFi.macAddress());
     Serial.printf("Connecting to MQTT Broker as %s.....\n", client_id.c_str());
     if (MQTTClient.connect(client_id.c_str(), MQTT_USERNAME, MQTT_PASSWORD)) {
        Serial.println("Connected to MQTT broker");
        MQTTClient.subscribe(MQTT_TOPIC);
        
      } else {
        Serial.print("Failed to connect to MQTT broker, rc=");
        Serial.print(MQTTClient.state());
        Serial.println(" try again in 5 seconds");
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

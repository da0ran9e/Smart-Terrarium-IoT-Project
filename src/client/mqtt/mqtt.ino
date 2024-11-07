#include <ESP8266WiFi.h>
#include <PubSubClient.h>

const char* ssid     = "Tung home"; 
const char* password = "0963617074";

// MQTT Broker settings
const char *mqtt_broker = "broker.hivemq.com"; // EMQX broker endpoint
const char *mqtt_topic = "ict66/smarterra/sensors/";   // MQTT topic
const char *mqtt_username = "emqx"; // MQTT username for authentication
const char *mqtt_password = "public"; // MQTT password for authentication
const int mqtt_port = 1883; // MQTT port (TCP)

WiFiClient espClient;
PubSubClient mqtt_client(espClient);

void connectToWiFi();

void connectToMQTTBroker();

void mqttCallback(char *topic, byte *payload, unsigned int length);

void setup() {
   Serial.begin(115200);
   connectToWiFi();
   mqtt_client.setServer(mqtt_broker, mqtt_port);
   mqtt_client.setCallback(mqttCallback);
   connectToMQTTBroker();
}
 
void connectToWiFi() {
   WiFi.begin(ssid, password);
   Serial.print("Connecting to WiFi");
   while (WiFi.status() != WL_CONNECTED) {
     delay(500);
     Serial.print(".");
    }
   Serial.println("\nConnected to the WiFi network");
}


void mqttPublishMessage(const char *topic, const char *message){
   Serial.print("Sending to topic: ");
   Serial.println(topic);
   Serial.print("Message: ");
   Serial.println(message);
   mqtt_client.publish(topic, message);
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
  mqttPublishMessage(mqtt_topic, "Hi EMQX I'm ESP8266 ^^");
}

void connectToMQTTBroker() {
   while (!mqtt_client.connected()) {
     String client_id = "esp8266-client-" + String(WiFi.macAddress());
     Serial.printf("Connecting to MQTT Broker as %s.....\n", client_id.c_str());
     if (mqtt_client.connect(client_id.c_str(), mqtt_username, mqtt_password)) {
        Serial.println("Connected to MQTT broker");
        mqtt_client.subscribe(mqtt_topic);
        
      } else {
        Serial.print("Failed to connect to MQTT broker, rc=");
        Serial.print(mqtt_client.state());
        Serial.println(" try again in 5 seconds");
        delay(5000);
      }
    }
}


void loop() {
   if (!mqtt_client.connected()) {
     connectToMQTTBroker();
   }
   mqtt_client.loop();
}

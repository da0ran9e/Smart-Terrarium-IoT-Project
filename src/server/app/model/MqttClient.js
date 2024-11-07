const mqtt = require('mqtt');

class MqttClient {
  constructor(brokerUrl, options = {}) {
    this.brokerUrl = brokerUrl;
    this.options = options;
    this.client = null;
  }

  // Connect to the MQTT broker
  connect() {
    this.client = mqtt.connect(this.brokerUrl, this.options);

    // Handle connection success
    this.client.on('connect', () => {
      console.log(`Connected to MQTT broker at ${this.brokerUrl}`);
    });

    // Handle connection failure
    this.client.on('error', (error) => {
      console.error('MQTT Connection Error:', error);
    });

    // Handle incoming messages
    this.client.on('message', (topic, message) => {
      console.log(`Received message on topic ${topic}: ${message.toString()}`);
    });
  }

  // Subscribe to a topic
  subscribe(topic) {
    if (this.client) {
      this.client.subscribe(topic, (err) => {
        if (err) {
          console.error(`Failed to subscribe to topic ${topic}:`, err);
        } else {
          console.log(`Subscribed to topic: ${topic}`);
        }
      });
    }
  }

  // Publish a message to a topic
  publish(topic, message) {
    if (this.client) {
      this.client.publish(topic, message, (err) => {
        if (err) {
          console.error(`Failed to publish message to topic ${topic}:`, err);
        } else {
          console.log(`Message published to topic ${topic}: ${message}`);
        }
      });
    }
  }

  // Disconnect from the MQTT broker
  disconnect() {
    if (this.client) {
      this.client.end(() => {
        console.log('Disconnected from MQTT broker');
      });
    }
  }
}

module.exports = MqttClient;

const mqtt = require('mqtt');

const client = mqtt.connect('mqtt://broker.hivemq.com:1883', {
  username: 'emqx',
  password: 'public',
});

// MQTT Topics
const sensorTopic = 'ict66/smarterra/sensors/';
const keepAliveTopic = 'ict66/smarterra/keepalive/';
const commandTopic = 'ict66/smarterra/commands/';

// Subscribe to sensor data and keep-alive topics
client.on('connect', () => {
  console.log('Connected to MQTT broker');
  client.subscribe([sensorTopic, keepAliveTopic], (err) => {
    if (err) {
      console.error('Failed to subscribe to topics:', err.message);
    } else {
      console.log('Subscribed to topics:', sensorTopic, keepAliveTopic);
    }
  });
});

// Handle incoming messages
client.on('message', (topic, message) => {
  if (topic === keepAliveTopic) {
    // Handle keep-alive messages
    console.log('Received keep-alive message:', message.toString());
    try {
      const parsedMessage = JSON.parse(message.toString());
      const keepAlive = new KeepAlive({
        Id: parsedMessage.Id,
        alive: parsedMessage.alive,
      });
      keepAlive.save((err) => {
        if (err) {
          console.error('Error saving keep-alive message:', err);
        } else {
          console.log('Keep-alive status saved to DB');
        }
      });
    } catch (error) {
      console.error('Error processing keep-alive message:', error);
    }
  } else if (topic === sensorTopic) {
    // Handle sensor data messages
    console.log('Received sensor data message:', message.toString());
    try {
      const parsedMessage = JSON.parse(message.toString());
      const sensorData = new SensorData({
        Id: parsedMessage.Id,
        temperature: parsedMessage.temperature,
        humidity: parsedMessage.humidity,
        moisture: parsedMessage.moisture,
      });
      sensorData.save((err) => {
        if (err) {
          console.error('Error saving sensor data:', err);
        } else {
          console.log('Sensor data saved to DB');
        }
      });
    } catch (error) {
      console.error('Error processing sensor data message:', error);
    }
  }
});

module.exports = {
  client,
  topics: {
    sensorTopic,
    keepAliveTopic,
    commandTopic,
  },
};

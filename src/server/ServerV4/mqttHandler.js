const mqtt = require('mqtt');
const { KeepAlive, SensorData } = require('./models');

const client = mqtt.connect('mqtt://broker.hivemq.com:1883', {
  username: 'emqx',
  password: 'public',
});

// MQTT Topics
const sensorTopic = 'ict66/smarterra/sensors/';
const keepAliveTopic = 'ict66/smarterra/keepalive/';
const commandTopic = 'ict66/smarterra/commands/';

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

client.on('message', (topic, message) => {
  try {
    const parsedMessage = JSON.parse(message.toString());
    if (topic === keepAliveTopic) {
      const keepAlive = new KeepAlive(parsedMessage);
      keepAlive.save();
      console.log('Keep-alive saved:', keepAlive);
    } else if (topic === sensorTopic) {
      const sensorData = new SensorData(parsedMessage);
      sensorData.save();
      console.log('Sensor data saved:', sensorData);
    }
  } catch (error) {
    console.error('Error processing MQTT message:', error);
  }
});

module.exports = {
  client,
  topics: { sensorTopic, keepAliveTopic, commandTopic },
};

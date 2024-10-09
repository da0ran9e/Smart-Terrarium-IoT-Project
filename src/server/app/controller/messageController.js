const Message = require('../model/message');
const MqttClient = require('../model/MqttClient');

// MQTT broker
const brokerUrl = 'mqtt://broker.hivemq.com:1883';
const options = {
    clientId: 'smart_terrarium',
    clean: true,
    connectTimeout: 5000,
    username: '', 
    password: '', 
  };
const mqttClient = new MqttClient(brokerUrl, options);

const sendMessage = (req, res) => {
  const { sender, recipient, content } = req.body;
  const message = new Message(sender, recipient, content);
  console.log('Message:', message);

  // Connect to the MQTT broker
  mqttClient.connect();
  mqttClient.publish('iot/66/smarterra', JSON.stringify(message));

  setTimeout(() => {
    mqttClient.disconnect();
  }, 5000);

  res.status(200).json({
    message: 'Message sent successfully!',
    data: message,
  });
};

module.exports = { sendMessage };

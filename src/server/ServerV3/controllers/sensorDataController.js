const SensorData = require('../models/sensorData');
const { client, topics } = require('../config/mqttConfig');

// Process incoming sensor data messages
client.on('message', async (topic, message) => {
  if (topic === topics.sensorTopic) {
    try {
      const sensorData = JSON.parse(message.toString());
      const newData = new SensorData(sensorData);
      await newData.save();
      console.log('Sensor data saved:', newData);
    } catch (error) {
      console.error('Error saving sensor data:', error.message);
    }
  }
});

exports.getAllSensorData = async (req, res) => {
  const data = await SensorData.find();
  res.status(200).json(data);
};

exports.getSensorDataById = async (req, res) => {
  const data = await SensorData.findOne({ Id: req.params.id });
  if (!data) return res.status(404).json({ error: 'Data not found' });
  res.status(200).json(data);
};

exports.createSensorData = async (req, res) => {
  const newData = new SensorData(req.body);
  await newData.save();
  res.status(201).json(newData);
};

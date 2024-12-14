require('dotenv').config();
const express = require('express');
const mongoose = require('mongoose');
const mqtt = require('mqtt');

const app = express();
app.use(express.json());

// MongoDB Schemas and Models
const commandSchema = new mongoose.Schema({
  Id: { type: String, required: true },
  fan: Boolean,
  pump: Boolean,
  duration: Number,
  timestamp: { type: Date, default: Date.now },
});

const keepAliveSchema = new mongoose.Schema({
  Id: { type: String, required: true },
  alive: { type: Boolean, required: true },
  timestamp: { type: Date, default: Date.now },
});

const sensorDataSchema = new mongoose.Schema({
  Id: { type: String, required: true },
  temperature: { type: Number },
  humidity: { type: Number },
  moisture: { type: Number },
  timestamp: { type: Date, default: Date.now },
});

const Command = mongoose.model('Command', commandSchema);
const KeepAlive = mongoose.model('KeepAlive', keepAliveSchema);
const SensorData = mongoose.model('SensorData', sensorDataSchema);

// MQTT Setup
const client = mqtt.connect('mqtt://broker.hivemq.com:1883', {
  username: 'emqx',
  password: 'public',
});

const sensorTopic = 'ict66/smarterra/sensors/';
const keepAliveTopic = 'ict66/smarterra/keepalive/';
const commandTopicBase = 'ict66/smarterra/commands/';

client.on('connect', () => {
  console.log('Connected to MQTT broker');
  client.subscribe([sensorTopic, keepAliveTopic], (err) => {
    if (err) console.error('Failed to subscribe:', err.message);
  });
});

client.on('message', (topic, message) => {
  try {
    const data = JSON.parse(message.toString());
    if (topic.startsWith(sensorTopic)) {
      const sensorData = new SensorData(data);
      sensorData.save().catch((err) => console.error('Failed to save sensor data:', err));
    } else if (topic.startsWith(keepAliveTopic)) {
      const keepAlive = new KeepAlive(data);
      keepAlive.save().catch((err) => console.error('Failed to save keep-alive data:', err));
    }
  } catch (err) {
    console.error('Failed to process message:', err.message);
  }
});

// In-memory storage for online clients
const onlineClients = new Map();

// Track keep-alive messages
client.on('message', (topic, message) => {
  if (topic === keepAliveTopic) {
    try {
      const { Id, alive } = JSON.parse(message.toString());
      if (alive) {
        onlineClients.set(Id, Date.now());
      }
    } catch (err) {
      console.error('Failed to process keep-alive message:', err.message);
    }
  }
});

// Periodically clean up offline clients
setInterval(() => {
  const now = Date.now();
  for (const [clientId, lastSeen] of onlineClients.entries()) {
    if (now - lastSeen > 5000) {
      onlineClients.delete(clientId);
      console.log(`Client ${clientId} is offline`);
    }
  }
}, 5000);

// RESTful APIs
// Get all sensor data
app.get('/api/sensorData', async (req, res) => {
  const data = await SensorData.find().sort({ timestamp: -1 });
  res.json(data);
});

// Get sensor data by ID
app.get('/api/sensorData/:id', async (req, res) => {
  const data = await SensorData.findById(req.params.id);
  if (!data) return res.status(404).json({ error: 'Sensor data not found' });
  res.json(data);
});

// Get all commands
app.get('/api/commands', async (req, res) => {
  const commands = await Command.find().sort({ timestamp: -1 });
  res.json(commands);
});

// Send a command
app.post('/api/commands', async (req, res) => {
  const { Id, pump, fan, duration } = req.body;
  const command = new Command({ Id, pump, fan, duration });
  await command.save();

  const topic = `${commandTopicBase}${Id}`;
  client.publish(topic, JSON.stringify({ pump, fan, duration }), (err) => {
    if (err) return res.status(500).json({ error: 'Failed to send command' });
    res.status(201).json(command);
  });
});

// Check if a client is online
app.get('/api/clients/:id/online', (req, res) => {
  const isOnline = onlineClients.has(req.params.id);
  res.json({ clientId: req.params.id, online: isOnline });
});

// Connect to MongoDB and Start Server
mongoose
  .connect(process.env.MONGO_URI, { useNewUrlParser: true, useUnifiedTopology: true })
  .then(() => {
    console.log('Connected to MongoDB');
    const PORT = process.env.PORT || 3000;
    app.listen(PORT, () => console.log(`Server running at http://localhost:${PORT}`));
  })
  .catch((err) => {
    console.error('Failed to connect to MongoDB:', err.message);
    process.exit(1);
  });

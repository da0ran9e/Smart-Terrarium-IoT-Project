const mongoose = require('mongoose');

// Command Schema
const commandSchema = new mongoose.Schema({
  Id: { type: Number, required: true },
  fan: Boolean,
  pump: Boolean,
  duration: Number,
  timestamp: { type: Date, default: Date.now },
});

// KeepAlive Schema
const keepAliveSchema = new mongoose.Schema({
  Id: { type: Number, required: true },
  alive: { type: Boolean, required: true },
  timestamp: { type: Date, default: Date.now },
});

// SensorData Schema
const sensorDataSchema = new mongoose.Schema({
  Id: { type: Number, required: true },
  temperature: { type: Number },
  humidity: { type: Number },
  moisture: { type: Number },
  timestamp: { type: Date, default: Date.now },
});

module.exports = {
  Command: mongoose.model('Command', commandSchema),
  KeepAlive: mongoose.model('KeepAlive', keepAliveSchema),
  SensorData: mongoose.model('SensorData', sensorDataSchema),
};

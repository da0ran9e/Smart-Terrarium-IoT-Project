const mongoose = require('mongoose');

const sensorDataSchema = new mongoose.Schema({
  Id: { type: Number, required: true },
  temperature: { type: Number },
  humidity: { type: Number },
  moisture: { type: Number },
  timestamp: { type: Date, default: Date.now },
});

module.exports = mongoose.model('SensorData', sensorDataSchema);

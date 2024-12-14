// models/keepAlive.js
const mongoose = require('mongoose');

const keepAliveSchema = new mongoose.Schema({
  Id: { type: Number, required: true },
  alive: { type: Boolean, required: true },
  timestamp: { type: Date, default: Date.now },
});

module.exports = mongoose.model('KeepAlive', keepAliveSchema);

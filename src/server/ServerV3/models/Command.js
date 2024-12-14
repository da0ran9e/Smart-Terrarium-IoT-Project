const mongoose = require('mongoose');

const commandSchema = new mongoose.Schema({
  Id: { type: Number, required: true },
  fan: Boolean,
  pump: Boolean,
  duration: Number,
  timestamp: { type: Date, default: Date.now },
});

module.exports = mongoose.model('Command', commandSchema);

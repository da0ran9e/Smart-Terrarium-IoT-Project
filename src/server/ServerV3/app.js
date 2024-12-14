const express = require('express');
const mongoose = require('mongoose');
const sensorDataRoutes = require('./routes/sensorDataRoutes');
const commandRoutes = require('./routes/commandRoutes');

require('dotenv').config();

const app = express();

app.use(express.json());
app.use('/api/SensorDatas', sensorDataRoutes);
app.use('/api/Commands', commandRoutes);

mongoose.connect(process.env.MONGO_URI, {
  useNewUrlParser: true,
  useUnifiedTopology: true,
});

module.exports = app;

const express = require('express');
const {
  getAllSensorData,
  getSensorDataById,
  createSensorData,
} = require('../controllers/sensorDataController');

const router = express.Router();

router.get('/', getAllSensorData);
router.post('/', createSensorData);
router.get('/:id', getSensorDataById);

module.exports = router;

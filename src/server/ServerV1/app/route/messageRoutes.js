const express = require('express');
const { sendMessage } = require('../controller/messageController');
const router = express.Router();

router.post('/send-message', sendMessage);

module.exports = router;

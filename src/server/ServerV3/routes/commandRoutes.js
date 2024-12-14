const express = require('express');
const {
  getAllCommands,
  getCommandById,
  createCommand,
  updateCommandById,
  deleteCommandById,
} = require('../controllers/commandController');

const router = express.Router();

router.get('/', getAllCommands);
router.post('/', createCommand);
router.get('/:id', getCommandById);
router.put('/:id', updateCommandById);
router.delete('/:id', deleteCommandById);

module.exports = router;

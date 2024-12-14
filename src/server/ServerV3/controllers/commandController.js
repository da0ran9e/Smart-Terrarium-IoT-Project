const Command = require('../models/command');
const { client, topics } = require('../config/mqttConfig');

exports.getAllCommands = async (req, res) => {
  const commands = await Command.find();
  res.status(200).json(commands);
};

exports.getCommandById = async (req, res) => {
  const command = await Command.findOne({ Id: req.params.id });
  if (!command) return res.status(404).json({ error: 'Command not found' });
  res.status(200).json(command);
};

exports.createCommand = async (req, res) => {
  try {
    const newCommand = new Command(req.body);
    await newCommand.save();

    // Publish the command to the commandTopic
    const commandMessage = JSON.stringify(req.body);
    client.publish(topics.commandTopic, commandMessage, (err) => {
      if (err) {
        console.error('Error publishing command:', err.message);
        return res.status(500).json({ error: 'Failed to send command to circuit' });
      }
      console.log('Command published:', commandMessage);
    });

    res.status(201).json(newCommand);
  } catch (error) {
    res.status(400).json({ error: error.message });
  }
};

exports.updateCommandById = async (req, res) => {
  const updatedCommand = await Command.findOneAndUpdate(
    { Id: req.params.id },
    req.body,
    { new: true }
  );
  if (!updatedCommand) return res.status(404).json({ error: 'Command not found' });
  res.status(200).json(updatedCommand);
};

exports.deleteCommandById = async (req, res) => {
  const result = await Command.findOneAndDelete({ Id: req.params.id });
  if (!result) return res.status(404).json({ error: 'Command not found' });
  res.status(200).json({ message: 'Command deleted' });
};

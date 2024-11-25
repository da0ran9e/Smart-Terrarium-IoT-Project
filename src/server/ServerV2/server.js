const mqtt = require('mqtt');
const fs = require('fs');
const express = require('express');

const app = express();
const port = 3000;

const getDataAPI = '/data/:client_id';
const sendCommandAPI = '/command';
const getClientStatus = '/status/:client_id';

const brokerUrl = 'mqtt://broker.emqx.io:1883';
const keepaliveTopic = 'ict66/smarterra/keepalive/';
const sensorDataTopic = 'ict66/smarterra/sensors/';
const commandTopic = 'ict66/smarterra/commands/';
const clientId = `mqtt_dee99bc42b9f`; // temp

const clientActiveTime = 5000;
const dataSamples = 100;

let onlineClients = [];
const clientStatus = {};

// Connect to MQTT broker
const client = mqtt.connect(brokerUrl, { clientId });

// Initialize local data file
const dataFile = './sensor_data.json';
if (!fs.existsSync(dataFile)) {
  fs.writeFileSync(dataFile, JSON.stringify([]));
}

// Connected to MQTT broker
client.on('connect', () => {
    console.log(`Connected to MQTT broker as ${clientId}`);
    const topics = [keepaliveTopic, sensorDataTopic];
    client.subscribe(topics, (err, granted) => {
        if (err) {
          console.error(`Failed to subscribe to topics: ${err.message}`);
        } else {
          console.log(`Subscribed to topics: ${granted.map(grant => grant.topic).join(', ')}`);
        }
    });
});

// Handle messages
client.on('message', (topic, message) => {
    if (topic === keepaliveTopic) {
        try {
            const payload = JSON.parse(message.toString());
            const { client_id, alive } = payload;
      
            if (alive) {
              if (!onlineClients.includes(client_id)) {
                onlineClients.push(client_id);
                console.log(`Client ${client_id} is now online.`);
                clientStatus[client_id] = Date.now();
              }
            }
          } catch (err) {
            console.error(`Failed to process keepalive message: ${err.message}`);
        }
    }
  
    if (topic === sensorDataTopic) {
        try {
            const payload = JSON.parse(message.toString());
            const timestamp = new Date().toISOString();
            const data = { ...payload, client_id: clientId, timestamp };

            console.log(`Received message:`, data);
        
            let existingData = [];
            if (fs.existsSync(dataFile)) {
              existingData = JSON.parse(fs.readFileSync(dataFile));
            }
        
            existingData.push(data);
        
            const trimmedData = existingData.slice(-1000);// Save only the latest 1000 entries to avoid file bloat
            fs.writeFileSync(dataFile, JSON.stringify(trimmedData, null, 2));
          } catch (err) {
            console.error('Failed to process message:', err.message);
          }
    }
});

// Periodic check for offline clients (remove clients after 5 seconds of inactivity)
setInterval(() => {
    const now = Date.now();
    onlineClients = onlineClients.filter((client_id) => {
      const lastActiveTime = clientStatus[client_id];
      if (now - lastActiveTime > clientActiveTime) {
        console.log(`Client ${client_id} is offline within ${clientActiveTime}ms.`);
        return false;
      }
      return true;
    });
  }, 1000);

// Error
client.on('error', (err) => {
  console.error(`MQTT Error: ${err.message}`);
});

app.get(getDataAPI, (req, res) => {
  const clientId = req.params.client_id;
  const data = JSON.parse(fs.readFileSync(dataFile));
  const filteredData = data.filter((entry) => entry.client_id === clientId).slice(-dataSamples);

  if (filteredData.length > 0) {
    res.json(filteredData);
  } else {
    res.status(404).json({ error: 'No data found for the specified client_id' });
  }
});

app.post(sendCommandAPI, (req, res) => {
    const { client_id, type, duration } = req.query; // Use req.query for query parameters
    if (!client_id) {
      return res.status(400).json({ error: `'client_id' is required` });
    }
    if (!['pump', 'fan'].includes(type)) {
      return res.status(400).json({ error: `'type' must be either 'pump' or 'fan'` });
    }
    if (isNaN(duration) || duration <= 0) {
      return res.status(400).json({ error: `'duration' must be a positive number` });
    }
    const command = { [type]: true, duration: Number(duration) };
    //const commandTopic = `ict66/smarterra/commands/${client_id}`;
    client.publish(commandTopic, JSON.stringify(command), (err) => {
      if (err) {
        console.error(`Failed to publish command: ${err.message}`);
        return res.status(500).json({ error: 'Failed to send command to MQTT broker' });
      }

      console.log(`Command published to ${commandTopic}:`, command);
      res.json({ success: true, message: `Command sent to ${commandTopic}`, command });
    });
});


app.get(getClientStatus, (req, res) => {
  const { client_id } = req.params;
  if (onlineClients.includes(client_id)) {
    return res.json({ client_id, online: true });
  } else {
    return res.json({ client_id, online: false });
  }
});

// Start the Express server
app.listen(port, () => {
  console.log(`HTTP server running at http://localhost:${port}`);
});

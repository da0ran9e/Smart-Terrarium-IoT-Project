# Create project
```bash
npm init -y
```
# Create `server.js` 
# Install packages
```bash
npm install express
npm install mqtt
npm install mqtt express fs
```
# Run server
```bash
node server.js
```
# Test
1. Keepalive:
- topic: ict66/smarterra/keepalive/
- Example message: 
```json
{
    "client_id": "mqtt_dee99bc42b9f",
    "alive": true
}
```
- API: `http://localhost:3000/status/mqtt_dee99bc42b9f`
2. Commands
- topic: ict66/smarterra/commands/
- API: `http://localhost:3000/command?client_id=mqtt_dee99bc42b9f&type=pump&duration=10`
3. Sensor data:
- topic: ict66/smarterra/sensors/
- Example: 
```json
{"temperature":84.02,"humidity":62,"moisture":329}
```
- API: `http://localhost:3000/data/mqtt_dee99bc42b9f`
- Expected Response (JSON):
```json
[
  {
    "temperature": 84.02,
    "humidity": 62,
    "moisture": 329,
    "client_id": "mqtt_dee99bc42b9f",
    "timestamp": "2024-11-25T10:00:00.000Z"
  },
  ...
]
```
# SmartTerra Server
This is the server for the SmartTerra project. It is a RESTful API that provides access to the data stored in the database. The server is built using Node.js and Express.js.

## MQTT Broker Information
See `config` folder.

## Installation
1. Clone the repository.
2. Run `npm install` to install the dependencies.
3. Run `npm start` to start the server.


## Models

### 1. **SensorData Model**
The `SensorData` model represents the data collected from the sensors in the garden, such as temperature, humidity, and moisture levels.

```javascript
const sensorDataSchema = new mongoose.Schema({ 
  Id: { type: Number, required: true },
  temperature: { type: Number },
  humidity: { type: Number },
  moisture: { type: Number },
  timestamp: { type: Date, default: Date.now }
});
```

#### Fields:
- **Id**: The unique identifier for each circuit (integer).
- **temperature**: Temperature value from the sensor (float).
- **humidity**: Humidity value from the sensor (float).
- **moisture**: Moisture level from the sensor (float).
- **timestamp**: The date and time when the data was recorded (Date).

### 2. **Command Model**
The `Command` model represents the commands sent to the actuators (fan and pump).

```javascript
const commandSchema = new mongoose.Schema({
  Id: { type: Number, required: true },
  fan: Boolean,
  pump: Boolean,
  duration: Number,
  timestamp: { type: Date, default: Date.now }
});
```

#### Fields:
- **Id**: The unique identifier for each circuit (integer).
- **fan**: A boolean indicating whether the fan should be on (`true`) or off (`false`).
- **pump**: A boolean indicating whether the pump should be on (`true`) or off (`false`).
- **duration**: The duration for which the command should be executed (in seconds).
- **timestamp**: The date and time when the command was issued (Date).

---

## API Endpoints

### 1. **SensorDatas Endpoints**

#### `GET /api/SensorDatas`
- **Description**: Retrieve all sensor data records.
- **Response Example**:
```json
[
  {
    "Id": 1,
    "temperature": 25.5,
    "humidity": 60.0,
    "moisture": 80.0,
    "timestamp": "2024-12-03T12:00:00Z"
  },
  {
    "Id": 2,
    "temperature": 22.0,
    "humidity": 55.0,
    "moisture": 75.0,
    "timestamp": "2024-12-03T12:05:00Z"
  }
]
```

#### `POST /api/SensorDatas`
- **Description**: Submit new sensor data to the system.
- **Request Example**:
```json
{
  "Id": 3,
  "temperature": 26.2,
  "humidity": 63.0,
  "moisture": 85.0
}
```
- **Response Example**:
```json
{
  "Id": 3,
  "temperature": 26.2,
  "humidity": 63.0,
  "moisture": 85.0,
  "timestamp": "2024-12-03T12:10:00Z"
}
```

#### `GET /api/SensorDatas/{id}`
- **Description**: Retrieve a specific sensor data record by its circuit `Id`.
- **Response Example**:
```json
{
  "Id": 1,
  "temperature": 25.5,
  "humidity": 60.0,
  "moisture": 80.0,
  "timestamp": "2024-12-03T12:00:00Z"
}
```

### 2. **Commands Endpoints**

#### `GET /api/Commands`
- **Description**: Retrieve all command records.
- **Response Example**:
```json
[
  {
    "Id": 1,
    "fan": true,
    "pump": false,
    "duration": 120,
    "timestamp": "2024-12-03T12:00:00Z"
  }
]
```

#### `POST /api/Commands`
- **Description**: Submit a new command to control the actuators (fan and pump).
- **Request Example**:
```json
{
  "Id": 2,
  "fan": true,
  "pump": true,
  "duration": 60
}
```
- **Response Example**:
```json
{
  "Id": 2,
  "fan": true,
  "pump": true,
  "duration": 60,
  "timestamp": "2024-12-03T12:15:00Z"
}
```

#### `GET /api/Commands/{id}`
- **Description**: Retrieve a specific command record by its `Id`.
- **Response Example**:
```json
{
  "Id": 1,
  "fan": true,
  "pump": false,
  "duration": 120,
  "timestamp": "2024-12-03T12:00:00Z"
}
```

#### `PUT /api/Commands/{id}`
- **Description**: Update an existing command by `Id`.
- **Request Example**:
```json
{
  "Id": 1,
  "fan": false,
  "pump": true,
  "duration": 180
}
```
- **Response Example**:
```json
{
  "Id": 1,
  "fan": false,
  "pump": true,
  "duration": 180,
  "timestamp": "2024-12-03T12:20:00Z"
}
```

#### `DELETE /api/Commands/{id}`
- **Description**: Delete a specific command record by `Id`.
- **Response Example**:
```json
{
  "message": "Command with Id 1 has been deleted successfully"
}
```

---

## MQTT Topics

- **Sensor Data**: `ict66/smarterra/sensors/`
  - Contains sensor readings: temperature, humidity, and moisture.
- **Command Data**: `ict66/smarterra/commands/`
  - Used to send commands to the actuators (fan, pump).
- **Keep-Alive**: `ict66/smarterra/keepalive/`
  - Used to check the circuit's status (alive or not).

---

## How to Run the App

### 1. Clone the Repository
```bash
git clone https://github.com/your-username/your-repository.git
cd your-repository
```

### 2. Install Dependencies
Make sure you have `Node.js` and `npm` installed. Then run:
```bash
npm install
```

### 3. Set Up Environment Variables
Install the `.env` file in the root directory of the project by:
```bash
npm install dotenv
```

### 4. Run the App
```bash
npm start
```

This will start the server on `http://localhost:3000`.

---

## Notes
- Ensure that the MongoDB server is running and accessible from your application. Add your IP address to the allowed list in the MongoDB Atlas dashboard.
- The app subscribes to the topics `ict66/smarterra/sensors/`, `ict66/smarterra/commands/`, and `ict66/smarterra/keepalive/` to handle sensor data, commands, and keep-alive messages.
- We work with only **one** circuit for now, so the default `Id` is set to `1`. You can modify the code to handle multiple circuits if needed.

---

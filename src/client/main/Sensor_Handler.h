#include "DHT.h"

#define DHTTYPE DHT11
#define DHT_PIN 1
#define SENSOR_PIN A0  
#define PUMP_PIN 2

const int WET_VAL = 500;  // Minimum value when soil is wet
const int DRY_VAL = 714;  // Maximum value when soil is dry

DHT dht(DHT_PIN, DHTTYPE);

typedef struct DHTData {
    float humidity = 0;
    float temperature = 0;
} DHTData;

void DHTSetup() {
    dht.begin();
    Serial.println("DHT11 Setup successfully!");
    pinMode(PUMP_PIN, OUTPUT);
    digitalWrite(PUMP_PIN, LOW);
    Serial.println("Pump pin Output Setup successfully!");
    delay(100);
}

DHTData GetDHTData(bool celsiusTemp = true) {
    DHTData data;
    float h = dht.readHumidity();
    float t = dht.readTemperature();
    float f = dht.readTemperature(true);

    if (isnan(h) || isnan(t) || isnan(f)) {
        Serial.println("Failed to get DHT data!");
    } else {
        data.humidity = h;
        if (celsiusTemp) {
            data.temperature = dht.computeHeatIndex(t, h, false);  // Celsius heat index
        } else {
            data.temperature = dht.computeHeatIndex(f, h);  // Fahrenheit heat index
        }
    }
    return data;
}

void PrintDHTData() {
    float h = dht.readHumidity();
    float t = dht.readTemperature();
    float f = dht.readTemperature(true);

    if (isnan(h) || isnan(t) || isnan(f)) {
        Serial.println("Failed to get DHT data!");
    } else {
        Serial.print("Humidity: ");
        Serial.println(h);
        Serial.print("Temperature (C): ");
        Serial.println(dht.computeHeatIndex(t, h, false));
        Serial.print("Temperature (F): ");
        Serial.println(dht.computeHeatIndex(f, h));
    }
}

int getMoisture() {
    int sensorVal = analogRead(SENSOR_PIN);

    // Return mapped moisture percentage if in range, otherwise -1
    if (sensorVal >= WET_VAL && sensorVal <= DRY_VAL) {
        return map(sensorVal, DRY_VAL, WET_VAL, 0, 100);
    } else {
        return -1;
    }
}

void PrintAnalog() {
    int sensorVal = analogRead(SENSOR_PIN);
    Serial.print("Soil Moisture Sensor Value: ");
    Serial.println(sensorVal);
}
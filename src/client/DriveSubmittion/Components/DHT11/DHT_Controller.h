#include <DHT.h>

#define DHTTYPE DHT11
#define DHT_PIN 1
DHT dht(DHT_PIN, DHTTYPE);

typedef struct DHTData{
    float humidity = 0;
    float temperature = 0;
} DHTData;

void DHTSetup(){
    dht.begin();
    Serial.println("DHT11 Setup successfully!");
    delay(100);
}

DHTData GetDHTData(bool celsiusTemp = true){
    DHTData data;
    float h = dht.readHumidity();
    float t = dht.readTemperature();
    float f = dht.readTemperature(true);
    if (isnan(h) || isnan(t) || isnan(f))
    {
        Serial.println("Fail getting DHT data!");
    }
    else{
        data.humidity = h;
        if(celsiusTemp){
            data.temperature = dht.computeHeatIndex(t, h, false);
        } else {
            data.temperature = dht.computeHeatIndex(f, h);
        }
    }

    return data;
}

void PrintDHTData(){
    float h = dht.readHumidity();
    float t = dht.readTemperature();
    float f = dht.readTemperature(true);
    if (isnan(h) || isnan(t) || isnan(f))
    {
        Serial.println("Fail getting DHT data!");
    }
    else{
        Serial.println(h);
        Serial.println(dht.computeHeatIndex(t, h, false));
        Serial.println(dht.computeHeatIndex(f, h));
    }
    
}
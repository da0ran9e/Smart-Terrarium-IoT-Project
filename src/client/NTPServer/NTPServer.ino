#include <ESP8266WiFi.h>
#include <time.h>

// Wi-Fi credentials
const char* SSID = "oản";
const char* PASSWORD = "1234567tam";

// NTP server settings
const char* NTP_SERVER = "pool.ntp.org"; // You can replace this with a regional server like "asia.pool.ntp.org"
const long GMT_OFFSET_SEC = 0;          // Adjust to your timezone (e.g., -18000 for GMT-5)
const int DAYLIGHT_OFFSET_SEC = 0;      // Daylight saving time offset

void connectToWiFi() {
  Serial.print("Connecting to Wi-Fi");
  WiFi.begin(SSID, PASSWORD);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nConnected to Wi-Fi!");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
}

void syncTime() {
  Serial.println("Synchronizing time...");
  configTime(GMT_OFFSET_SEC, DAYLIGHT_OFFSET_SEC, NTP_SERVER);

  time_t now = time(nullptr);
  unsigned long startAttemptTime = millis();

  // Wait for time to sync (timeout after 10 seconds)
  while (now < 1609459200 && (millis() - startAttemptTime) < 10000) {
    delay(500);
    now = time(nullptr);
  }

  if (now < 1609459200) {
    Serial.println("Failed to sync time with NTP server.");
  } else {
    Serial.println("Time synchronized successfully!");
    Serial.print("Current time: ");
    Serial.println(ctime(&now));
  }
}

void setup() {
  Serial.begin(115200);
  connectToWiFi();
  syncTime();
}

void loop() {
  // Display current time every 5 seconds
  delay(5000);

  time_t now = time(nullptr);
  Serial.print("Current time: ");
  Serial.println(ctime(&now));
}

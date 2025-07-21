#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <SoftwareSerial.h>


// WiFi credentials
const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";

// MQTT broker details
const char* mqtt_server = "YOUR_MQTT_BROKER_IP";
const int mqtt_port = 1883;
const char* mqtt_user = "MQTT_USER";     // optional
const char* mqtt_pass = "MQTT_PASSWORD"; // optional

// MQTT topics
const char* topic_total_energy = "power/total_energy_kWh";
const char* topic_power_now = "power/current_consumption_W";
const char* topic_feed_in = "power/feed_in_W";

// Relay pin
const int relayPin = D1; // GPIO5

// Feed-in threshold
const int feedInThreshold = 1400;

// SML over SoftwareSerial (IR head)
SoftwareSerial smlSerial(D7, D6); // RX, TX (we only use RX)

// WiFi and MQTT clients
WiFiClient espClient;
PubSubClient client(espClient);

unsigned long lastMsg = 0;
const long interval = 10000; // send every 10 seconds

// Mock function to simulate SML data parsing
// Replace with actual parser logic
struct SmlData {
  float total_energy_kWh;
  float current_power_W;
  float feed_in_W;
};

SmlData readSmlData() {
  // Replace with actual SML parsing from smlSerial
  SmlData data;
  data.total_energy_kWh = 12345.67;
  data.current_power_W = 567.89;
  data.feed_in_W = -1600.00; // negative = feeding into the grid
  return data;
}

void setup_wifi() {
  delay(10);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }
}

void reconnect_mqtt() {
  while (!client.connected()) {
    if (client.connect("ESP8266Client", mqtt_user, mqtt_pass)) {
      // Connected
    } else {
      delay(2000);
    }
  }
}

void setup() {
  pinMode(relayPin, OUTPUT);
  digitalWrite(relayPin, LOW); // heating off

  Serial.begin(115200);
  smlSerial.begin(9600); // depends on your meter

  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);
}

void loop() {
  if (!client.connected()) {
    reconnect_mqtt();
  }
  client.loop();

  unsigned long now = millis();
  if (now - lastMsg > interval) {
    lastMsg = now;

    SmlData data = readSmlData(); // Replace with actual parsing

    // Convert to strings
    char energyStr[16];
    char powerStr[16];
    char feedInStr[16];

    dtostrf(data.total_energy_kWh, 8, 2, energyStr);
    dtostrf(data.current_power_W, 8, 2, powerStr);
    dtostrf(data.feed_in_W, 8, 2, feedInStr);

    client.publish(topic_total_energy, energyStr);
    client.publish(topic_power_now, powerStr);
    client.publish(topic_feed_in, feedInStr);

    // Relay logic
    if (data.feed_in_W < -feedInThreshold) {
      digitalWrite(relayPin, HIGH); // turn ON heating
    } else {
      digitalWrite(relayPin, LOW);  // turn OFF heating
    }

    // Debug
    Serial.print("Total Energy: "); Serial.print(energyStr); Serial.println(" kWh");
    Serial.print("Current Power: "); Serial.print(powerStr); Serial.println(" W");
    Serial.print("Feed-in Power: "); Serial.print(feedInStr); Serial.println(" W");
    Serial.println("----------------------------");
  }
}

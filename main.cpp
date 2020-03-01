#include <Arduino.h>
#include <SimpleDHT.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

const char* ssid = "wifi";
const char* password = "123456";
const char* mqtt_server = "broker.hivemq.com";

WiFiClient espClient;
PubSubClient client(espClient);

int pinDHT11 = 13;
SimpleDHT11 dht11(pinDHT11);

long lastMsg = 0;
char msg[10];
int value = 0;

void setup_wifi() {
  delay(10);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }
  randomSeed(micros());
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    // Create a random client ID
    String clientId = "ESP8266Client-dth11-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      client.publish("/iot/dht11", "ready");
    } else {
      delay(5000);
    }
  }
}

void setup() {
  pinMode(2, OUTPUT);     // Initialize the BUILTIN_LED pin as an output
  Serial.begin(9600);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  long now = millis();
  if (now - lastMsg > 5000) {
    lastMsg = now;
    ++value;
    byte temperature = 0;
    byte humidity = 0;
    int err = SimpleDHTErrSuccess;
    if ((err = dht11.read(&temperature, &humidity, NULL)) != SimpleDHTErrSuccess) {
      Serial.print("Read DHT11 failed, err="); Serial.println(err);delay(1000);
      return;
    }
    int t = (int)temperature;
    int h = (int)humidity;
    snprintf (msg, 10, "%ld", t);
    client.publish("/iot/dht11/tem", msg);
    snprintf (msg, 10, "%ld", h);
    client.publish("/iot/dht11/hum", msg);
  }
}

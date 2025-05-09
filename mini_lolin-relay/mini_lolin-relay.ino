#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <PubSubClient.h>
#include <EEPROM.h>
#include <Ticker.h>
#include <FS.h>
#include <ESP8266mDNS.h>
#include <ESP8266HTTPUpdateServer.h>

#define RELAY_PIN D1
#define BUTTON_PIN D3
#define EEPROM_ADDR 0

const char* ssid = "PLT-DIR";
const char* password = "Epaminondas";
IPAddress local_ip(10, 0, 0, 52);
IPAddress gateway(10, 0, 0, 1);
IPAddress subnet(255, 255, 255, 0);

const char* mqtt_server = "10.0.0.141";
const char* mqtt_user = "MQTT";
const char* mqtt_pass = "planeta";
const char* mqtt_topic_cmd = "rele/comando";
const char* mqtt_topic_status = "ESP8266/52/status";

WiFiClient espClient;
PubSubClient client(espClient);
ESP8266WebServer server(80);
ESP8266HTTPUpdateServer httpUpdater;
Ticker watchdogTicker;

bool releState = false;
bool lastButtonState = HIGH;
unsigned long lastDebounceTime = 0;
unsigned long lastPublish = 0;
const unsigned long publishInterval = 60000;

void saveState(bool state) {
  EEPROM.write(EEPROM_ADDR, state);
  EEPROM.commit();
}

bool readState() {
  return EEPROM.read(EEPROM_ADDR);
}

void publishStatus() {
  client.publish(mqtt_topic_status, releState ? "ON" : "OFF", true);
}

void applyRelayState(bool forcePublish = false) {
  digitalWrite(RELAY_PIN, releState ? HIGH : LOW);
  if (forcePublish) publishStatus();
}

void checkButton() {
  bool reading = digitalRead(BUTTON_PIN);
  if (reading != lastButtonState) lastDebounceTime = millis();
  if ((millis() - lastDebounceTime) > 50 && reading == LOW) {
    releState = !releState;
    applyRelayState(true);
    saveState(releState);
    lastDebounceTime = millis();
  }
  lastButtonState = reading;
}

void callback(char* topic, byte* payload, unsigned int length) {
  String msg = "";
  for (unsigned int i = 0; i < length; i++) msg += (char)payload[i];
  msg.toUpperCase();
  if (msg == "ON" && !releState) {
    releState = true;
    applyRelayState(true);
    saveState(releState);
  } else if (msg == "OFF" && releState) {
    releState = false;
    applyRelayState(true);
    saveState(releState);
  }
}

void reconnectMQTT() {
  while (!client.connected()) {
    if (client.connect("releClient", mqtt_user, mqtt_pass)) {
      client.subscribe(mqtt_topic_cmd);
      publishStatus();
    } else {
      delay(5000);
    }
  }
}

void resetWatchdog() {
  ESP.wdtFeed();
}

void handleStatusJson() {
  String json = "{";
  json += "\"ip\":\"" + WiFi.localIP().toString() + "\",";
  json += "\"rele\":\"" + String(releState ? "Ligado" : "Desligado") + "\",";
  json += "\"uptime\":" + String(millis() / 1000) + ",";
  json += "\"rssi\":" + String(WiFi.RSSI());
  json += "}";
  server.send(200, "application/json", json);
}

void handleToggle() {
  releState = !releState;
  applyRelayState(true);
  saveState(releState);
  server.send(200, "text/plain", "OK");
}

void setup() {
  pinMode(RELAY_PIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  EEPROM.begin(512);
  releState = readState();
  applyRelayState();

  Serial.begin(115200);
  WiFi.config(local_ip, gateway, subnet);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) delay(500);

  SPIFFS.begin();
  server.serveStatic("/", SPIFFS, "/index.html");
  server.serveStatic("/style.css", SPIFFS, "/style.css");
  server.serveStatic("/script.js", SPIFFS, "/script.js");
  server.on("/toggle", HTTP_POST, handleToggle);
  server.on("/status.json", HTTP_GET, handleStatusJson);

  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);

  ESP.wdtEnable(WDTO_8S);
  watchdogTicker.attach(2, resetWatchdog);

  MDNS.begin("esp8266");
  httpUpdater.setup(&server);
  server.begin();

  lastPublish = millis();
}

void loop() {
  if (!client.connected()) reconnectMQTT();
  client.loop();
  server.handleClient();
  checkButton();
  ESP.wdtFeed();
  if (millis() - lastPublish > publishInterval) {
    publishStatus();
    lastPublish = millis();
  }
}

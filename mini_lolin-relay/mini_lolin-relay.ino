/**************************************************************************
 Placa de desenvolvimento Wemos D1 Mini Lolin
 Acionamento de um módulo relé 
 Necessita de shield relé para Wemos D1 Mini
 Maio de 2025
 Epaminondas de Souza Lage
 **************************************************************************/

// === Inclusão de bibliotecas necessárias ===
#include <ESP8266WiFi.h>                // Conexão Wi-Fi
#include <ESP8266WebServer.h>          // Servidor HTTP embutido
#include <PubSubClient.h>              // Cliente MQTT
#include <EEPROM.h>                    // Memória EEPROM para persistência de dados
#include <Ticker.h>                    // Temporizador para Watchdog
#include <FS.h>                        // SPIFFS para arquivos da interface web
#include <ESP8266mDNS.h>               // Acesso por nome local ex: http://esp8266.local
#include <ESP8266HTTPUpdateServer.h>   // Atualização OTA via navegador

// === Definições de pinos e parâmetros ===
#define RELAY_PIN D1           // Pino digital conectado ao módulo relé
#define BUTTON_PIN D3          // Pino digital conectado ao botão físico
#define EEPROM_ADDR 0          // Endereço EEPROM onde o estado do relé será salvo

// === Credenciais Wi-Fi e rede estática ===
const char* ssid = "PLT-DIR";
const char* password = "epaminondas";
IPAddress local_ip(10, 0, 2, 52);
IPAddress gateway(10, 0, 2, 1);
IPAddress subnet(255, 255, 255, 0);

// === Configurações MQTT ===
const char* mqtt_server = "10.0.0.141";
const char* mqtt_user = "MQTT";
const char* mqtt_pass = "planeta";
const char* mqtt_topic_cmd = "rele/comando";           // Tópico de entrada (comando)
const char* mqtt_topic_status = "Mini-Lolin/52/status";// Tópico de saída (status)

// === Objetos globais ===
WiFiClient espClient;
PubSubClient client(espClient);
ESP8266WebServer server(80);
ESP8266HTTPUpdateServer httpUpdater;
Ticker watchdogTicker;

// === Variáveis de estado ===
bool releState = false;            // Estado atual do relé
bool lastButtonState = HIGH;       // Estado anterior do botão
unsigned long lastDebounceTime = 0;// Controle de debounce
unsigned long lastPublish = 0;     // Controle de publicação periódica
const unsigned long publishInterval = 60000; // Intervalo para publicar o status (ms)

// === Funções auxiliares ===

// Salva o estado do relé na EEPROM
void saveState(bool state) {
  EEPROM.write(EEPROM_ADDR, state);
  EEPROM.commit();
}

// Lê o estado salvo na EEPROM
bool readState() {
  return EEPROM.read(EEPROM_ADDR);
}

// Publica o estado do relé via MQTT
void publishStatus() {
  client.publish(mqtt_topic_status, releState ? "ON" : "OFF", true); // retain = true
}

// Aplica o estado ao pino do relé e publica se necessário
void applyRelayState(bool forcePublish = false) {
  digitalWrite(RELAY_PIN, releState ? HIGH : LOW);
  if (forcePublish) publishStatus();
}

// Lê o botão físico com debounce e alterna o relé se pressionado
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

// Callback executado ao receber mensagens MQTT
void callback(char* topic, byte* payload, unsigned int length) {
  String msg = "";
  for (unsigned int i = 0; i < length; i++) msg += (char)payload[i];
  msg.toUpperCase(); // Converte para maiúsculo

  // Interpreta comandos ON e OFF
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

// Reestabelece conexão com o broker MQTT se cair
void reconnectMQTT() {
  while (!client.connected()) {
    if (client.connect("releClient", mqtt_user, mqtt_pass)) {
      client.subscribe(mqtt_topic_cmd);
      publishStatus(); // publica status ao reconectar
    } else {
      delay(5000); // aguarda 5s e tenta de novo
    }
  }
}

// Alimenta o watchdog para evitar reset
void resetWatchdog() {
  ESP.wdtFeed();
}

// Retorna um JSON com status atual do sistema
void handleStatusJson() {
  String json = "{";
  json += "\"ip\":\"" + WiFi.localIP().toString() + "\",";
  json += "\"rele\":\"" + String(releState ? "Ligado" : "Desligado") + "\",";
  json += "\"uptime\":" + String(millis() / 1000) + ",";
  json += "\"rssi\":" + String(WiFi.RSSI());
  json += "}";
  server.send(200, "application/json", json);
}

// Alterna o estado do relé via requisição web AJAX
void handleToggle() {
  releState = !releState;
  applyRelayState(true);
  saveState(releState);
  server.send(200, "text/plain", "OK");
}

// === Função de inicialização ===
void setup() {
  pinMode(RELAY_PIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  EEPROM.begin(512);
  releState = readState(); // Recupera estado salvo
  applyRelayState();

  Serial.begin(115200);

  WiFi.config(local_ip, gateway, subnet);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) delay(500);

  SPIFFS.begin(); // Inicia sistema de arquivos
  server.serveStatic("/", SPIFFS, "/index.html");
  server.serveStatic("/style.css", SPIFFS, "/style.css");
  server.serveStatic("/script.js", SPIFFS, "/script.js");
  server.on("/toggle", HTTP_POST, handleToggle);
  server.on("/status.json", HTTP_GET, handleStatusJson);

  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);

  ESP.wdtEnable(WDTO_8S); // watchdog com 8s
  watchdogTicker.attach(2, resetWatchdog); // alimenta a cada 2s

  MDNS.begin("esp8266"); // permite acesso via http://esp8266.local
  httpUpdater.setup(&server); // rota /update para atualização OTA
  server.begin(); // inicia servidor HTTP

  lastPublish = millis(); // marca o tempo da última publicação
}

// === Função principal de execução contínua ===
void loop() {
  if (!client.connected()) reconnectMQTT();
  client.loop();
  server.handleClient(); // atende requisições HTTP
  checkButton();         // verifica botão físico
  ESP.wdtFeed();         // alimenta o watchdog

  // publicação periódica do status via MQTT
  if (millis() - lastPublish > publishInterval) {
    publishStatus();
    lastPublish = millis();
  }
}

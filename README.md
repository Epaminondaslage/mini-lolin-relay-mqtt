# 🚀 Projeto: Controle de Relé com ESP8266 (LOLIN D1 Mini V4.0.0)

> **Versão:** `1.0` | **Hardware:** `LOLIN D1 Mini (ESP8266)` | **Licença:** Livre para fins didáticos e profissionais  

---

## 🎯 Objetivo do Projeto

Desenvolver uma solução **IoT leve e eficiente**, baseada no **ESP8266**, com controle de relé via:

- 🌐 Página Web (AJAX)  
- 📡 Comandos MQTT  
- 🔘 Botão físico local  
- 💾 Persistência de estado na EEPROM  
- 🧠 Suporte a OTA  
- 📁 Arquivos armazenados via SPIFFS

---

# ⚙️ LOLIN D1 Mini V4.0.0

A LOLIN D1 Mini V4.0.0 é uma placa de desenvolvimento compacta baseada no microcontrolador ESP8266EX, projetada especialmente para aplicações de Internet das Coisas (IoT). Ela oferece conectividade Wi-Fi integrada, 4 MB de memória flash, 11 pinos digitais multifuncionais (suporte a PWM, interrupções, I2C e One-Wire) e uma entrada analógica, sendo alimentada por 5V via porta USB-C — que substitui a antiga Micro-USB das versões anteriores. A versão 4.0.0 traz melhorias no circuito de reset (com resistor de 470Ω entre RST e GPIO16 para facilitar o uso de modo Deep Sleep), otimizações no layout da placa e ajuste no LED onboard para reduzir o consumo de energia. Compacta (34,2 × 25,6 mm) e leve (3 g), é altamente compatível com ambientes como Arduino IDE, MicroPython e NodeMCU, sendo ideal para projetos de automação residencial, dispositivos inteligentes e integração com assistentes virtuais como o Home Assistant.

### 🔧 Especificações Técnicas

- **MCU:** ESP8266EX (32 bits, até 160 MHz)  
- **Memória Flash:** 4 MB  
- **Tensão de operação:** 3,3V  
- **Entrada USB:** 5V via USB-C  
- **I2C:** Conector dedicado  
- **Digitais:** 11 (com suporte a PWM, interrupções, I2C e One-Wire, exceto D0)  
- **Analógico:** 1 (A0, máx. 3,2V)  
- **Dimensões:** 34,2 × 25,6 mm | **Peso:** 3 g  

### 🆕 Novidades da Versão 4.0.0

- USB-C no lugar da Micro-B  
- Conector I2C dedicado  
- Melhorias no layout e no circuito de reset  
- Redução no consumo do LED onboard

---

## 💻 Programação & Compatibilidade

Compatível com:

- ✅ Arduino IDE  
- ✅ MicroPython  
- ✅ NodeMCU (Lua)

---

## 💡 Aplicações Típicas

- 🏠 Automação Residencial  
- 🌱 Monitoramento Ambiental  
- 🛠️ Projetos DIY e makers  
- 🤖 Integração com Home Assistant, Tasmota, etc.

---

## 📦 Conteúdo da Embalagem

- 1× Placa LOLIN D1 Mini V4.0.0  
- Conectores de pinos (headers)

---

# 🔌 Módulo de Relé V2.0.0

Relé ideal para acionar dispositivos AC/DC em projetos com D1 Mini.

### 📐 Especificações

- **Modelo:** SRD-05VDC-SL-C  
- **Pino de controle padrão:** D1 (GPIO5), configurável  
- **Tensão de operação:** 5V (alimentado pela D1 Mini)  
- **Conexões:**  
  - NO: 5A @ 250VAC / 30VDC  
  - NC: 3A @ 250VAC / 30VDC  
- **Empilhável** com outros shields

### 🔄 Configuração do Pino de Controle

1. ✂️ Corte da trilha padrão (D1)  
2. 🔧 Solda no novo pino desejado (D2, D3, etc.)

---

## 🖥️ Funcionalidades

- Interface Web com AJAX  
- Comando via MQTT  
- Status em JSON  
- Atualização OTA via navegador  
- Arquivos hospedados no SPIFFS

---

## 🧩 Arquitetura do Sistema

```
+---------------------+           +----------------------+
|     Interface Web   |<--------->|    ESP8266 WebServer |
|  (AJAX + HTML/CSS)  |           |                      |
+---------------------+           |  SPIFFS (index.html) |
                                   +----------+-----------+
                                              |
                                              v
                         +------------------[RELE]------------------+
                         |                                          |
                         |                                          |
                [Botão Físico]                             [MQTT Broker 10.0.0.141]
                         ^                                          ^
                         |                                          |
                   GPIO0 (D3)                             Tópicos: rele/comando, ESP8266/52/status
```

---

## 🗂️ Estrutura do Projeto

```bash
esp8266_rele_ota_ajax/
├── data/
│   ├── index.html     # Interface AJAX
│   ├── style.css      # Estilo visual
│   └── script.js      # Funções AJAX
├── src/
│   └── main.ino       # Código principal
```

---

## 🌐 Interfaces e Endpoints

- Página Web AJAX: `http://10.0.0.52/`  
- JSON de status: `http://10.0.0.52/status.json`  
- MQTT Comando: `rele/comando` (ON / OFF)  
- MQTT Status: `ESP8266/52/status`  

---

## 📚 Bibliotecas Utilizadas

| Biblioteca                   | Função                                    |
|-----------------------------|-------------------------------------------|
| `ESP8266WiFi.h`             | Wi-Fi                                     |
| `ESP8266WebServer.h`        | Servidor HTTP                             |
| `PubSubClient.h`            | Comunicação MQTT                          |
| `EEPROM.h`                  | Armazenamento de estado                   |
| `Ticker.h`                  | Watchdog                                  |
| `FS.h`                      | Acesso ao SPIFFS                          |
| `ESP8266HTTPUpdateServer.h`| Atualização OTA via navegador             |
| `ESP8266mDNS.h`             | Acesso via `esp8266.local`                |

---

## 🌍 Variáveis Globais

```cpp
bool releState;
unsigned long lastPublish;
WiFiClient espClient;
PubSubClient client;
ESP8266WebServer server;
Ticker watchdogTicker;
```

---

## 🔧 Principais Funções

| Função             | Descrição                                   |
|--------------------|---------------------------------------------|
| `setup()`          | Inicialização geral                         |
| `loop()`           | Manutenção do sistema                       |
| `applyRelayState()`| Aplica e publica o estado do relé           |
| `publishStatus()`  | Publica status MQTT                         |
| `checkButton()`    | Verifica botão local                        |
| `handleToggle()`   | Alterna estado via AJAX (POST `/toggle`)    |
| `handleStatusJson()`| Retorna JSON de status (`/status.json`)   |

---

## 🔁 Comunicação AJAX

Atualização de status:

```js
fetch('/status.json')
```

Alternância de estado:

```js
fetch('/toggle', { method: 'POST' })
```

---

## 📄 Exemplo JSON (`/status.json`)

```json
{
  "ip": "10.0.0.52",
  "rele": "Ligado",
  "uptime": 348,
  "rssi": -67
}
```

---

## 🌐 Configuração MQTT

- **Broker:** `10.0.0.141`  
- **Usuário:** `MQTT`  
- **Senha:** `planeta`  
- **Tópico comando:** `rele/comando`  
- **Tópico status:** `ESP8266/52/status`

---

## 🧠 SPIFFS

- `index.html` – interface  
- `style.css` – estilo  
- `script.js` – controle AJAX  

Upload via: **Arduino IDE → Tools → Upload Filesystem Image**

---

## 🔄 Atualização OTA

Acesse:  
**http://10.0.0.52/update**  
Faça upload do firmware `.bin` direto via navegador.

---

## 💎 Diferenciais Técnicos

- Interface AJAX rápida e leve  
- Controle via Web, Botão e MQTT  
- Estado persistente na EEPROM  
- Comunicação com watchdog  
- Suporte JSON para integração  
- Modular, comentado e expansível

---

## 📌 Próximos Passos

- 🌡️ Integração com sensores (DHT22, LDR)  
- 📝 Registro de eventos no SPIFFS  
- 🤖 Integração automática com Home Assistant

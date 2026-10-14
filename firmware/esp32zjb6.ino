#include <WiFi.h>
#include <ArduinoJson.h>

// ===============================
// CONFIGURAÇÃO DO WIFI
// ===============================

const char* ssid = "xxxxx";
const char* password = "xxxx";

// ===============================
// SENSOR ZJ-B6
// ===============================

const int PINO_SENSOR = 26;

volatile unsigned long pulsos = 0;

unsigned long pulsosAnterior = 0;
unsigned long tempoAnterior = 0;

// ===============================
// INTERRUPÇÃO
// ===============================

void IRAM_ATTR contarPulso() {
  pulsos++;
}

// ===============================
// SETUP
// ===============================

void setup() {

  Serial.begin(115200);
  delay(2000);

  Serial.println();
  Serial.println("================================");
  Serial.println("ESP32 + ZJ-B6 + WIFI");
  Serial.println("================================");

  // Sensor
  pinMode(PINO_SENSOR, INPUT_PULLUP);

  attachInterrupt(
    digitalPinToInterrupt(PINO_SENSOR),
    contarPulso,
    FALLING
  );

  // Wi-Fi
  Serial.print("Conectando ao Wi-Fi");

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.println("Wi-Fi conectado!");

  Serial.print("IP do ESP32: ");
  Serial.println(WiFi.localIP());

  Serial.println();
  Serial.println("Pode começar a passar a agua!");

  tempoAnterior = millis();
}

// ===============================
// LOOP
// ===============================

void loop() {

  noInterrupts();
  unsigned long p = pulsos;
  interrupts();

  // Litros acumulados
  float litros = p / 396.0;

  // Tempo atual
  unsigned long tempoAtual = millis();

  if (tempoAtual - tempoAnterior >= 1000) {

    // Pulsos ocorridos no último segundo
    unsigned long pulsosIntervalo = p - pulsosAnterior;

    // Calcula fluxo em L/min
    float fluxo_l_min =
      (pulsosIntervalo / 396.0) * 60.0;

    pulsosAnterior = p;
    tempoAnterior = tempoAtual;

    // ===============================
    // CRIA JSON
    // ===============================

    StaticJsonDocument<200> doc;

    doc["device_id"] = "ESP32_001";
    doc["pulsos"] = p;
    doc["litros"] = litros;
    doc["fluxo_l_min"] = fluxo_l_min;

    // ===============================
    // MOSTRA NO SERIAL
    // ===============================

    Serial.println();

    Serial.print("Pulsos: ");
    Serial.println(p);

    Serial.print("Litros: ");
    Serial.println(litros, 3);

    Serial.print("Fluxo: ");
    Serial.print(fluxo_l_min, 3);
    Serial.println(" L/min");

    Serial.print("JSON: ");

    serializeJson(doc, Serial);

    Serial.println();
  }

  delay(100);
}
#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>

#define SENSOR_PIN 13
#define LED_INTERNO 2

const char* ssid     = "WILLIANS";
const char* password = "w1n5t0n@97";
const char* serverUrl = "http://18.222.199.177:5000/sensor";

void setup() {
  Serial.begin(115200);
  pinMode(SENSOR_PIN, INPUT_PULLDOWN);  // <-- cambiado
  pinMode(LED_INTERNO, OUTPUT);

  WiFi.begin(ssid, password);
  Serial.print("Conectando a WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConectado! IP: " + WiFi.localIP().toString());

  Serial.println("Esperando calibracion del sensor (10 segundos)...");
  delay(10000);
  Serial.println("Listo!");
}

void loop() {
  int estado = digitalRead(SENSOR_PIN);
  digitalWrite(LED_INTERNO, estado == HIGH ? HIGH : LOW);

  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(serverUrl);
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");
    String body = "movimiento=" + String(estado);
    http.POST(body);
    http.end();
  }

  Serial.print("Pin GPIO13: ");
  Serial.print(estado);
  Serial.println(estado == HIGH ? " --> MOVIMIENTO detectado" : " --> Sin movimiento");

  delay(500);
}
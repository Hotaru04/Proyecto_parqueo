/*
 * ESP32 - I2C Master + Web Server
 * Proyecto 3 - IE3054
 */

#include <WiFi.h>
#include <WebServer.h>
#include <Wire.h>

const char* ssid     = "Redmi15";
const char* password = "12345678";

#define STM32_ADDR 0x08   // Direccion I2C del STM32

bool ledState = false;
WebServer server(80);

// Enviar comando al STM32 por I2C
void enviarI2C(uint8_t dato) {
  Wire.beginTransmission(STM32_ADDR);
  Wire.write(dato);
  Wire.endTransmission();
  Serial.print("I2C enviado: 0x");
  Serial.println(dato, HEX);
}

String paginaHTML() {
  String html = "<!DOCTYPE html><html><head><title>I2C LED</title></head><body>";
  html += "<h1>Control LED STM32</h1>";
  html += "<p>ESP32 -> I2C -> STM32</p>";

  if (ledState) {
    html += "<p>Estado: <b>ENCENDIDO</b></p>";
    html += "<a href='/off'><button>Apagar</button></a>";
  } else {
    html += "<p>Estado: <b>APAGADO</b></p>";
    html += "<a href='/on'><button>Encender</button></a>";
  }

  html += "</body></html>";
  return html;
}

void handleRoot() {
  server.send(200, "text/html", paginaHTML());
}

void handleOn() {
  ledState = true;
  enviarI2C(0x01);
  server.send(200, "text/html", paginaHTML());
}

void handleOff() {
  ledState = false;
  enviarI2C(0x00);
  server.send(200, "text/html", paginaHTML());
}

void setup() {
  Serial.begin(115200);

  // I2C Master
  Wire.begin(21, 22);  // SDA=21, SCL=22

  // WiFi
  WiFi.begin(ssid, password);
  Serial.print("Conectando");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());

  // Servidor
  server.on("/", handleRoot);
  server.on("/on", handleOn);
  server.on("/off", handleOff);
  server.begin();
  Serial.println("Servidor listo");
}

void loop() {
  server.handleClient();
}

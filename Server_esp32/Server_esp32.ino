/*
 * ESP32 Web Server - Control de LED
 * Proyecto 3 - IE3054
 */

#include <WiFi.h>
#include <WebServer.h>

const char* ssid     = "Redmi15";
const char* password = "12345678";

#define LED_PIN 2

bool ledState = false;
WebServer server(80);


String paginaHTML() {
  String html = "<!DOCTYPE html><html><head><title>LED</title></head><body>";
  html += "<h1>Control LED</h1>";

  if (ledState) {
    html += "<p>Estado: ENCENDIDO</p>";
    html += "<a href='/off'><button>Apagar</button></a>";
  } else {
    html += "<p>Estado: APAGADO</p>";
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
  digitalWrite(LED_PIN, HIGH);
  server.send(200, "text/html", paginaHTML());
}

void handleOff() {
  ledState = false;
  digitalWrite(LED_PIN, LOW);
  server.send(200, "text/html", paginaHTML());
}

void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  WiFi.begin(ssid, password);
  Serial.print("Conectando");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());

  server.on("/", handleRoot);
  server.on("/on", handleOn);
  server.on("/off", handleOff);
  server.begin();
}

void loop() {
  server.handleClient();
}

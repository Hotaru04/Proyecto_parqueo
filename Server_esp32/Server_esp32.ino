/*
 * ESP32 #1 - Servidor Web para Parqueo
 * Proyecto 3 - IE3054 Electronica Digital 2
 *
 */

#include <WiFi.h>
#include <WebServer.h>

// ===== CONFIGURACION =====
const char* ssid     = "Redmi15";
const char* password = "12345678";

WebServer server(80);

// ===== ESTADO DE PARQUEOS =====
// grupoA = 4 bits, parqueos 1-4 (STM32 de Abner)
// grupoB = 4 bits, parqueos 5-8 (STM32 del companero)
uint8_t grupoA = 0x00;
uint8_t grupoB = 0x00;

/* --- Generar pagina HTML --- */
String paginaHTML() {
  // Contar libres
  uint8_t ocupadosA = 0, ocupadosB = 0;
  for (int i = 0; i < 4; i++) {
    if (grupoA & (1 << i)) ocupadosA++;
    if (grupoB & (1 << i)) ocupadosB++;
  }
  uint8_t totalLibres = 8 - ocupadosA - ocupadosB;

  String html = "<!DOCTYPE html><html><head>";
  html += "<meta charset='UTF-8'>";
  html += "<meta http-equiv='refresh' content='3'>";
  html += "<title>Parqueo</title>";
  html += "<style>";
  html += "body{font-family:Arial;text-align:center;margin:20px;}";
  html += "table{margin:0 auto;border-collapse:collapse;}";
  html += "td{border:1px solid #333;padding:15px 25px;font-size:18px;min-width:80px;}";
  html += ".libre{background:#4CAF50;color:white;}";
  html += ".ocupado{background:#F44336;color:white;}";
  html += "h1{margin-bottom:5px;}";
  html += "h2{margin-top:5px;color:#555;}";
  html += "</style>";
  html += "</head><body>";

  html += "<h1>Centro de Control - Parqueo</h1>";
  html += "<h2>Espacios libres: " + String(totalLibres) + " / 8</h2>";
  html += "<p>IE3054 - Proyecto 3</p>";

  // Tabla: 2 filas x 4 columnas
  html += "<table>";

  // Fila 1: Grupo A (parqueos 1-4)
  html += "<tr>";
  for (int i = 0; i < 4; i++) {
    bool ocupado = grupoA & (1 << i);
    html += "<td class='";
    html += ocupado ? "ocupado" : "libre";
    html += "'>";
    html += "P" + String(i + 1) + "<br>";
    html += ocupado ? "OCUPADO" : "LIBRE";
    html += "</td>";
  }
  html += "</tr>";

  // Fila 2: Grupo B (parqueos 5-8)
  html += "<tr>";
  for (int i = 0; i < 4; i++) {
    bool ocupado = grupoB & (1 << i);
    html += "<td class='";
    html += ocupado ? "ocupado" : "libre";
    html += "'>";
    html += "P" + String(i + 5) + "<br>";
    html += ocupado ? "OCUPADO" : "LIBRE";
    html += "</td>";
  }
  html += "</tr>";

  html += "</table>";

  // Detalle por grupo
  html += "<p><b>Grupo A (1-4):</b> 0b";
  for (int i = 3; i >= 0; i--) html += String((grupoA >> i) & 1);
  html += " &nbsp; <b>Grupo B (5-8):</b> 0b";
  for (int i = 3; i >= 0; i--) html += String((grupoB >> i) & 1);
  html += "</p>";

  html += "</body></html>";
  return html;
}

/* --- Ruta: pagina principal --- */
void handleRoot() {
  server.send(200, "text/html", paginaHTML());
}

/* --- Ruta: actualizar datos (llamada por ESP32 #2) --- */
void handleUpdate() {
  if (server.hasArg("a")) {
    grupoA = server.arg("a").toInt() & 0x0F;
  }
  if (server.hasArg("b")) {
    grupoB = server.arg("b").toInt() & 0x0F;
  }

  Serial.print("Update -> A=");
  Serial.print(grupoA, BIN);
  Serial.print(" B=");
  Serial.println(grupoB, BIN);

  server.send(200, "text/plain", "OK");
}

/* --- Ruta: estado JSON (debug) --- */
void handleStatus() {
  String json = "{\"grupoA\":";
  json += String(grupoA);
  json += ",\"grupoB\":";
  json += String(grupoB);
  json += "}";
  server.send(200, "application/json", json);
}

/* --- 404 --- */
void handleNotFound() {
  server.send(404, "text/plain", "No encontrado");
}

void setup() {
  Serial.begin(115200);
  Serial.println("\n=== ESP32 Servidor Web - Parqueo ===");

  // Conectar a WiFi
  WiFi.begin(ssid, password);
  Serial.print("Conectando a WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.println("=================================");
  Serial.print("IP del SERVIDOR: ");
  Serial.println(WiFi.localIP());
  Serial.println("=================================");
  Serial.println("Pon esta IP en el codigo del ESP32 cliente");
  Serial.println("y en el navegador de tu PC para ver la pagina.\n");

  // Configurar rutas
  server.on("/", handleRoot);
  server.on("/update", handleUpdate);
  server.on("/status", handleStatus);
  server.onNotFound(handleNotFound);

  server.begin();
  Serial.println("Servidor HTTP iniciado en puerto 80");
}

void loop() {
  server.handleClient();
}

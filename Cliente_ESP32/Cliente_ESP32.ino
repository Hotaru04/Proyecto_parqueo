/*
 * ESP32 #2 - Cliente I2C + HTTP
 * Proyecto 3 - IE3054 Electronica Digital 2
 *
 */

#include <WiFi.h>
#include <HTTPClient.h>
#include <Wire.h>

// ===== CONFIGURACION =====
const char* ssid     = "Redmi15";
const char* password = "12345678";

const char* serverIP = "10.65.161.107";

// Direcciones I2C de las dos STM32
#define STM32_7SEG    0x08  // STM32 con 7-seg + sensores 1-4
#define STM32_TFT 0x09  // STM32 con TFT + sensores 5-8

// Intervalo de lectura (ms)
#define INTERVALO_LECTURA  1000

// ===== VARIABLES =====
uint8_t grupoA = 0x00;  // 4 bits: parqueos 1-4 (STM32 Abner)
uint8_t grupoB = 0x00;  // 4 bits: parqueos 5-8 (STM32 companero)

// Comandos de servos recibidos del servidor web
uint8_t servo1 = 0;  // 0=cerrada, 1=abierta
uint8_t servo2 = 0;

unsigned long ultimaLectura = 0;

void setup() {
  Serial.begin(115200);
  Serial.println("\n=== ESP32 Cliente I2C + HTTP ===");

  // Iniciar I2C como Master
  Wire.begin(21, 22);
  Wire.setClock(100000);  // 100kHz standard mode
  Serial.println("I2C Master iniciado (SDA=21, SCL=22)");

  // Conectar a WiFi
  WiFi.begin(ssid, password);
  Serial.print("Conectando a WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.print("IP del cliente: ");
  Serial.println(WiFi.localIP());
  Serial.print("Servidor en: ");
  Serial.println(serverIP);
  Serial.println("Listo!\n");
}

/* Escribir 1 byte a un esclavo I2C */
void escribirI2C(uint8_t direccion, uint8_t dato) {
  Wire.beginTransmission(direccion);
  Wire.write(dato);
  uint8_t err = Wire.endTransmission();
  if (err != 0) {
    Serial.print("Error escribir I2C 0x");
    Serial.print(direccion, HEX);
    Serial.print(" err=");
    Serial.println(err);
  }
}

/* Leer 1 byte de un esclavo I2C
 * Retorna el byte leido, o 0xFF si hubo error */
uint8_t leerI2C(uint8_t direccion) {
  uint8_t dato = 0xFF;

  // Solicitar 1 byte al esclavo
  uint8_t bytesRecibidos = Wire.requestFrom(direccion, (uint8_t)1);

  if (bytesRecibidos == 1 && Wire.available()) {
    dato = Wire.read();
    // Mascara: solo los 4 bits bajos interesan
    dato &= 0x0F;
  } else {
    Serial.print("Error I2C addr 0x");
    Serial.print(direccion, HEX);
    Serial.print(" - bytes: ");
    Serial.println(bytesRecibidos);
  }

  return dato;
}

/* Enviar datos al servidor ESP32 #1 por HTTP
 * La respuesta trae "servo1,servo2" para controlar talanqueras */
void enviarAlServidor(uint8_t gA, uint8_t gB) {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi desconectado!");
    return;
  }

  HTTPClient http;

  // Construir URL: /update?a=XX&b=XX
  String url = "http://";
  url += serverIP;
  url += "/update?a=";
  url += String(gA);
  url += "&b=";
  url += String(gB);

  http.begin(url);
  int httpCode = http.GET();

  if (httpCode == 200) {
    // Respuesta: "servo1,servo2"
    String resp = http.getString();
    int coma = resp.indexOf(',');
    if (coma > 0) {
      servo1 = (uint8_t)resp.substring(0, coma).toInt();
      servo2 = (uint8_t)resp.substring(coma + 1).toInt();
    }
    Serial.print("OK -> S1=");
    Serial.print(servo1);
    Serial.print(" S2=");
    Serial.println(servo2);
  } else {
    Serial.print("Error HTTP: ");
    Serial.println(httpCode);
  }

  http.end();
}

void loop() {
  unsigned long ahora = millis();

  if (ahora - ultimaLectura >= INTERVALO_LECTURA) {
    ultimaLectura = ahora;

    // 1. Leer STM32 de Abner (parqueos 1-4)
    uint8_t datoA = leerI2C(STM32_7SEG);
    if (datoA != 0xFF) {
      grupoA = datoA;
    }

    // 2. Leer STM32 del companero (parqueos 5-8)
    uint8_t datoB = leerI2C(STM32_TFT);
    if (datoB != 0xFF) {
      grupoB = datoB;
    }

    // 3. Reenviar datos cruzados
    // Al STM32_7SEG: grupoB (bits 0-3) + servo1 (bit 4) + servo2 (bit 5)
    uint8_t byteParaAbner = (grupoB & 0x0F) | ((servo1 & 0x01) << 4) | ((servo2 & 0x01) << 5);
    escribirI2C(STM32_7SEG, byteParaAbner);
    delay(10);
    escribirI2C(STM32_TFT, grupoA);
    delay(10);

    // 4. Debug
    Serial.print("A: 0b");
    for (int i = 3; i >= 0; i--) Serial.print((grupoA >> i) & 1);
    Serial.print("  B: 0b");
    for (int i = 3; i >= 0; i--) Serial.print((grupoB >> i) & 1);
    Serial.print("  Servo: ");
    Serial.print(servo1);
    Serial.print(",");
    Serial.println(servo2);

    // 5. Enviar al servidor (y recibir comandos de servo)
    enviarAlServidor(grupoA, grupoB);
  }
}

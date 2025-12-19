#include <HardwareSerial.h>

HardwareSerial SerialRFID(1);

String lastEPC = "";
unsigned long lastRead = 0;
bool modoAutomatico = false;
bool modoManual = false;
unsigned int lecturaCount = 0;

void setup() {
  Serial.begin(115200);
  SerialRFID.begin(115200, SERIAL_8N1, 16, 17); // RX=16, TX=17
  Serial.println("🛰️ Lector JRD-4035 listo.");
  Serial.println("Comandos:");
  Serial.println(" A -> Activar modo automático");
  Serial.println(" L -> Activar modo manual");
  Serial.println(" R -> Leer en modo manual");
  Serial.println(" EXIT -> Salir del modo actual");
  Serial.println("--------------------------------------------------");
}

void loop() {
  // Leer comandos desde monitor serie
  if (Serial.available()) {
    String cmd = Serial.readStringUntil('\n');
    cmd.trim();
    cmd.toUpperCase();

    if (cmd == "A") {
      modoAutomatico = true;
      modoManual = false;
      Serial.println("⚙️  Modo automático activado.");
    } 
    else if (cmd == "L") {
      modoManual = true;
      modoAutomatico = false;
      Serial.println("⚙️  Modo manual activado. Escribe 'R' para leer.");
    } 
    else if (cmd == "EXIT") {
      modoAutomatico = false;
      modoManual = false;
      Serial.println("⛔ Modo detenido. Esperando nuevo comando...");
    } 
    else if (modoManual && cmd == "R") {
      lecturaCount++;
      realizarLectura();
    }
  }

  // Lectura automática cada 1 segundo
  if (modoAutomatico && millis() - lastRead > 1000) {
    lastRead = millis();
    lecturaCount++;
    realizarLectura();
  }
}

// ---------------------------------------------------
// 🔹 FUNCIÓN PRINCIPAL DE LECTURA
// ---------------------------------------------------
void realizarLectura() {
  Serial.printf("\n✅ Lectura #%d iniciada...\n", lecturaCount);
  Serial.println("📋 --- Registro de lectura ---");
  Serial.printf("⏱️ Tiempo: %lu ms\n", millis());

  byte inventoryCmd[] = {0xBB, 0x00, 0x22, 0x00, 0x00, 0x22, 0x7E};
  String response = enviarYRecibir(inventoryCmd, sizeof(inventoryCmd));

  // 🔎 Verificar si la respuesta indica "sin etiquetas"
  if (response.startsWith("BB01FF000115167E") || response.equalsIgnoreCase("BB01FF000115167E")) {
    Serial.println("📭 Ninguna etiqueta detectada (trama vacía).");
    Serial.println("✅ Lectura finalizada");
    Serial.println("-----------------------------");
    return;
  }

  // Extraer EPCs
  String epcs[8];
  int n = extraerEPCs(response, epcs, 8);

  if (n > 0) {
    for (int i = 0; i < n; i++) {
      Serial.printf("🏷️ EPC #%d detectado: %s\n", i + 1, epcs[i].c_str());
      lastEPC = epcs[i];
    }
  } else {
    Serial.println("📭 Ningún EPC detectado en trama válida.");
  }

  Serial.println("✅ Lectura finalizada");
  Serial.println("-----------------------------");
}

// ---------------------------------------------------
// 🔹 ENVÍO Y RECEPCIÓN DE TRAMAS
// ---------------------------------------------------
String enviarYRecibir(byte* cmd, int len) {
  SerialRFID.write(cmd, len);
  delay(200);

  String hexResponse = "";
  while (SerialRFID.available()) {
    byte b = SerialRFID.read();
    if (b < 16) hexResponse += "0";
    hexResponse += String(b, HEX);
  }

  if (hexResponse.length() > 0) {
    hexResponse.toUpperCase();
    Serial.print("📥 Respuesta (hex): ");
    Serial.println(hexResponse);
  }

  return hexResponse;
}

// ---------------------------------------------------
// 🔹 EXTRACCIÓN DE EPC DE 24 CARACTERES HEX
// ---------------------------------------------------
int extraerEPCs(String response, String* lista, int maxEPCs) {
  response.toUpperCase();
  response.replace(" ", "");
  int count = 0;

  // Buscar tramas válidas que empiecen con BB0222 (respuesta con EPC)
  int start = response.indexOf("BB0222");
  while (start != -1 && count < maxEPCs) {
    // Buscar "3000" dentro de la trama (inicio del EPC real)
    int epcStart = response.indexOf("3000", start);
    if (epcStart != -1) {
      epcStart += 4; // Saltar "3000"
      if (epcStart + 24 <= response.length()) {
        String epc = response.substring(epcStart, epcStart + 24);
        if (esHexValido(epc)) {
          lista[count++] = epc;
        }
      }
    } else {
      // Si no hay "3000", buscar directamente el bloque EPC (según posición del payload)
      int dataStart = start + 20;  // Ajustado según tramas reales
      if (dataStart + 24 <= response.length()) {
        String epc = response.substring(dataStart, dataStart + 24);
        if (esHexValido(epc)) lista[count++] = epc;
      }
    }

    start = response.indexOf("BB0222", start + 1);
  }

  return count;
}

// ---------------------------------------------------
// 🔹 VALIDAR HEXADECIMAL
// ---------------------------------------------------
bool esHexValido(const String &s) {
  if (s.length() != 24) return false;
  for (unsigned int i = 0; i < s.length(); i++) {
    char c = s[i];
    if (!isxdigit(c)) return false;
  }
  return true;
}

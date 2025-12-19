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
  Serial.println("Comandos: A (auto), L (manual), R (leer en manual), EXIT (detener)");
  Serial.println("---------------------------------------------------------");
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

  // Lectura automática cada 1 s
  if (modoAutomatico && millis() - lastRead > 1000) {
    lastRead = millis();
    lecturaCount++;
    realizarLectura();
  }
}

// realiza una lectura: enviar inventario, recibir trama, extraer EPCs
void realizarLectura() {
  Serial.printf("\n✅ Lectura #%d iniciada...\n", lecturaCount);
  Serial.println("📋 --- Registro de lectura ---");
  Serial.printf("⏱️ Tiempo: %lu ms\n", millis());

  byte inventoryCmd[] = {0xBB, 0x00, 0x22, 0x00, 0x00, 0x22, 0x7E};
  String response = enviarYRecibir(inventoryCmd, sizeof(inventoryCmd));

  // Extraer EPCs: primero intento por posición fija (byte 8..19), luego fallback
  String epcs[8];
  int n = extraerEPCs(response, epcs, 8);

  if (n > 0) {
    for (int i = 0; i < n; i++) {
      Serial.printf("🏷️ EPC #%d detectado: %s\n", i + 1, epcs[i].c_str());
      lastEPC = epcs[i];
    }
  } else {
    Serial.println("📭 Ningún EPC detectado.");
  }

  Serial.println("✅ Lectura finalizada");
  Serial.println("-----------------------------");
}

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

// --- EXTRAER MÚLTIPLES EPCs ---
// 1) Intenta extraer por posición fija: after "BB0222" take bytes 8..19 (12 bytes -> 24 hex chars).
// 2) Si no aparecen, busca "3000" y toma 24 hex siguientes.
// 3) Si aún no, busca cualquier bloque válido de 24 hex consecutivos.
int extraerEPCs(String response, String* lista, int maxEPCs) {
  response.toUpperCase();
  response.replace(" ", "");
  int count = 0;

  // 1) Posición fija basada en tu diagrama: encontrar "BB0222" y extraer desde byte index 8 (char offset +16)
  int start = response.indexOf("BB0222");
  if (start != -1) {
    int epcCharStart = start + 16; // 8 bytes * 2 chars/byte = 16 chars from start
    if (epcCharStart + 24 <= response.length()) {
      String epc = response.substring(epcCharStart, epcCharStart + 24);
      if (esHexValido(epc)) {
        lista[count++] = epc;
      }
    }
  }

  // 2) Buscar todas las ocurrencias de "3000" (fallback más semántico)
  int idx = 0;
  while (count < maxEPCs) {
    int pos = response.indexOf("3000", idx);
    if (pos == -1) break;
    int candidateStart = pos + 4; // después de "3000"
    if (candidateStart + 24 <= response.length()) {
      String epc = response.substring(candidateStart, candidateStart + 24);
      if (esHexValido(epc)) {
        // evitar duplicados
        bool dup = false;
        for (int k = 0; k < count; k++) if (lista[k] == epc) { dup = true; break; }
        if (!dup) lista[count++] = epc;
      }
    }
    idx = pos + 1;
  }

  // 3) Si aún no hay resultados, buscar cualquier bloque de 24 hex consecutivos
  if (count == 0) {
    for (int i = 0; i <= response.length() - 24 && count < maxEPCs; i++) {
      String candidate = response.substring(i, i + 24);
      if (esHexValido(candidate)) {
        // evitar duplicados
        bool dup = false;
        for (int k = 0; k < count; k++) if (lista[k] == candidate) { dup = true; break; }
        if (!dup) lista[count++] = candidate;
      }
    }
  }

  return count;
}

// validar que la cadena tenga sólo dígitos hexadecimales
bool esHexValido(const String &s) {
  if (s.length() == 0) return false;
  for (unsigned int i = 0; i < s.length(); i++) {
    char c = s[i];
    if (!isxdigit(c)) return false;
  }
  return true;
}

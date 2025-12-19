#include <HardwareSerial.h>

HardwareSerial SerialRFID(1);
unsigned long lastRead = 0;

void setup() {
  Serial.begin(115200);
  SerialRFID.begin(115200, SERIAL_8N1, 16, 17); // RX=16, TX=17
  Serial.println("🛰️ Lector JRD-4035 listo. Escaneando automáticamente...");
}

void loop() {
  if (millis() - lastRead > 800) {  // lectura cada 0.8s
    lastRead = millis();
    readEPC();
  }
}

void readEPC() {
  Serial.println("\n✅ Lectura iniciada...");

  byte inventoryCmd[] = {0xBB, 0x00, 0x22, 0x00, 0x00, 0x22, 0x7E};
  String response = sendAndGetResponse(inventoryCmd, sizeof(inventoryCmd));

  String epc = extractEPC(response);

  if (epc.length() > 0) {
    Serial.print("🏷️ EPC detectado: ");
    Serial.println(epc);
  } else {
    Serial.println("📥 Ningún EPC detectado.");
  }

  Serial.println("✅ Lectura finalizada\n");
}

String sendAndGetResponse(byte* cmd, int len) {
  SerialRFID.write(cmd, len);
  delay(200);

  String hexResponse = "";
  while (SerialRFID.available()) {
    byte b = SerialRFID.read();
    if (b < 16) hexResponse += "0";
    hexResponse += String(b, HEX);
  }

  if (hexResponse.length() > 0) {
    Serial.print("📥 Respuesta (hex): ");
    Serial.println(hexResponse);
  }

  return hexResponse;
}

String extractEPC(String response) {
  response.toUpperCase();
  response.replace(" ", "");

  int start = response.indexOf("BB0222");
  if (start == -1) return "";

  // Buscar patrón ??3000
  int epcStart = -1;
  for (int i = start; i < response.length() - 6; i++) {
    if (response.substring(i + 2, i + 6) == "3000") {
      epcStart = i + 6;
      break;
    }
  }
  if (epcStart == -1) return "";

  int end = response.indexOf("7E", epcStart);
  if (end == -1) end = response.length();

  String epc = response.substring(epcStart, end);

  // 🔧 Aumentamos el límite a 28 caracteres (14 bytes)
  if (epc.length() > 28) epc = epc.substring(0, 28);

  return epc;
}

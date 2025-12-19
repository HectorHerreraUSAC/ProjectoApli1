#include <HardwareSerial.h>

HardwareSerial SerialRFID(1);

bool modoAutomatico = false;
bool modoManual = false;

void setup() {
  Serial.begin(115200);
  SerialRFID.begin(115200, SERIAL_8N1, 16, 17);
  Serial.println("Lector JRD-4035 - Listo");
  Serial.println("A -> Modo automático");
  Serial.println("L -> Modo manual");
  Serial.println("R -> Leer (modo manual)");
  Serial.println("EXIT -> Salir");
}

void loop() {
  if (Serial.available()) {
    String cmd = Serial.readStringUntil('\n');
    cmd.trim();
    cmd.toUpperCase();

    if (cmd == "A") {
      modoAutomatico = true;
      modoManual = false;
      Serial.println("Modo automático activado");
    } 
    else if (cmd == "L") {
      modoManual = true;
      modoAutomatico = false;
      Serial.println("Modo manual activado - Escribe R para leer");
    } 
    else if (cmd == "EXIT") {
      modoAutomatico = false;
      modoManual = false;
      Serial.println("Modo detenido");
    } 
    else if (modoManual && cmd == "R") {
      realizarLectura();
    }
  }

  if (modoAutomatico) {
    realizarLectura();
    delay(1000);
  }
}

void realizarLectura() {
  byte inventoryCmd[] = {0xBB, 0x00, 0x22, 0x00, 0x00, 0x22, 0x7E};
  
  // Enviar comando
  SerialRFID.write(inventoryCmd, sizeof(inventoryCmd));
  delay(200);

  // Leer respuesta
  String hexResponse = "";
  int byteCount = 0;
  unsigned long startTime = millis();
  
  while (millis() - startTime < 500) {
    if (SerialRFID.available()) {
      byte b = SerialRFID.read();
      byteCount++;
      if (b < 16) hexResponse += "0";
      hexResponse += String(b, HEX);
    }
  }

  hexResponse.toUpperCase();
  hexResponse.trim();
  
  if (byteCount > 0) {
    Serial.print("📥 Respuesta (");
    Serial.print(byteCount);
    Serial.print(" bytes): ");
    Serial.println(hexResponse);
    
    // Extraer y mostrar EPC
    String epc = extraerEPCPreciso(hexResponse);
    if (epc.length() > 0) {
      Serial.print("🏷️ EPC detectado: ");
      Serial.println(epc);
    } else {
      Serial.println("No se detectó etiqueta");
    }
  } else {
    Serial.println("No hay respuesta del módulo");
  }
}

String extraerEPCPreciso(String response) {
  response.toUpperCase();
  response.replace(" ", "");
  
  if (response.length() < 40 || !response.startsWith("BB0222")) {
    return "";
  }

  int inicioEPC = 16;
  int finEPC = inicioEPC + 24;
  
  if (response.length() >= finEPC) {
    String epc = response.substring(inicioEPC, finEPC);
    
    // Validar EPC
    if (epc.length() == 24) {
      bool valido = true;
      for (int i = 0; i < epc.length(); i++) {
        char c = epc[i];
        if (!((c >= '0' && c <= '9') || (c >= 'A' && c <= 'F'))) {
          valido = false;
          break;
        }
      }
      if (valido) return epc;
    }
  }
  
  return "";
}
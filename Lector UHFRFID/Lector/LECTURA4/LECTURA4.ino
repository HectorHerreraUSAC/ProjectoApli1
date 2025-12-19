#include <HardwareSerial.h>

HardwareSerial SerialRFID(1);

bool modoAutomatico = false;
bool modoManual = false;

void setup() {
  Serial.begin(115200);
  SerialRFID.begin(115200, SERIAL_8N1, 16, 17);
  Serial.println("Lector JRD-4035 - Detector Universal");
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
  // Comando de inventario
  byte inventoryCmd[] = {0xBB, 0x00, 0x22, 0x00, 0x00, 0x22, 0x7E};
  
  // Enviar comando
  SerialRFID.write(inventoryCmd, sizeof(inventoryCmd));
  delay(200);

  // Leer respuesta
  String hexResponse = "";
  int byteCount = 0;
  unsigned long startTime = millis();
  
  while (millis() - startTime < 2000) {
    if (SerialRFID.available()) {
      byte b = SerialRFID.read();
      byteCount++;
      if (b < 16) hexResponse += "0";
      hexResponse += String(b, HEX);
      hexResponse += " ";
    }
  }

  hexResponse.toUpperCase();
  hexResponse.trim();
  
  if (byteCount > 0) {
    // Mostrar respuesta completa
    Serial.print("📥 Respuesta (");
    Serial.print(byteCount);
    Serial.print(" bytes): ");
    Serial.println(hexResponse);
    
    // Extraer y mostrar todas las etiquetas únicas
    extraerYMostrarEtiquetasUnicas(hexResponse);
  } else {
    Serial.println("No hay respuesta del módulo");
  }
}

void extraerYMostrarEtiquetasUnicas(String response) {
  response.toUpperCase();
  String responseSinEspacios = response;
  responseSinEspacios.replace(" ", "");
  
  // Array para almacenar etiquetas únicas
  String etiquetasUnicas[20];
  int totalUnicas = 0;
  
  // Buscar todas las tramas BB0222 en la respuesta
  int posicion = 0;
  while ((posicion = responseSinEspacios.indexOf("BB0222", posicion)) != -1) {
    // Verificar que hay suficientes caracteres para una trama completa
    if (posicion + 46 <= responseSinEspacios.length()) {
      String tramaCompleta = responseSinEspacios.substring(posicion, posicion + 46);
      
      // Extraer EPC de la trama (posiciones 16 a 39)
      String epc = tramaCompleta.substring(16, 40);
      
      // Validar que es un EPC válido
      if (esEPCValido(epc)) {
        // Verificar si ya existe en el array
        bool existe = false;
        for (int i = 0; i < totalUnicas; i++) {
          if (etiquetasUnicas[i] == epc) {
            existe = true;
            break;
          }
        }
        
        // Si no existe, agregarlo
        if (!existe && totalUnicas < 20) {
          etiquetasUnicas[totalUnicas] = epc;
          totalUnicas++;
        }
      }
    }
    
    // Moverse a la siguiente posición
    posicion += 2;
  }
  
  // Mostrar todas las etiquetas únicas detectadas
  if (totalUnicas > 0) {
    //Serial.println("🏷️ Etiquetas detectadas:");
    for (int i = 0; i < totalUnicas; i++) {
      Serial.print("  ");
      Serial.print(i + 1);
      Serial.print(". ");
      Serial.println(etiquetasUnicas[i]);
    }
    Serial.print("✅ Total de etiquetas únicas: ");
    Serial.println(totalUnicas);
  } else {
    Serial.println("📭 No se detectaron etiquetas");
  }
}

bool esEPCValido(String epc) {
  // Debe tener exactamente 24 caracteres HEX
  if (epc.length() != 24) return false;
  
  // Verificar que todos los caracteres sean HEX válidos
  for (int i = 0; i < epc.length(); i++) {
    char c = epc[i];
    if (!((c >= '0' && c <= '9') || (c >= 'A' && c <= 'F'))) {
      return false;
    }
  }
  
  // No debe ser todo ceros
  if (esTodoCeros(epc)) return false;
  
  // No debe ser todo Fs
  if (esTodoFF(epc)) return false;
  
  return true;
}

bool esTodoCeros(String str) {
  for (int i = 0; i < str.length(); i++) {
    if (str[i] != '0') return false;
  }
  return true;
}

bool esTodoFF(String str) {
  for (int i = 0; i < str.length(); i++) {
    if (str[i] != 'F') return false;
  }
  return true;
}
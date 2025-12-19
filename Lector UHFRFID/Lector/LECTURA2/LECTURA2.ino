#include <HardwareSerial.h>

HardwareSerial SerialRFID(1);

bool modoAutomatico = false;
bool modoManual = false;

// Array para almacenar etiquetas conocidas
String etiquetasConocidas[] = {
  "E280689400005031A1C90D86",  // Impinj Monza
  "AABB0000C209830000000000",  // #83
  "AABB0000C209840000000000",  // #84  
  "AABB0000C209850000000000",  // #85
  "AABB0000C209860000000000",  // #86
  "AABB0000C209870000000000"   // #87
};

void setup() {
  Serial.begin(115200);
  SerialRFID.begin(115200, SERIAL_8N1, 16, 17);
  Serial.println("Lector JRD-4035 - Modo Múltiples Etiquetas");
  Serial.println("A -> Modo automático");
  Serial.println("L -> Modo manual");
  Serial.println("R -> Leer (modo manual)");
  Serial.println("M -> Modo múltiples etiquetas");
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
    else if (cmd == "M") {
      modoManual = true;
      modoAutomatico = false;
      Serial.println("Modo múltiples etiquetas activado - Escribe R para leer");
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
  // Usar comando de inventario múltiple para mejor detección
  byte inventoryCmd[] = {0xBB, 0x00, 0x27, 0x00, 0x03, 0x22, 0x27, 0x10, 0x83, 0x7E};
  
  // Enviar comando
  SerialRFID.write(inventoryCmd, sizeof(inventoryCmd));
  delay(200);

  // Leer respuesta
  String hexResponse = "";
  int byteCount = 0;
  unsigned long startTime = millis();
  
  while (millis() - startTime < 1000) {
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
    Serial.print("📥 Respuesta (");
    Serial.print(byteCount);
    Serial.print(" bytes): ");
    Serial.println(hexResponse);
    
    // Analizar respuesta para múltiples etiquetas
    analizarRespuestaMultiple(hexResponse);
  } else {
    Serial.println("No hay respuesta del módulo");
  }
}

void analizarRespuestaMultiple(String response) {
  response.toUpperCase();
  String responseSinEspacios = response;
  responseSinEspacios.replace(" ", "");
  
  int etiquetasDetectadas = 0;
  
  // Buscar etiquetas conocidas en la respuesta
  for (int i = 0; i < 6; i++) {
    if (responseSinEspacios.indexOf(etiquetasConocidas[i]) >= 0) {
      etiquetasDetectadas++;
      Serial.print("🏷️ EPC detectado #");
      Serial.print(etiquetasDetectadas);
      Serial.print(": ");
      Serial.println(etiquetasConocidas[i]);
      
      // Mostrar información de la etiqueta
      mostrarInfoEtiqueta(etiquetasConocidas[i]);
    }
  }
  
  // Si no encontramos etiquetas conocidas, buscar nuevos patrones
  if (etiquetasDetectadas == 0) {
    buscarNuevosEPCs(responseSinEspacios);
  } else {
    Serial.print("✅ Total de etiquetas detectadas: ");
    Serial.println(etiquetasDetectadas);
  }
}

void mostrarInfoEtiqueta(String epc) {
  if (epc == "E280689400005031A1C90D86") {
    Serial.println("   Tipo: Impinj Monza");
  } else if (epc == "AABB0000C209830000000000") {
    Serial.println("   Tipo: Genérica - #83");
  } else if (epc == "AABB0000C209840000000000") {
    Serial.println("   Tipo: Genérica - #84");
  } else if (epc == "AABB0000C209850000000000") {
    Serial.println("   Tipo: Genérica - #85");
  } else if (epc == "AABB0000C209860000000000") {
    Serial.println("   Tipo: Genérica - #86");
  } else if (epc == "AABB0000C209870000000000") {
    Serial.println("   Tipo: Genérica - #87");
  }
}

void buscarNuevosEPCs(String response) {
  // Buscar patrones de EPC en la respuesta
  // Los EPCs suelen ser secuencias de 24 caracteres HEX
  int epcsEncontrados = 0;
  
  for (int i = 0; i <= response.length() - 24; i++) {
    String posibleEPC = response.substring(i, i + 24);
    
    if (esEPCValido(posibleEPC)) {
      epcsEncontrados++;
      Serial.print("🔍 Nuevo EPC detectado #");
      Serial.print(epcsEncontrados);
      Serial.print(": ");
      Serial.println(posibleEPC);
      
      // Analizar el nuevo EPC
      analizarNuevoEPC(posibleEPC);
    }
  }
  
  if (epcsEncontrados == 0) {
    Serial.println("No se detectaron etiquetas");
    
    // Mostrar secciones interesantes de la respuesta para análisis
    if (response.length() > 40) {
      Serial.println("🔍 Secciones para análisis:");
      for (int i = 0; i < response.length(); i += 16) {
        if (i + 16 <= response.length()) {
          String segmento = response.substring(i, i + 16);
          if (!esTodoCeros(segmento) && !esTodoFF(segmento)) {
            Serial.print("   Pos ");
            Serial.print(i);
            Serial.print(": ");
            Serial.println(segmento);
          }
        }
      }
    }
  }
}

bool esEPCValido(String epc) {
  if (epc.length() != 24) return false;
  
  // Verificar que sean caracteres HEX válidos
  for (int i = 0; i < epc.length(); i++) {
    char c = epc[i];
    if (!((c >= '0' && c <= '9') || (c >= 'A' && c <= 'F'))) {
      return false;
    }
  }
  
  // No debe ser todo ceros o todo Fs
  if (esTodoCeros(epc) || esTodoFF(epc)) return false;
  
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

void analizarNuevoEPC(String epc) {
  Serial.print("   📊 Análisis: ");
  
  if (epc.startsWith("E2")) {
    Serial.println("Posible etiqueta Impinj");
  } else if (epc.startsWith("30")) {
    Serial.println("Posible etiqueta Alien Higgs");
  } else if (epc.startsWith("AA")) {
    Serial.println("Posible etiqueta genérica");
  } else {
    Serial.println("Tipo desconocido");
  }
  
  Serial.print("   🔍 Prefijo: ");
  Serial.println(epc.substring(0, 8));
}
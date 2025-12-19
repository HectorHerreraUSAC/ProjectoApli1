#include <HardwareSerial.h>

HardwareSerial SerialRFID(2);

// Pin para controlar la alimentación del módulo RFID (usar MOSFET o relé)
#define RFID_POWER_PIN 4

void setup() {
  Serial.begin(115200);
  
  // Configurar pin de control de alimentación
  pinMode(RFID_POWER_PIN, OUTPUT);
  digitalWrite(RFID_POWER_PIN, LOW); // Iniciar apagado
  
  delay(1000);
  
  Serial.println("==========================================");
  Serial.println("   JRD-4035 - CONTROL DE ALIMENTACIÓN");
  Serial.println("==========================================");
  Serial.println("ESTRATEGIA: Reset por power cycling");
  Serial.println("");
  Serial.println("Comandos:");
  Serial.println("  P - Power cycle + lectura");
  Serial.println("  C - Lectura continua (10 ciclos)");
  Serial.println("  T - Test con tags");
  Serial.println("  O - Encender módulo");
  Serial.println("  F - Apagar módulo");
  Serial.println("==========================================");
  
  // Encender módulo al inicio
  powerOn();
}

void loop() {
  if (Serial.available() > 0) {
    char cmd = Serial.read();
    
    switch(cmd) {
      case 'P':
      case 'p':
        powerCycleRead();
        break;
      case 'C':
      case 'c':
        continuousPowerCycle();
        break;
      case 'T':
      case 't':
        testWithTags();
        break;
      case 'O':
      case 'o':
        powerOn();
        break;
      case 'F':
      case 'f':
        powerOff();
        break;
    }
    
    while (Serial.available() > 0) Serial.read();
  }
  
  delay(100);
}

void powerOn() {
  Serial.println("\n🔌 ENCENDIENDO MÓDULO RFID");
  digitalWrite(RFID_POWER_PIN, HIGH);
  delay(3000); // Esperar que el módulo se inicialice
  Serial.println("✅ Módulo encendido");
}

void powerOff() {
  Serial.println("\n🔌 APAGANDO MÓDULO RFID");
  digitalWrite(RFID_POWER_PIN, LOW);
  Serial.println("✅ Módulo apagado");
}

void powerCycleRead() {
  Serial.println("\n🔄 POWER CYCLE + LECTURA");
  
  // 1. Apagar
  powerOff();
  delay(1000);
  
  // 2. Encender
  powerOn();
  
  // 3. Esperar inicialización completa
  delay(2000);
  
  // 4. Enviar comando de lectura
  Serial.println("📖 Enviando comando de lectura...");
  byte readCmd[] = {0xBB, 0x00, 0x39, 0x00, 0x09, 0x00, 0x00, 0xFF, 0xFF, 0x7E};
  sendAndRead(readCmd, sizeof(readCmd));
}

void continuousPowerCycle() {
  Serial.println("\n🔁 LECTURA CONTINUA CON POWER CYCLING");
  Serial.println("Realizando 10 ciclos de lectura...");
  
  for (int i = 1; i <= 10; i++) {
    Serial.print("\n🔄 Ciclo ");
    Serial.println(i);
    
    powerCycleRead();
    
    // Esperar entre ciclos
    delay(2000);
  }
}

void testWithTags() {
  Serial.println("\n🏷️  TEST CON ETIQUETAS");
  Serial.println("Coloca etiquetas UHF cerca del módulo...");
  
  for (int i = 1; i <= 5; i++) {
    Serial.print("\n📖 Intento ");
    Serial.println(i);
    
    powerCycleRead();
    
    // Esperar entre intentos
    delay(3000);
  }
}

void sendAndRead(byte* cmd, int length) {
  Serial.print("📤 Enviando: ");
  for (int i = 0; i < length; i++) {
    SerialRFID.write(cmd[i]);
    if (cmd[i] < 0x10) Serial.print("0");
    Serial.print(cmd[i], HEX);
    Serial.print(" ");
  }
  Serial.println();
  SerialRFID.flush();
  
  readAndAnalyzeResponse(3000);
}

void readAndAnalyzeResponse(int timeout) {
  String hexResponse = "";
  unsigned long startTime = millis();
  int bytesReceived = 0;
  
  while (millis() - startTime < timeout) {
    if (SerialRFID.available()) {
      byte b = SerialRFID.read();
      bytesReceived++;
      
      if (b < 0x10) hexResponse += "0";
      hexResponse += String(b, HEX);
      hexResponse += " ";
    }
  }
  
  if (bytesReceived > 0) {
    Serial.print("📥 Respuesta (");
    Serial.print(bytesReceived);
    Serial.print(" bytes): ");
    Serial.println(hexResponse);
    
    // Análisis automático
    if (bytesReceived > 50) {
      Serial.println("🎯 ¡DATOS DETECTADOS! Analizando...");
      extractAndDisplayTags(hexResponse);
    } else if (hexResponse.indexOf("0f 0f 00 e1") >= 0) {
      Serial.println("ℹ️  Estado: Módulo listo, sin tags detectados");
    }
  } else {
    Serial.println("📥 Sin respuesta");
  }
  Serial.println("------------------------------------");
}

void extractAndDisplayTags(String response) {
  Serial.println("🔍 EXTRAYENDO INFORMACIÓN DE TAGS:");
  
  // Convertir respuesta a bytes
  int byteCount = 0;
  byte data[256];
  
  int start = 0;
  while (start < response.length()) {
    int spaceIndex = response.indexOf(' ', start);
    if (spaceIndex == -1) spaceIndex = response.length();
    
    String byteStr = response.substring(start, spaceIndex);
    if (byteStr.length() == 2) {
      data[byteCount] = (byte)strtol(byteStr.c_str(), NULL, 16);
      byteCount++;
    }
    
    start = spaceIndex + 1;
    if (byteCount >= 256) break;
  }
  
  // Buscar EPC en la respuesta
  // Basado en tu respuesta: los bytes 70-75 parecen contener datos del tag
  Serial.println("Buscando datos EPC...");
  
  // Mostrar secciones con datos (no ceros)
  for (int i = 0; i < byteCount; i += 16) {
    bool hasData = false;
    for (int j = i; j < i + 16 && j < byteCount; j++) {
      if (data[j] != 0x00) {
        hasData = true;
        break;
      }
    }
    
    if (hasData) {
      Serial.print("Posición ");
      Serial.print(i);
      Serial.print("-");
      Serial.print(min(i+15, byteCount-1));
      Serial.print(": ");
      
      for (int j = i; j < i + 16 && j < byteCount; j++) {
        if (data[j] < 0x10) Serial.print("0");
        Serial.print(data[j], HEX);
        Serial.print(" ");
      }
      Serial.println();
    }
  }
  
  // Intentar extraer EPC específico
  extractEPCFromResponse(data, byteCount);
}

void extractEPCFromResponse(byte* data, int length) {
  Serial.println("\n🏷️  INTENTANDO EXTRAER EPC:");
  
  // Buscar patrones que se parezcan a EPC
  // Los EPC típicamente son 6-12 bytes no consecutivos en ceros
  
  for (int i = 0; i < length - 6; i++) {
    // Verificar si hay una secuencia de varios bytes no cero
    int nonZeroCount = 0;
    for (int j = i; j < min(i + 12, length); j++) {
      if (data[j] != 0x00) nonZeroCount++;
    }
    
    // Si encontramos una secuencia con al menos 6 bytes no cero, podría ser EPC
    if (nonZeroCount >= 6) {
      Serial.print("Posible EPC en ");
      Serial.print(i);
      Serial.print(": ");
      
      for (int j = i; j < min(i + 12, length); j++) {
        if (data[j] < 0x10) Serial.print("0");
        Serial.print(data[j], HEX);
        Serial.print(" ");
      }
      Serial.println();
      
      // Saltar adelante para no mostrar superposiciones
      i += 11;
    }
  }
}

int min(int a, int b) {
  return (a < b) ? a : b;
}
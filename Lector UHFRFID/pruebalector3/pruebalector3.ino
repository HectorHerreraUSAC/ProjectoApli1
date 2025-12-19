#include <HardwareSerial.h>

HardwareSerial SerialRFID(2);

void setup() {
  Serial.begin(115200);
  delay(3000);
  
  Serial.println("🎯 JRD-4035 - PROTOCOLO CORREGIDO");
  Serial.println("==================================");
  Serial.println("Usando baud rate: 9600 (comprobado)");
  Serial.println("Comandos: L=Leer, C=Config, V=Versión, T=Test");
  Serial.println("==================================");
  
  // Usar 9600 baudios que mostró respuesta
  SerialRFID.begin(9600, SERIAL_8N1, 16, 17);
  
  // Inicializar con comando simple primero
  initializeModule();
}

void loop() {
  if (Serial.available() > 0) {
    char cmd = Serial.read();
    
    switch(cmd) {
      case 'L':
      case 'l':
        readTags();
        break;
      case 'C':
      case 'c':
        configureModule();
        break;
      case 'V':
      case 'v':
        getVersion();
        break;
      case 'T':
      case 't':
        testCommunication();
        break;
      case 'D':
      case 'd':
        debugMode();
        break;
    }
    
    while (Serial.available() > 0) Serial.read();
  }
  
  delay(100);
}

void initializeModule() {
  Serial.println("\n⚡ INICIALIZANDO MÓDULO");
  
  // Comando simple de reset o inicialización
  byte initCmd[] = {0xBB, 0x00, 0x02, 0x00, 0x02, 0x7E};
  sendCommand(initCmd, sizeof(initCmd));
  readResponse(2000);
}

void readTags() {
  Serial.println("\n🔍 INICIANDO LECTURA DE TAGS");
  
  // COMANDO CORREGIDO - con checksum apropiado
  // BB 00 22 00 00 [checksum] 7E
  byte inventoryCmd[] = {0xBB, 0x00, 0x22, 0x00, 0x00, 0x22, 0x7E};
  
  Serial.println("Comando de inventario estándar:");
  sendCommand(inventoryCmd, sizeof(inventoryCmd));
  readResponse(3000);
  
  // También probar comando alternativo
  delay(500);
  Serial.println("\n🔄 Comando alternativo de inventario:");
  byte altInventory[] = {0xBB, 0x00, 0x27, 0x00, 0x03, 0x22, 0x27, 0x10, 0x83, 0x7E};
  sendCommand(altInventory, sizeof(altInventory));
  readResponse(3000);
}

void configureModule() {
  Serial.println("\n⚙️ CONFIGURACIÓN BÁSICA");
  
  // Configurar región con checksum correcto
  Serial.println("Configurando región FCC:");
  byte regionCmd[] = {0xBB, 0x00, 0x07, 0x00, 0x01, 0x01, 0x11, 0x7E};
  sendCommand(regionCmd, sizeof(regionCmd));
  readResponse(1000);
  
  delay(500);
  
  // Configurar potencia
  Serial.println("Configurando potencia:");
  byte powerCmd[] = {0xBB, 0x00, 0xB6, 0x00, 0x02, 0x14, 0x6A, 0x7E}; // 20dBm
  sendCommand(powerCmd, sizeof(powerCmd));
  readResponse(1000);
}

void getVersion() {
  Serial.println("\n📋 SOLICITANDO INFORMACIÓN");
  
  // Comando de versión estándar
  byte versionCmd[] = {0xBB, 0x00, 0x03, 0x00, 0x03, 0x7E};
  sendCommand(versionCmd, sizeof(versionCmd));
  readResponse(1000);
}

void testCommunication() {
  Serial.println("\n🔧 TEST DE COMUNICACIÓN");
  
  // Probar diferentes estructuras de comando
  Serial.println("1. Comando simple con checksum 0x00:");
  byte test1[] = {0xBB, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7E};
  sendCommand(test1, sizeof(test1));
  readResponse(1000);
  
  delay(500);
  
  Serial.println("2. Comando eco/reset:");
  byte test2[] = {0xBB, 0x00, 0x02, 0x00, 0x02, 0x7E};
  sendCommand(test2, sizeof(test2));
  readResponse(1000);
}

void debugMode() {
  Serial.println("\n🐛 MODO DEBUG - ENVIAR COMANDOS MANUALES");
  Serial.println("Formato: BB 00 03 00 03 7E (sin 0x, separados por espacios)");
  Serial.println("Escribe 'exit' para salir");
  
  while(true) {
    if (Serial.available()) {
      String input = Serial.readStringUntil('\n');
      input.trim();
      
      if (input.equalsIgnoreCase("exit")) break;
      
      // Procesar comando manual
      processManualCommand(input);
    }
    delay(100);
  }
}

void processManualCommand(String hexStr) {
  // Convertir string a bytes
  hexStr.toUpperCase();
  hexStr.replace("0X", "");
  hexStr.replace(" ", "");
  
  if (hexStr.length() % 2 != 0) {
    Serial.println("❌ Error: Longitud impar");
    return;
  }
  
  int len = hexStr.length() / 2;
  byte cmd[len];
  
  for (int i = 0; i < len; i++) {
    String byteStr = hexStr.substring(i * 2, i * 2 + 2);
    cmd[i] = (byte)strtol(byteStr.c_str(), NULL, 16);
  }
  
  Serial.print("➡️  Enviando: ");
  sendCommand(cmd, len);
  readResponse(2000);
}

void sendCommand(byte* cmd, int length) {
  for (int i = 0; i < length; i++) {
    SerialRFID.write(cmd[i]);
    if (cmd[i] < 0x10) Serial.print("0");
    Serial.print(cmd[i], HEX);
    Serial.print(" ");
  }
  Serial.println();
  SerialRFID.flush();
}

void readResponse(int timeout) {
  Serial.print("📨 Respuesta: ");
  
  String hexResponse = "";
  unsigned long startTime = millis();
  bool frameComplete = false;
  int bytesReceived = 0;
  
  while (millis() - startTime < timeout) {
    if (SerialRFID.available()) {
      byte b = SerialRFID.read();
      bytesReceived++;
      
      // Mostrar en HEX
      if (b < 0x10) hexResponse += "0";
      hexResponse += String(b, HEX);
      hexResponse += " ";
      
      // Detectar fin de frame
      if (b == 0x7E) {
        frameComplete = true;
        // No break inmediatamente, leer todo lo disponible
      }
      
      // Pequeña pausa para dejar que lleguen más datos
      delay(10);
    } else if (frameComplete) {
      // Si ya tenemos frame completo y no hay más datos, salir
      break;
    }
  }
  
  if (bytesReceived > 0) {
    Serial.println(hexResponse);
    analyzeResponse(hexResponse);
  } else {
    Serial.println("SIN RESPUESTA");
  }
  Serial.println("------------------------------------");
}

void analyzeResponse(String response) {
  response.toLowerCase();
  
  // Buscar errores específicos
  if (response.indexOf("bb 01 ff") >= 0) {
    Serial.println("❌ ERROR DETECTADO");
    
    // Extraer código de error
    int errorStart = response.indexOf("01 ff 00 01");
    if (errorStart >= 0) {
      String errorCode = response.substring(errorStart + 11, errorStart + 13);
      Serial.print("📋 Código de error: 0x");
      Serial.println(errorCode);
      
      if (errorCode == "17") {
        Serial.println("💡 0x17: Error de formato de comando o parámetro inválido");
        Serial.println("   - Verificar estructura del comando");
        Serial.println("   - Revisar cálculo de checksum");
        Serial.println("   - Posible comando no soportado");
      }
    }
  }
  
  // Buscar respuestas exitosas
  if (response.indexOf("bb 00") >= 0 && response.indexOf("ff") == -1) {
    Serial.println("✅ POSIBLE RESPUESTA EXITOSA");
    
    // Intentar extraer datos útiles
    if (response.length() > 20) {
      Serial.println("📦 Respuesta con datos - posible tag detectado");
    }
  }
  
  // Buscar patrones específicos
  if (response.indexOf("f1") >= 0 || response.indexOf("22") >= 0) {
    Serial.println("🔍 Respuesta en modo raw - posible necesidad de inicialización");
  }
}
#include <HardwareSerial.h>
#include <SPI.h>
#include <MFRC522.h>
#include "HX711.h"

/////////////////////////////////////////////////
// RFID UHF (JRD-4035) - UART1
/////////////////////////////////////////////////
HardwareSerial SerialRFID(1);

/////////////////////////////////////////////////
// RFID RC522 - SPI
/////////////////////////////////////////////////
#define SS_PIN   5
#define RST_PIN  4
MFRC522 mfrc522(SS_PIN, RST_PIN);

/////////////////////////////////////////////////
// HX711 - BALANZA
/////////////////////////////////////////////////
#define LOADCELL_DOUT_PIN 32
#define LOADCELL_SCK_PIN  33

HX711 scale;
float factorCalibracion = -500.49;

/////////////////////////////////////////////////

void setup() {
  // Monitor serial
  Serial.begin(115200);
  while (!Serial);

  // RFID UHF
  SerialRFID.begin(115200, SERIAL_8N1, 16, 17);

  // HX711
  scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);
  scale.set_scale(factorCalibracion);
  scale.tare();

  // SPI + RC522
  SPI.begin(18, 19, 23, SS_PIN);
  mfrc522.PCD_Init();

 // Serial.println("ESP32 listo");
 // Serial.println("Comandos disponibles:");
 // Serial.println("  P -> Leer balanza HX711");
 // Serial.println("  R -> Leer etiquetas RFID UHF");
 // Serial.println("  C -> Leer tarjeta RFID RC522");
 // Serial.println("--------------------------------");
}

void loop() {
  if (Serial.available()) {
    String cmd = Serial.readStringUntil('\n');
    cmd.trim();
    cmd.toUpperCase();

    if (cmd == "P") {
      leerBalanza();
    }
    else if (cmd == "R") {
      realizarLectura();
    }
    else if (cmd == "C") {
      leerRC522();
    }
    else {
      Serial.println("Comando no reconocido");
    }
  }
}

/////////////////////////////////////////////////
// BALANZA HX711
/////////////////////////////////////////////////
void leerBalanza() {
  scale.power_up();
  delay(200);

  float peso = scale.get_units(5);
  scale.power_down();

  Serial.print("peso total:");
  Serial.print(peso, 2);
  Serial.println(" gramos");
}

/////////////////////////////////////////////////
// RFID UHF (TU CÓDIGO ORIGINAL)
/////////////////////////////////////////////////
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
    //Serial.print("📥 Respuesta (");
    //Serial.print(byteCount);
    //Serial.print(" bytes): ");
    //Serial.println(hexResponse);
    
    // Extraer y mostrar todas las etiquetas únicas
    extraerYMostrarEtiquetasUnicas(hexResponse);
  } else {
    Serial.println("No hay respuesta del módulo");
  }
  //Serial.println("----------------------------------------");
  //Serial.println("Escribe R para otra lectura");
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
    Serial.println("🏷️ Etiquetas detectadas:");
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
/////////////////////////////////////////////////
// RFID RC522 (TARJETAS)
/////////////////////////////////////////////////
void leerRC522() {
 // Serial.println("Acerque una tarjeta RC522...");

  unsigned long inicio = millis();
  while (millis() - inicio < 10000) {

    if (mfrc522.PICC_IsNewCardPresent() &&
        mfrc522.PICC_ReadCardSerial()) {

      Serial.print("Card UID:");
      for (byte i = 0; i < mfrc522.uid.size; i++) {
        Serial.print(" ");
        if (mfrc522.uid.uidByte[i] < 0x10) Serial.print("0");
        Serial.print(mfrc522.uid.uidByte[i], HEX);
      }
      Serial.println();

      mfrc522.PICC_HaltA();
      return;
    }
  }

  Serial.println("No se detectó tarjeta RC522");
}

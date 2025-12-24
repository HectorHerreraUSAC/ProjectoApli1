#include <MFRC522.h>

//#include <Pushbutton.h>
//#define RelayAlarma 5
//#define Buzzer 4

#define led 2

#define RST_PIN	9    //Pin 9 para el reset del RC522
#define SS_PIN	10   //Pin 10 para el SS (SDA) del RC522

MFRC522 mfrc522(SS_PIN, RST_PIN); //Creamos el objeto para el RC522

void setup() {
	Serial.begin(9600); //Iniciamos la comunicación  serial
	SPI.begin();        //Iniciamos el Bus SPI
	mfrc522.PCD_Init(); // Iniciamos  el MFRC522
	Serial.println("Iniciando lector de tarjetas");
  pinMode(led, OUTPUT);
  pinMode(3, OUTPUT);
}

void loop() {

 if ( mfrc522.PICC_IsNewCardPresent()) 
        {  
  		      digitalWrite(led, HIGH); //enviar señal al microcontrolador
            digitalWrite(2, HIGH); //enviar señal al microcontrolador
            //Seleccionamos una tarjeta
            if ( mfrc522.PICC_ReadCardSerial()) 
            {
                  // Enviamos el UID
                  Serial.print("Card UID:");
                  for (byte i = 0; i < mfrc522.uid.size; i++) {
                          Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
                          Serial.print(mfrc522.uid.uidByte[i], HEX);   
                  } 
                  Serial.println();
                  Serial.println("El Pago ha sido exitoso.");
                  // Terminamos la lectura de la tarjeta  actual
                  mfrc522.PICC_HaltA();         
            }
            delay(3000);      
	}
  else
  {
     digitalWrite(led, LOW); //enviar señal al microcontrolador
     digitalWrite(2, LOW); //enviar señal al microcontrolador
  }	

}

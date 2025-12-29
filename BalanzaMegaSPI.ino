////////////////////////////////////////////////////////////////////////////////MFRC522////////////////////////////////////////////////////////////////////////////////
#include <MFRC522.h>

#define RST_PIN	4    //Pin 4 para el reset del RC522
#define SS_PIN	53   //Pin 10 para el SS (SDA) del RC522 // Cambiar al pin 53 para el arduino mega

MFRC522 mfrc522(SS_PIN, RST_PIN); //Creamos el objeto para el RC522

////////////////////////////////////////////////////////////////////////////////HX711////////////////////////////////////////////////////////////////////////////////

//#include <Arduino.h> 
#include "HX711.h"                //Incluir librería del módulo HX711
//todos los pines son digitales a menos que se indique lo contrario.
const int LOADCELL_DOUT_PIN = 2;  //Pin DOUT del módulo HX
const int LOADCELL_SCK_PIN = 3;   //Pin sck del módulo HX

HX711 scale;  //Crear objeto Escala

////////////////////////////////////////////////////////////////////////////////Variables////////////////////////////////////////////////////////////////////////////////

float pesoCarrito = 1;      //Precio predeterminado de punto flotante 

float margen = 10; //margen de error medido en gramos
float pesoSuperior = pesoCarrito + margen;
float pesoInferior = pesoCarrito - margen;

bool estaApagado = false;   //Bool para el cambio de estado del módulo
bool estaEncendido = false; //Bool para el cambio de estado del módulo

bool haSidoPesado = false;    //Bool para saber si el carrito has sido pesado
bool pesoEsCorrecto = false;  //Bool para saber si el peso es correcto

bool puedePagar = false;
bool ApagarAlarma = false; // cambiar a true para que siempre se ejecute a menos que se diga lo contrario

////////////////////////////////////////////////////////////////////////////////Otros pines////////////////////////////////////////////////////////////////////////////////

#define pinMas 11       //Pin para el boton de avance
//#define pinMenos 12   //Pin para el boton de retroceso
#define pinLlave 12     //Definir pin para el switch con llave
#define pinRelay 5      //Definir pin para el modulo relay que activa la luz estroboscópica
#define pinTarjeta 6

#define LedAzul 8
#define LedRojo 9
#define LedVerde 10 

////////////////////////////////////////////////////////////////////////////////PUSHBUTTON////////////////////////////////////////////////////////////////////////////////

#include <Pushbutton.h>           //Incluir librería pushbutton
Pushbutton BotonMas(pinMas);      //Crear objeto pushbutton para avance
//Pushbutton BotonMenos(pinMenos);//Crear objeto pushbutton para retroceso
Pushbutton Llave(pinLlave);  //Crear objeto pushbutton para el switch con llave
Pushbutton Tarjeta(pinTarjeta);

int Indice = 0;             //Punto de partida para navegación
int intentos = 3;           //Iniciar con 3 intentos para escanear correctamente el contenido
//const int totalBloques = 2; //Total de bloques de navegación

void setup() {

  Serial.begin(57600);                    //El puerto debe de ser inicializado a 57600 baudios para el modulo HX, de lo contrario no funciona
  Serial.println("Iniciando la balanza"); 

  scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);

  Serial.println("Antes de calibrar la balanza:");
  Serial.print("read: \t\t");
  Serial.println(scale.read());                  //Lectura directa del modulo

  Serial.print("read average: \t\t");
  Serial.println(scale.read_average(10));        //Promedio de 10 lecturas

  Serial.print("get value: \t\t");
  Serial.println(scale.get_value(5));            //Promedio de 5 lecturas

  Serial.print("get units: \t\t");
  Serial.println(scale.get_units(5), 1);         //Promedio de 5 lecturas en unidades
            
  scale.set_scale(-500.49);                      //obtenido de calibrar un peso de 400 gramos con la celda de 5kg.

  scale.tare();                                  //Tarar

  Serial.println("Después de calibrar la balanza:");

  Serial.print("read: \t\t");
  Serial.println(scale.read());                 //Lectura directa de la celda

  Serial.print("read average: \t\t");
  Serial.println(scale.read_average(10));       //Promedio de 10 lecturas

  Serial.print("get value: \t\t");
  Serial.println(scale.get_value(5));           //Promedio de 5 lecturas

  Serial.print("get units: \t\t");
  Serial.println(scale.get_units(5), 1);        //Imprime el promedio de 5 lecturas  una vez tarado. Este valor deberia ser lo más cercano a 0.

	Serial2.begin(9600); //Iniciamos la comunicación  serial
	SPI.begin();        //Iniciamos el Bus SPI
	mfrc522.PCD_Init(); // Iniciamos  el MFRC522
  if (Serial2.available()) {
	Serial.println("Iniciando lector de tarjetas");
  }

  
  //Declarar pines para leds indicadores  
  
  pinMode(LedAzul, OUTPUT);
  pinMode(LedRojo, OUTPUT);
  pinMode(LedVerde, OUTPUT);
  pinMode(pinRelay, OUTPUT);  //Declarar relay como output

}

void loop()
{
    Serial.println("Nuevo inicio el programa");

  pesar();

  alarma();

    Serial.println("Empezando el pago de carrito.");

  pagar();

    Serial.println("Programa terminado, regresando al inicio del programa.");

}

void pesar()
{
  //Solicitar a la base de datos el peso

  digitalWrite(pinRelay, HIGH);
  ApagarAlarma = false;
  while (intentos > 0) {
  if (BotonMas.getSingleDebouncedPress()) {
      estaApagado = false; //devolver el flag de estaApagado a false para poder volver a enviar la señal de apagado. 
      
      if(!estaEncendido)  //Enviar una señal de encendido una sola vez. Checa por el opuesto de estaEncendido que por defecto es false 
      {
      scale.power_up();
      estaEncendido = true; //Cambiar el estado de estaEncendido.
      }
      if(!haSidoPesado)    //Checa por el opuesto de haSidoPesado, por defecto false.  
      {
        Serial.println("Pesando, por favor espere...");
        pesoCarrito = scale.get_units(5);   //Si no ha sido pesado, obtener el peso promedio de 10 lecturas redondeado 2 decimales de gramo.
          Serial.print("El carrito ha sido pesado, el peso es de ");
          Serial.print(pesoCarrito, 2);     
          Serial.println(" gramos.");
            //Enviar el peso obtenido de la balanza devuelta a control
            if(pesoCarrito<pesoSuperior && pesoCarrito >pesoInferior){
            //if (pesoCarrito > pesoTeorico) { 
            intentos = 0;             //Condición para salir del bucle
//          pesoEsCorrecto = true;
            puedePagar = true;
            ApagarAlarma = true;
            }
            else {
            intentos--;
            }
        haSidoPesado = true;                //Cambiar el estado de haSidoPesado a true
      }
      digitalWrite(LedVerde, HIGH);  
      digitalWrite(LedAzul, LOW);

  }
  else {
        haSidoPesado = false;
        estaEncendido = false;
        if(!estaApagado)  //Enviar una señal de apagado una sola vez. Checa por el opuesto de estaApagado que por defecto es false 
      {
        scale.power_down(); 
        Serial.println("Porfavor remueva cualquier objeto. Luego, coloque los productos en la balanza");
          Serial.print("Usted tiene ");
          Serial.print(intentos);
          Serial.print(" intentos restantes.");
        estaApagado = true;   //Cambiar el estado de estaApagado para evitar que esta instrucción se ejecute más de una vez
      }
      digitalWrite(LedVerde, LOW); //LED indicador
      digitalWrite(LedAzul, HIGH);

      }
  }

  estaApagado = false;
  digitalWrite(LedVerde, LOW);  //LED indicador
  digitalWrite(LedRojo, LOW);   //Apagar los Leds
  digitalWrite(LedAzul, LOW);

}

void alarma()
{
  Serial.println("Encendiendo la alarma");
 
  while (!ApagarAlarma) {
    digitalWrite(LedRojo, HIGH);
    digitalWrite(pinRelay, LOW);              //Cambia el estado del relay que enciende la luz estroboscopica
      if (Llave.getSingleDebouncedPress()) {  //Si el interruptor de seguridad es presionado 1 vez
      intentos = 3;                           //Reiniciar el contador para la cantidad de intentos
      ApagarAlarma = true;                    //Se debe de devolver este flag para que el codigo vuelva a ejecutarse. 
      puedePagar = false;                     //Asegurarse que aún no pueda pagar
      digitalWrite(LedVerde, HIGH);
      Serial.println("Una persona autorizada acaba de reiniciar el sistema. Usted ahora puede continuar con el programa.");
      }
  }
  //Reiniciando las variables
  digitalWrite(pinRelay, HIGH);  //devolver el estado del relay
  digitalWrite(LedRojo, LOW);

}

void pagar()
{
  if(puedePagar)
  {
    Serial.println("Por favor, acerce una tarjeta compatible al módulo de lectura.");
  }

  while(puedePagar)
  {
     if (mfrc522.PICC_IsNewCardPresent()) 
        {  
  		      digitalWrite(LedAzul, HIGH); //enviar señal al microcontrolador
            //digitalWrite(2, HIGH); //enviar señal al microcontrolador
            //Seleccionamos una tarjeta
            if ( mfrc522.PICC_ReadCardSerial()) 
            {
                  // Enviamos el UID
                  Serial.print("Card UID:");
                  for (byte i = 0; i < mfrc522.uid.size; i++) {
                          Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
                          Serial.print(mfrc522.uid.uidByte[i], HEX);   
                  }
                  Serial.println(""); 
                  Serial.println("Pago exitoso. Gracias por su compra!");
                  puedePagar = false;
                  intentos = 3;
                  delay(500);
                  mfrc522.PICC_HaltA();         
            }
	}
}
digitalWrite(LedAzul, LOW); //enviar señal al microcontrolador
}
// void reiniciarVars()
// {

// Serial.println(estaApagado);
// Serial.println(estaEncendido);
// Serial.println(haSidoPesado);
// Serial.println(pesoEsCorrecto);
// Serial.println(puedePagar);
// Serial.println(ApagarAlarma);

// }

////////////////////////////////////////////////////////////////////////////////HX711////////////////////////////////////////////////////////////////////////////////

//#include <Arduino.h> 
#include "HX711.h"                //Incluir librería del módulo HX711
//todos los pines son digitales a menos que se indique lo contrario.
const int LOADCELL_DOUT_PIN = 2;  //Pin DOUT del módulo HX
const int LOADCELL_SCK_PIN = 3;   //Pin sck del módulo HX

HX711 scale;  //Crear objeto Escala

////////////////////////////////////////////////////////////////////////////////Variables////////////////////////////////////////////////////////////////////////////////

float pesoCarrito = 0;      //Precio predeterminado de punto flotante 
float pesoTeorico = 1;

bool estaApagado = false;   //Bool para el cambio de estado del módulo
bool estaEncendido = false; //Bool para el cambio de estado del módulo

bool haSidoPesado = false;    //Bool para saber si el carrito has sido pesado
bool pesoEsCorrecto = false;  //Bool para saber si el peso es correcto

bool puedePagar = false;
bool ApagarAlarma = false;

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

int Indice = 0;             //Punto de partida para navegación
int intentos = 3;           //Iniciar con 3 intentos para escanear correctamente el contenido
//const int totalBloques = 2; //Total de bloques de navegación

void setup() {

  Serial.begin(57600);                    //El puerto debe de ser inicializado a 57600 baudios para el modulo HX, de lo contrario no funciona
  Serial.println("Initializing the scale"); 

  scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);

  Serial.println("Before setting up the scale:");
  Serial.print("read: \t\t");
  Serial.println(scale.read());      // print a raw reading from the ADC

  Serial.print("read average: \t\t");
  Serial.println(scale.read_average(10));   // print the average of 20 readings from the ADC

  Serial.print("get value: \t\t");
  Serial.println(scale.get_value(5));   // print the average of 5 readings from the ADC minus the tare weight (not set yet)

  Serial.print("get units: \t\t");
  Serial.println(scale.get_units(5), 1);  // print the average of 5 readings from the ADC minus tare weight (not set) divided
            // by the SCALE parameter (not set yet)
            
  scale.set_scale(-500.49); //obtenido de calibrar un peso de 400 gramos con la celda de 5kg.
  // this value is obtained by calibrating the scale with known weights; see the README for details
  scale.tare();               // reset the scale to 0

  Serial.println("After setting up the scale:");

  Serial.print("read: \t\t");
  Serial.println(scale.read());                 // print a raw reading from the ADC

  Serial.print("read average: \t\t");
  Serial.println(scale.read_average(10));       // print the average of 20 readings from the ADC

  Serial.print("get value: \t\t");
  Serial.println(scale.get_value(5));   // print the average of 5 readings from the ADC minus the tare weight, set with tare()

  Serial.print("get units: \t\t");
  Serial.println(scale.get_units(5), 1);        //Imprime el promedio de 5 lecturas menos una vez tarado. Este valor deberia ser lo más cercano a 0.

  Serial.println("Readings:");
  //Declarar pines para leds indicadores
  pinMode(LedAzul, OUTPUT);
  pinMode(LedRojo, OUTPUT);
  pinMode(LedVerde, OUTPUT);
  pinMode(pinRelay, OUTPUT);  //Declarar relay como output

}

void loop()
{
    Serial.println("Empezando el programa de peso.");

  pesar();

  alarma();

    Serial.println("Empezando el programa de pago de carrito.");

  pagar();

    Serial.println("Programa terminado, regresando al inicio del programa.");
}

void pesar()
{
  digitalWrite(pinRelay, HIGH);
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
            if (pesoCarrito < 200) {  //Empezando haciendo pruebas con un valor ridiculamente alto
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
  Serial.println("Iniciando medidas de seguridad.");
  while (!ApagarAlarma) {
    digitalWrite(LedRojo, HIGH);
    digitalWrite(pinRelay, LOW);             //Cambia el estado del relay que enciende la luz estroboscopica
      if (Llave.getSingleDebouncedPress()) {  //Si el interruptor de seguridad es presionado 1 vez
      intentos = 3; //Reiniciar el contador para la cantidad de intentos
      ApagarAlarma = true;
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
   Serial.println("Ha llegado al final del programa.");
  //Serial.println("Usted debe el monto de Q.xxx. Porfavor acerce su tarjeta al lector.");
  while(puedePagar)
  {
    // if(PinTarjeta == HIGH)
    // {
    //   Serial.print("Pago exitoso. Gracias por su compra!");
    // }
   digitalWrite(LedAzul, HIGH);
  }
}

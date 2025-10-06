////////////////////////////////////////////////////////////////////////////////HX711////////////////////////////////////////////////////////////////////////////////

//#include <Arduino.h> 
#include "HX711.h" //Incluir librería del módulo HX711
//todos los pines son digitales a menos que se indique lo contrario.
const int LOADCELL_DOUT_PIN = 2;  //Pin DOUT del módulo HX
const int LOADCELL_SCK_PIN = 3;   //Pin sck del módulo HX

HX711 scale;  //Crear objeto Escala

float pesoCarrito = 0; //Precio predeterminado de punto flotante 

bool estaApagado = false;   //Bool para el cambio de estado del módulo
bool estaEncendido = false; //Bool para el cambio de estado del módulo

bool haSidoPesado = false;  //Bool para saber si el carrito has sido pesado
////////////////////////////////////////////////////////////////////////////////PUSHBUTTON////////////////////////////////////////////////////////////////////////////////
#include <Pushbutton.h>     //Incluir librería pushbutton

#define pinMas 11   //Pin para el boton de avance
#define pinMenos 12 //Pin para el boton de retroceso

Pushbutton BotonMas(pinMas);    //Crear objeto pushbutton para avance
Pushbutton BotonMenos(pinMenos);//Crear objeto pushbutton para retroceso

int Indice = 0; //Punto de partida para navegación
const int totalBloques = 2; //Total de bloques de navegación

void setup() {

   Serial.begin(57600); //El puerto debe de ser inicializado a 57600 baudios para el modulo HX, de lo contrario no funciona
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
            
  scale.set_scale(223.875); //obtenido de calibrar un peso de 28.9gramos
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
  Serial.println(scale.get_units(5), 1);        // print the average of 5 readings from the ADC minus tare weight, divided
            // by the SCALE parameter set with set_scale

  Serial.println("Readings:");
  //Declarar pines para leds indicadores
  pinMode(10, OUTPUT);
  pinMode(9, OUTPUT);
  pinMode(8, OUTPUT);

}
void loop()
{
if(BotonMas.getSingleDebouncedPress())  //getSingleDebouncePress monitorea el cambio de estado de abierto a cerrado una sola vez
  {
    Indice++;         //Si se cumple, incrementar el valor Indice por uno una sola vez por cambio de estado.
    if (Indice > 2)   //Prevee que el valor se salga del rango
    {
      Indice = 0;
    }
  }
  if (BotonMenos.getSingleDebouncedPress())
  {
  Indice--;
  if (Indice<0)   //Prevee que el valor sea negativo
    {
    Indice = totalBloques;
    }
  }
 ejecutarBloque(Indice); //Constantemete checa el valor de Indice para saber que bloque ejecutar
}
 

void ejecutarBloque(int i) {
  while(intentos <= 3)
  {

   
  }

  switch (i) {
    case 0: //Estado: reposo
    haSidoPesado = false;
    estaEncendido = false;

      if(!estaApagado)  //Enviar una señal de apagado una sola vez. Checa por el opuesto de estaApagado que por defecto es false 
      {
        scale.power_down(); 
        Serial.println("Esperando. Porfavor ponga los productos en posición...");
        estaApagado = true; //Cambiar el estado de estaApagado para evitar que esta instrucción se ejecute más de una vez
      }
      digitalWrite(10, HIGH); //LED indicador
      digitalWrite(9, LOW); //Apagar los otros leds
      digitalWrite(8, LOW);
    break;

    case 1: //Estado: lectura de peso
    
    estaApagado = false; //devolver el flag de estaApagado a false para poder volver a enviar la señal de apagado.
    
    if(!estaEncendido)  //Enviar una señal de encendido una sola vez. Checa por el opuesto de estaEncendido que por defecto es false 
    {
    scale.power_up();
    estaEncendido = true; //Cambiar el estado de estaEncendido.
    }
    
    if(!haSidoPesado)    //Checa por el opuesto de haSidoPesado, por defecto false.  
    {
      Serial.println("Pesando, por favor espere...")
      pesoCarrito = scale.get_units(10)   //Si no ha sido pesado, obtener el peso promedio de 10 lecturas redondeado 2 decimales de gramo.
      Serial.println("El carrito ha sido pesado, el peso (g) es: ")
      Serial.println(pesoCarrito, 2);     
      haSidoPesado = true;                //Cambiar el estado de haSidoPesado a true
    }

    if(pesoCarrito > pesoAnalitico)
    {
      El peso no es correcto, por favor vuelva a pesar, usted tiene 2 intentos 
      intentos--
      No se ha podido confirmar el peso de su carrito. Por favor espera a que un operador le asista
    }
    else
    {
      void pago()
    }
    if(llave == HIGH)
    {
      intentos = 0;
    }
    digitalWrite(9, HIGH);  //Encender el led indicador
    digitalWrite(10, LOW);  
    digitalWrite(8, LOW);
      break;
    case 2: //
      digitalWrite(8, HIGH);
      digitalWrite(9, LOW);
      digitalWrite(10, LOW);
      break;
  }
}

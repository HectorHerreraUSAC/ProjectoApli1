////////////////////////////////////////////////////////////////////////////////HX711////////////////////////////////////////////////////////////////////////////////

//#include <Arduino.h> 
#include "HX711.h" //Incluir librería del módulo HX711
//todos los pines son digitales a menos que se indique lo contrario.
const int LOADCELL_DOUT_PIN = 2;  //Pin DOUT del módulo HX
const int LOADCELL_SCK_PIN = 3;   //Pin sck del módulo HX

HX711 scale;  //Crear objeto Escala

float pesoCarrito = 0; //Precio predeterminado de punto flotante 
float pesoTeorico = 1;

bool estaApagado = false;   //Bool para el cambio de estado del módulo
bool estaEncendido = false; //Bool para el cambio de estado del módulo

bool haSidoPesado = false;  //Bool para saber si el carrito has sido pesado
bool pesoEsCorrecto = false;

bool puedeContinuar = false;
bool Reinicio = false;

////////////////////////////////////////////////////////////////////////////////PUSHBUTTON////////////////////////////////////////////////////////////////////////////////
#include <Pushbutton.h>     //Incluir librería pushbutton

#define pinMas 11   //Pin para el boton de avance
#define pinMenos 12 //Pin para el boton de retroces
#define pinLlave 10

Pushbutton BotonMas(pinMas);    //Crear objeto pushbutton para avance
Pushbutton BotonMenos(pinMenos);//Crear objeto pushbutton para retroceso
Pushbutton LlaveAdmin(pinLlave);

int Indice = 0; //Punto de partida para navegación
int intentos = 3; //Iniciar con 3 intentos para escanear correctamente el contenido
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
            
  scale.set_scale(-504.775); //obtenido de calibrar un peso de 400 gramos con la celda de 5kg.
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
  pinMode(10, OUTPUT);
  pinMode(9, OUTPUT);
  pinMode(8, OUTPUT);

}
// void BotonIndice()
// {
// if(BotonMas.getSingleDebouncedPress())  //getSingleDebouncedPress monitorea el cambio de estado de abierto a cerrado una sola vez
//   {
//     Indice++;         //Si se cumple, incrementar el valor Indice por uno una sola vez por cambio de estado.
//     if (Indice > 2)   //Prevee que el valor se salga del rango
//     {
//       Indice = 0;
//     }
//   }
//   if (BotonMenos.getSingleDebouncedPress())
//   {
//   Indice--;
//   if (Indice<0)   //Prevee que el valor sea negativo
//     {
//     Indice = totalBloques;
//     }
//   }
// }

void loop()
{
  
}

void loop()
{
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
            if (pesoCarrito < 200) { //Empezando haciendo pruebas con un valor ridiculamente alto
            pesoEsCorrecto = true;
            intentos = 0;
            }
            else {
            intentos--;
            }
        haSidoPesado = true;                //Cambiar el estado de haSidoPesado a true
      }
      digitalWrite(9, LOW);  //Encender el led indicador
      digitalWrite(10, HIGH);  
      digitalWrite(8, LOW);
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
      digitalWrite(10, LOW); //LED indicador
      digitalWrite(9, LOW);   //Apagar los otros leds
      digitalWrite(8, HIGH);
  }
  }

  Continuar();
  
}

void Continuar()
{
  while (pesoEsCorrecto == true) {
    digitalWrite(10, HIGH);
    delay(400);
    digitalWrite(10, LOW);
    delay(400);
  }
   while ( pesoEsCorrecto == false) {
    digitalWrite(9, HIGH);
    delay(200);
    digitalWrite(9, LOW);
    delay(200);
  }
  // while (!Reinicio)
  // {
  // digitalWrite(RelayAlarma, HIGH);
  // if (getSingleDebouncedRelease(pinLlave))
  //   {
  //   Reinicio = true;
  //   intentos = 0;
  //   }
  // }
}

// void ejecutarBloque(int i) {

//   switch (i) {
//     case 0: //Estado: reposo
//     haSidoPesado = false;
//     estaEncendido = false;

//       if(!estaApagado)  //Enviar una señal de apagado una sola vez. Checa por el opuesto de estaApagado que por defecto es false 
//       {
//         scale.power_down(); 
//         Serial.println("Porfavor ponga los productos en posición...");
//         estaApagado = true; //Cambiar el estado de estaApagado para evitar que esta instrucción se ejecute más de una vez
//       }
//       digitalWrite(10, HIGH); //LED indicador
//       digitalWrite(9, LOW); //Apagar los otros leds
//       digitalWrite(8, LOW);
//     break;

//     case 1: //Estado: lectura de peso
    
//     estaApagado = false; //devolver el flag de estaApagado a false para poder volver a enviar la señal de apagado. 
//     if(!estaEncendido)  //Enviar una señal de encendido una sola vez. Checa por el opuesto de estaEncendido que por defecto es false 
//     {
//     scale.power_up();
//     estaEncendido = true; //Cambiar el estado de estaEncendido.
//     }
//     if(!haSidoPesado)    //Checa por el opuesto de haSidoPesado, por defecto false.  
//     {
//       Serial.println("Pesando, por favor espere...");
//       pesoCarrito = scale.get_units(10);   //Si no ha sido pesado, obtener el peso promedio de 10 lecturas redondeado 2 decimales de gramo.
//       Serial.println("El carrito ha sido pesado, el peso es de");
//       Serial.print(pesoCarrito, 2);     
//       Serial.print("gramos.");
      
//       haSidoPesado = true;                //Cambiar el estado de haSidoPesado a true
//     }
//     digitalWrite(9, HIGH);  //Encender el led indicador
//     digitalWrite(10, LOW);  
//     digitalWrite(8, LOW);
//       break;
//     case 2: //
//       digitalWrite(8, HIGH);
//       digitalWrite(9, LOW);
//       digitalWrite(10, LOW);
//       break;
//   }
// }

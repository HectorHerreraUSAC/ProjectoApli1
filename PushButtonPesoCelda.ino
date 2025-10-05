////////////////////////////////////////////////////////////////////////////////HX711////////////////////////////////////////////////////////////////////////////////

#include <Arduino.h>
#include "HX711.h"

// HX711 circuit wiring
const int LOADCELL_DOUT_PIN = 2;
const int LOADCELL_SCK_PIN = 3;

HX711 scale;

bool estaApagado = false;
bool estaEncendido = false;
////////////////////////////////////////////////////////////////////////////////PUSHBUTTON////////////////////////////////////////////////////////////////////////////////
#include <Pushbutton.h>

#define pinMas 11
#define pinMenos 12

Pushbutton BotonMas(pinMas);
Pushbutton BotonMenos(pinMenos);

int Indice = 0;
const int totalBloques = 2;

void setup() {

   Serial.begin(57600);
  Serial.println("Initializing the scale");

  scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);

  Serial.println("Before setting up the scale:");
  Serial.print("read: \t\t");
  Serial.println(scale.read());      // print a raw reading from the ADC

  Serial.print("read average: \t\t");
  Serial.println(scale.read_average(20));   // print the average of 20 readings from the ADC

  Serial.print("get value: \t\t");
  Serial.println(scale.get_value(5));   // print the average of 5 readings from the ADC minus the tare weight (not set yet)

  Serial.print("get units: \t\t");
  Serial.println(scale.get_units(5), 1);  // print the average of 5 readings from the ADC minus tare weight (not set) divided
            // by the SCALE parameter (not set yet)
            
  scale.set_scale(-359.723); //obtenido de calibrar un peso de 28.9gramos
  // this value is obtained by calibrating the scale with known weights; see the README for details
  scale.tare();               // reset the scale to 0

  Serial.println("After setting up the scale:");

  Serial.print("read: \t\t");
  Serial.println(scale.read());                 // print a raw reading from the ADC

  Serial.print("read average: \t\t");
  Serial.println(scale.read_average(20));       // print the average of 20 readings from the ADC

  Serial.print("get value: \t\t");
  Serial.println(scale.get_value(5));   // print the average of 5 readings from the ADC minus the tare weight, set with tare()

  Serial.print("get units: \t\t");
  Serial.println(scale.get_units(5), 1);        // print the average of 5 readings from the ADC minus tare weight, divided
            // by the SCALE parameter set with set_scale

  Serial.println("Readings:");

  pinMode(10, OUTPUT);
  pinMode(9, OUTPUT);
  pinMode(8, OUTPUT);

}
void loop()
{
if(BotonMas.getSingleDebouncedPress())
  {
    Indice++;
    if (Indice > 2)
    {
      Indice = 0;
    }
  }
if (BotonMenos.getSingleDebouncedPress()) {
Indice--;
if (Indice<0) {

Indice = totalBloques;
}
}
 ejecutarBloque(Indice);
}
 

void ejecutarBloque(int i) {
  switch (i) {
    case 0:
    estaEncendido = false;
    if(!estaApagado)
    {
      scale.power_down(); 
      Serial.println("Esperando. Porfavor ponga los productos en posición...");
      estaApagado = true;
    }
      digitalWrite(10, HIGH);
      digitalWrite(9, LOW);
      digitalWrite(8, LOW);
    break;
    case 1:
    estaApagado = false;
    if(!estaEncendido)
    {
    scale.power_up();
    estaEncendido = true;
    }
    
     Serial.print("\t| average:\t");
     Serial.println(scale.get_units(10), 2);
     delay(50);
      digitalWrite(9, HIGH);
      digitalWrite(10, LOW);
      digitalWrite(8, LOW);
      break;
    case 2:
      digitalWrite(8, HIGH);

      digitalWrite(9, LOW);
      digitalWrite(10, LOW);
      break;
  }
}

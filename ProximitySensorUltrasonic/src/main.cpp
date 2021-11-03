/*
     Autor:      Vitor Baccin
     Link:       www.ducknds.org
     Compre/Buy: https://s.click.aliexpress.com/e/_A5RjGb
     Sketch:     ******
     Sensor:     AJ-SR04M
     Data:       11/06/2021
*/
#include <Arduino.h>
#include <ultrasonic.h>

#define TRIG_PIN 4 //Pino 4 do Arduino será a saída de trigger RX
#define ECHO_PIN 5 //Pino 5 do Arduino será a entrada de echo TX

Ultrasonic ultrasonic(TRIG_PIN, ECHO_PIN);

void setup()
{
  Serial.begin(115200);
  ultrasonic.setTimeout(22500UL); // Para maximo 401cm
}

void loop()
{
  delay(500);
  int dist = ultrasonic.read();
  Serial.print("[Sensor] Distância: ");
  Serial.print(dist);
  Serial.println(" cm");
}
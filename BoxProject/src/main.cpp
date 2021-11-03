#include <SPI.h>
#include <Arduino.h>
#include <Wire.h>
#include <SparkFun_APDS9960.h>
//  Arduino Pin  APDS-9960 Board  Function
//  3.3V         VCC              Power (3.3V ok)
//  GND          GND              Ground
//  A4           SDA              I2C Data
//  A5           SCL              I2C Clock

SparkFun_APDS9960 APDS9960 = SparkFun_APDS9960();

uint16_t LUZ_AMBIENTE = 0;
uint16_t LUZ_VERMELHA = 0;
uint16_t LUZ_VERDE = 0;
uint16_t LUZ_AZUL = 0;

uint8_t PROXIMIDADE = 0;

#include <Adafruit_Sensor.h> // incluir librerias para sensor BMP280
#include <Adafruit_BMP280.h>
//(Vcc (3.3V) e GND), e os pinos de interface SPI: SCL(Clock) e SDA (Data), CSB (Chip Select) e SDO (Serial Data OUT).
// Arduino PIN   Display OLED     Sensor BMP280
// A4            SDA              SDI
// A5            SCL              SCL
// GND           GND              GND
// 5V            VCC              VCC e no SDD ( 3.3v ou 5v)
Adafruit_BMP280 BMP280; // cria objeto com nome BMP280

float TEMPERATURA; // variable para almacenar valor de temperatura
float PRESSAO, P0; // variables para almacenar valor de presion atmosferica
float ALTITUDE;    // y presion actual como referencia para altitud

#include <Adafruit_HTU21DF.h>
// Connect Vin to 3-5VDC (3.3V OK)
// Connect GND to ground
// Connect SCL to I2C clock pin (A5 on UNO)
// Connect SDA to I2C data pin (A4 on UNO)
Adafruit_HTU21DF HTU21DF = Adafruit_HTU21DF();

float TEMPERATURA_C; // valor de temperatura Celsius
float TEMPERATURA_F; // valor de temperatura Fahrenheit
float TEMPERATURA_K; // valor de temperatura Kelvin
float HUMIDADE;    // valor humidade

// Declaração das funções
void handleGesture();
float getHTU(char type);
void printDegree();

void setup()
{
  // Initialize Serial port
  Serial.begin(115200);
  Serial.println(F("APDS-9960 BMP280 HTU21DF "));
  // Initialize APDS-9960 (configure I2C and initial values)
  if (APDS9960.init())
  {
    Serial.println(F("APDS-9960 - Initialização completa"));
  }
  else
  {
    Serial.println(F("APDS-9960 - Algo deu errado na inicialização do sensor!"));
  }

  // Start running the APDS-9960 gesture sensor engine
  if (APDS9960.enableGestureSensor(false))
  {
    Serial.println(F("APDS-9960 - Sensor de gestos esta executando!"));
  }
  else
  {
    Serial.println(F("APDS-9960 - Algo deu errado na inicialização do sensor de gestos!"));
  }

  // Start running the APDS-9960 light sensor (no interrupts)
  if (APDS9960.enableLightSensor(false))
  {
    Serial.println(F("APDS-9960 - Sensor de luz esta executando!"));
  }
  else
  {
    Serial.println(F("APDS-9960 - Algo deu errado na inicialização do sensor de luz!"));
  }

  // Adjust the Proximity sensor gain
  if (!APDS9960.setProximityGain(PGAIN_2X))
  {
    Serial.println(F("Something went wrong trying to set PGAIN"));
  }

  // Start running the APDS-9960 proximity sensor (no interrupts)
  if (APDS9960.enableProximitySensor(false))
  {
    Serial.println(F("APDS-9960 - Sensor de proximidade esta executando!"));
  }
  else
  {
    Serial.println(F("APDS-9960 - Algo deu errado na inicialização do sensor de proximidade!"));
  }

  if (!BMP280.begin())
  {                                           // si falla la comunicacion con el sensor mostrar
    Serial.println("Sensor BMP280 não encontrado!"); // texto y detener flujo del programa
    while (1) delay(10); // mediante bucle infinito
  }

  P0 = BMP280.readPressure() / 100;

  if (!HTU21DF.begin())
  {
    Serial.println("Sensor HTU21DF não encontrado!!");
    while (1) delay(10);
  }
}

void loop()
{
  // APDS9960
  // Read the light levels (ambient, red, green, blue)
  if (!APDS9960.readAmbientLight(LUZ_AMBIENTE) ||
      !APDS9960.readRedLight(LUZ_VERMELHA) ||
      !APDS9960.readGreenLight(LUZ_VERDE) ||
      !APDS9960.readBlueLight(LUZ_AZUL))
  {
    Serial.println("Error reading light values");
  }
  else
  {
    Serial.print("Luz ambiente: ");
    Serial.print(LUZ_AMBIENTE);
    Serial.print(" Red: ");
    Serial.print(LUZ_VERMELHA);
    Serial.print(" Green: ");
    Serial.print(LUZ_VERDE);
    Serial.print(" Blue: ");
    Serial.println(LUZ_AZUL);
  }

  // Read the proximity value
  if (!APDS9960.readProximity(PROXIMIDADE))
  {
    Serial.println("Error reading proximity value");
  }
  else
  {
    Serial.print("Proximidade: ");
    Serial.println(PROXIMIDADE);
  }

  //BMP280
  TEMPERATURA = BMP280.readTemperature(); // almacena en variable el valor de temperatura
  PRESSAO = BMP280.readPressure() / 100;  // almacena en variable el valor de presion divido
  ALTITUDE = BMP280.readAltitude(P0);     // por 100 para covertirlo a hectopascales

  Serial.print("Temperatura: "); // muestra texto
  Serial.print(TEMPERATURA);     // muestra valor de la variable
  Serial.print(" C ");           // muestra letra C indicando grados centigrados

  Serial.print("Pressão: "); // muestra texto
  Serial.print(PRESSAO);     // muestra valor de la variable
  Serial.println(" hPa");    // muestra texto hPa indicando hectopascales

  Serial.print("Altitude aprox: "); // muestra texto
  Serial.print(ALTITUDE);           // muestra valor de altitud con referencia a P0
  Serial.println(" m");             // muestra letra m indicando metros

  // HTU21DF
  TEMPERATURA_C = getHTU('C'); // valor de temperatura Celsius
  TEMPERATURA_F = getHTU('F'); // valor de temperatura Fahrenheit
  TEMPERATURA_K = getHTU('K'); // valor de temperatura Kelvin
  HUMIDADE = getHTU('H'); // valor da humidade

  Serial.print(TEMPERATURA_C);
  printDegree();
  Serial.println("C");

  Serial.print(TEMPERATURA_F);
  printDegree();
  Serial.println("F");

  Serial.print(TEMPERATURA_K);
  Serial.println("K");
  Serial.println(" ");

  Serial.print("Humidade:");
  Serial.print(HUMIDADE);
  Serial.println("%");

  // Wait 10 second before next reading
  delay(10000);
}

void handleGesture()
{
  if (APDS9960.isGestureAvailable())
  {
    switch (APDS9960.readGesture())
    {
    case DIR_UP:
      Serial.println("UP");
      break;
    case DIR_DOWN:
      Serial.println("DOWN");
      break;
    case DIR_LEFT:
      Serial.println("LEFT");
      break;
    case DIR_RIGHT:
      Serial.println("RIGHT");
      break;
    case DIR_NEAR:
      Serial.println("NEAR");
      break;
    case DIR_FAR:
      Serial.println("FAR");
      break;
    default:
      Serial.println("NONE");
    }
  }
}

/*
 * @brief returns temperature or relative humidity
 * @param "type" is character
 *     C = Celsius
 *     K = Keliven
 *     F = Fahrenheit
 *     H = Humidity
 * @return returns one of the values above
 * Usage: to get Fahrenheit type: getHTU('F')
 * to print it on serial monitor Serial.println(getHTU('F'));
 */
float getHTU(char type)
{
  float value;
  float temp = HTU21DF.readTemperature();
  float rel_hum = HTU21DF.readHumidity();
  if (type == 'F')
  {
    value = temp * 9 / 5 + 32; //convert to Fahrenheit
  }
  else if (type == 'K')
  {
    value = temp + 273.15; //convert to Kelvin
  }
  else if (type == 'H')
  {
    value = rel_hum; //return relative humidity
  }
  else
  {
    value = temp; // return Celsius
  }
  return value;
}

/*
 * @brief prints degree symbol on serial monitor
 * @param none
 * @return returns nothing
 */
void printDegree()
{
  Serial.print("\xC2");
  Serial.print("\xB0");
}
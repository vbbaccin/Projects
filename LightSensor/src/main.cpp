/****************************************************************
APDS-9960 RGB and Gesture Sensor
https://github.com/sparkfun/APDS-9960_RGB_and_Gesture_Sensor

Hardware Connections:

IMPORTANT: The APDS-9960 can only accept 3.3V!

 Arduino Pin  APDS-9960 Board  Function
 3.3V         VCC              Power
 GND          GND              Ground
 A4           SDA              I2C Data
 A5           SCL              I2C Clock
****************************************************************/
#include <string>

#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h> /* strtoumax */
#include <stdbool.h>

#include <Wire.h>
#include <SparkFun_APDS9960.h>
#include <WiFi.h>
#include <PubSubClient.h>
//#include <WiFiClientSecure.h>
//#include <Adafruit_MQTT.h>
//#include <Adafruit_MQTT_Client.h>

#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7735.h> // Hardware-specific library for ST7735
#include <SPI.h>
#include <credentials.h>

// These pins will also work for the 1.8" TFT shield

// ESP32-WROOM
#define TFT_DC 12   // A0
#define TFT_CS 13   // CS
#define TFT_MOSI 14 // SDA
#define TFT_CLK 27  // SCK
#define TFT_RST 0
#define TFT_MISO 0

Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_MOSI, TFT_CLK, TFT_RST);

uint32_t CHIP_ID = 0;

// Global Variables para sensor
uint16_t LUZ_AMBIENTE = 0;
uint16_t LUZ_VERMELHA = 0;
uint16_t LUZ_VERDE = 0;
uint16_t LUZ_AZUL = 0;
uint8_t PROXIMIDADE = 0;

WiFiClient client_WiFi;
PubSubClient client_MQTT(client_WiFi);
// WiFiClientSecure client_WiFi_Secure;
// Adafruit_MQTT_Client client_MQTT_Secure(client_WiFi_Secure);
SparkFun_APDS9960 APDS9960 = SparkFun_APDS9960();
// WiFiUDP ntpClient;

// Declaração das Funções
void reconecta();      // Garante que as conexoes com WiFi e MQTT Broker se mantenham ativas
void conecta_WiFi();   // Faz conexão com WiFi
void conecta_MQTT();   // Faz conexão com Broker MQTT
void publica_status(); // Envia pacotes ao broker

void setup()
{
  // Initialize Serial port
  Serial.begin(115200);

  tft.initR(INITR_BLACKTAB);

  uint16_t time = millis();
  tft.fillScreen(ST77XX_BLACK);
  time = millis() - time;

  Serial.println(time, DEC);
  delay(500);

  // Initialize APDS-9960 (configure I2C and initial values)
  if (APDS9960.init())
  {
    Serial.println(F("APDS-9960 - Initialização completa"));
  }
  else
  {
    Serial.println(F("APDS-9960 - Algo deu errado na inicialização do sensor!"));
  }

  if (APDS9960.enableLightSensor(false))
  {
    Serial.println(F("APDS-9960 - Sensor de luz esta executando!"));
  }
  else
  {
    Serial.println(F("APDS-9960 - Algo deu errado na inicialização do sensor de luz!"));
  }

  if (!APDS9960.setProximityGain(PGAIN_4X))
  {
    Serial.println(F("APDS-9960 - Algo deu errado ao tentar configurar PGAIN"));
  }

  if (APDS9960.enableProximitySensor(false))
  {
    Serial.println(F("APDS-9960 - Sensor de proximidade esta executando!"));
  }
  else
  {
    Serial.println(F("APDS-9960 - Algo deu errado na inicialização do sensor de proximidade!"));
  }

  for (int i = 0; i < 17; i = i + 8)
  {
    CHIP_ID |= ((ESP.getEfuseMac() >> (40 - i)) & 0xff) << i;
  }

  Serial.print("Chip ID: ");
  Serial.println(CHIP_ID);

  conecta_WiFi();
  client_MQTT.setServer(MQTT_SERVER, MQTT_PORT);

  // Wait for initialization and calibration to finish
  delay(500);
}

void loop()
{
  reconecta();
  publica_status();
  client_MQTT.loop();
}

void conecta_WiFi()
{
  while (WiFi.status() == WL_CONNECTED)
  {
    return;
  }

  Serial.print("\nConectando-se a rede ");
  Serial.print(WIFI_SSID);

  WiFi.softAP(DEVICE_ID);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  delay(2000);
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(500);
  }

  Serial.println("");
  Serial.print("\nConectou-se a ");
  Serial.print(WIFI_SSID);
  Serial.print("\nEndereço de IP: ");
  Serial.println(WiFi.localIP());
}

void reconecta()
{
  conecta_MQTT();
  conecta_WiFi();
}

void conecta_MQTT()
{
  // Retorna se já estiver conectado
  if (client_MQTT.connected())
  {
    return;
  }

  Serial.print("Conectando ao Broker MQTT: ");
  Serial.println(MQTT_SERVER);

  uint8_t retries = 3;
  while (!client_MQTT.connect(DEVICE_ID, MQTT_USER, MQTT_PASS))
  {
    Serial.println("Tentando conexão com MQTT em 5 segundos...");
    // client_MQTT.disconnect();
    delay(5000);
    retries--;
    if (retries == 0)
    {
      // basically die and wait for WDT to reset me
      while (1)
        delay(10);
    }
  }
  Serial.println("Conectado ao Broker MQTT com sucesso!");
}

void publica_status()
{
  // APDS9960
  // Le os niveis de luz (ambient, red, green, blue)
  if (!APDS9960.readAmbientLight(LUZ_AMBIENTE) ||
      !APDS9960.readRedLight(LUZ_VERMELHA) ||
      !APDS9960.readGreenLight(LUZ_VERDE) ||
      !APDS9960.readBlueLight(LUZ_AZUL))
  {
    Serial.println("Erro ao ler os valores da luz");
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
    // publica status do sensor
    char luz[50];
    String temp_luz = String(LUZ_AMBIENTE);
    temp_luz.toCharArray(luz, temp_luz.length() + 1);
    client_MQTT.publish("ESP32-001/Luz ambiente", luz);
  }

  if (!APDS9960.readProximity(PROXIMIDADE))
  {
    Serial.println("Error reading proximity value");
  }
  else
  {
    Serial.print("Proximidade: ");
    Serial.println(PROXIMIDADE);
    char prox[50];
    String temp_prox = String(PROXIMIDADE);
    temp_prox.toCharArray(prox, temp_prox.length() + 1);
    client_MQTT.publish("ESP32-001/Proximidade", prox);
  }

  // Wait 10 second before next reading
  delay(10000);
}

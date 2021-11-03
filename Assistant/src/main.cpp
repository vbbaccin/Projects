#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <stdio.h>
#include <string.h>
#include "fauxmoESP.h"
#include "credentials.h"

fauxmoESP echo;
WiFiClient client_WiFi;
PubSubClient client_MQTT(client_WiFi);

uint32_t CHIP_ID = 0;

// Dispositivos Zigbee
#define ID_LUZ_SALA_ESTAR "Luz da sala de estar"
#define ID_LUZ_SALA_JANTAR "Luz da sala de jantar"
#define ID_LUZ_BANCADA "Luz da bancada"
#define ID_LUZ_PORTA_ENTRADA "Luz da porta de entrada"
#define ID_LUZ_COZINHA "Luz da cozinha"
#define ID_LUZ_AREA_SERVICO "Luz da area de servico"
#define ID_LUZ_CORREDOR "Luz do corredor"
#define ID_LUZ_ESCRITORIO "Luz do escritorio"
#define ID_LUZ_BANHEIRO_VISITAS "Luz do banheiro de visitas"
#define ID_LUZ_BANHEIRO_VISITAS_AUXILIAR "Luz auxiliar do banheiro de visitas"
#define ID_LUZ_BANHEIRO_VISITAS_LED "Luz do led do banheiro de visitas"
#define ID_LUZ_BANHEIRO_SUITE "Luz do banheiro da suite"
#define ID_LUZ_BANHEIRO_SUITE_AUXILIAR "Luz auxiliar do banheiro da suite"
#define ID_LUZ_BANHEIRO_SUITE_LED "Luz do led do banheiro da suite"
#define ID_LUZ_SUITE "Luz da suite"
#define ID_LUZ_ROUPEIRO "Luz do roupeiro"
#define ID_LUZ_LED_CAMA "Luz da cama"
#define ID_TODAS_LUZES "Todas as luzes"
#define ID_LUZES_APARTAMENTO "luzes do apartamento"
#define ID_TOMADA_SALA "Tomada da sala"
#define ID_TOMADA_ESCRITORIO "Tomada do escritorio"
#define ID_TOMADA_SUITE "Tomada da suite"
#define ID_LED_BUILDIN "Led da placa"

// // WiFi
// const char *WIFI_SSID = "VTR_IOT";
// const char *WIFI_PASS = "s3nh4.w1f1.IOT";

// // MQTT
// const char *MQTT_USER = "broker_mqtt_user";
// const char *MQTT_PASS = "1M0squ1t02_MQTT3";
// const char *MQTT_SERVER = "192.168.0.3";
// const int MQTT_PORT = 1883;

// Device
const int SERIAL_BAUDRATE = 115200;
const int LED_BUILD_IN = 2;
const char *DEVICE_ID = "ESP32-Assistant";

// Declaração das funções
void connect_WiFi();
void connect_MQTT();
void reconnect();
void publish_Status(const char* topic, const char* payload);
void print_Chip_ID();
void callback_MQTT(char* topic, byte* message, unsigned int length);

void setup()
{
  Serial.begin(SERIAL_BAUDRATE);
  Serial.println();
  pinMode(LED_BUILD_IN, OUTPUT);

  print_Chip_ID();
  connect_WiFi();
  connect_MQTT();

  // By default, fauxmoESP creates it's own webserver on the defined port
  // The TCP port must be 80 for gen3 devices (default is 1901)
  // This has to be done before the call to enable()
  echo.createServer(true); // not needed, this is the default value
  echo.setPort(80);        // This is required for gen3 devices

  // You have to call enable(true) once you have a WiFi connection
  // You can enable or disable the library at any moment
  // Disabling it will prevent the devices from being discovered and switched
  echo.enable(true);

  // You can use different ways to invoke alexa to modify the devices state:
  // "Alexa, turn yellow lamp on"
  // "Alexa, turn on yellow lamp
  // "Alexa, set yellow lamp to fifty" (50 means 50% of brightness, note, this example does not use this functionality)

  // Add virtual devices
  echo.addDevice(ID_LUZ_BANCADA);
  echo.addDevice(ID_LUZ_CORREDOR);
  echo.addDevice(ID_TODAS_LUZES);
  echo.addDevice(ID_LED_BUILDIN);

  echo.onSetState([](unsigned char device_id, const char *device_name, bool state, unsigned char value)
                  {
                    // Callback when a command from Alexa is received.
                    // You can use device_id or device_name to choose the element to perform an action onto (relay, LED,...)
                    // State is a boolean (ON/OFF) and value a number from 0 to 255 (if you say "set kitchen light to 50%" you will receive a 128 here).
                    // Just remember not to delay too much here, this is a callback, exit as soon as possible.
                    // If you have to do something more involved here set a flag and process it in your main loop.

                    Serial.printf("[MAIN] Device #%d (%s) state: %s value: %d\n", device_id, device_name, state ? "ON" : "OFF", value);

                    // Checking for device_id is simpler if you are certain about the order they are loaded and it does not change.
                    // Otherwise comparing the device_name is safer.

                    if (strcmp(device_name, ID_LUZ_BANCADA) == 0)
                    {
                      publish_Status(device_name, (state ? "on" : "off"));
                      Serial.printf("[ECHO] Alterou #%d (%s) estado: %s valor: %d\n", device_id, device_name, state ? "on" : "off", value);
                    }
                    else if (strcmp(device_name, ID_TODAS_LUZES) == 0)
                    {
                      publish_Status(device_name, (state ? "on" : "off"));
                      Serial.printf("[ECHO] Alterou #%d (%s) estado: %s valor: %d\n", device_id, device_name, state ? "on" : "off", value);
                    }
                    else if (strcmp(device_name, ID_LUZ_CORREDOR) == 0)
                    {
                      publish_Status(device_name, (state ? "on" : "off"));
                      Serial.printf("[ECHO] Alterou #%d (%s) estado: %s valor: %d\n", device_id, device_name, state ? "on" : "off", value);
                    }
                    else if (strcmp(device_name, ID_LED_BUILDIN) == 0)
                    {
                      digitalWrite(LED_BUILD_IN, state ? HIGH : LOW);
                      publish_Status(device_name, (state ? "on" : "off"));
                      Serial.printf("[ECHO] Alterou #%d (%s) estado: %s valor: %d\n", device_id, device_name, state ? "on" : "off", value);
                    }
                  });
}

void loop()
{
  reconnect();

  client_MQTT.loop();


  // put your main code here, to run repeatedly:
  // fauxmoESP uses an async TCP server but a sync UDP server
  // Therefore, we have to manually poll for UDP packets
  echo.handle();

  // This is a sample code to output free heap every 5 seconds
  // This is a cheap way to detect memory leaks
  static unsigned long last = millis();
  if (millis() - last > 5000)
  {
    last = millis();
    Serial.printf("[MAIN] Free heap: %d bytes\n", ESP.getFreeHeap());
  }

  // If your device state is changed by any other means (MQTT, physical button,...)
  // you can instruct the library to report the new state to Alexa on next request:
  // echo.setState(ID_YELLOW, true, 255);
  // echo.setState("kitchen light", true, 128);
}

void reconnect()
{
  connect_WiFi();
  connect_MQTT();
}

void connect_WiFi()
{
  while (WiFi.status() == WL_CONNECTED)
  {
    return;
  }

  // Set WIFI module to STA mode
  WiFi.mode(WIFI_STA);

  // Connect
  Serial.printf("[WIFI] Connecting to %s ", WIFI_SSID);
  WiFi.softAP(DEVICE_ID);
  WiFi.begin(WIFI_SSID, WIFI_PASS);

  // Wait
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    digitalWrite(LED_BUILD_IN, HIGH);
    Serial.print(".");
    delay(500);
    digitalWrite(LED_BUILD_IN, LOW);
  }
  Serial.println();

  // Connected!
  Serial.printf("[WIFI] STATION Mode, SSID: %s, IP address: %s\n", WiFi.SSID().c_str(), WiFi.localIP().toString().c_str());
  digitalWrite(LED_BUILD_IN, LOW);
}

void connect_MQTT()
{
  // Retorna se já estiver conectado
  if (client_MQTT.connected())
  {
    client_MQTT.subscribe("ESP32-Assistant/Led da placa", 1);
    return;
  }

  Serial.print("[MQTT] Conectando ao Broker: ");
  Serial.println(MQTT_SERVER);

  uint8_t retries = 3;
  client_MQTT.setServer(MQTT_SERVER, MQTT_PORT);
  client_MQTT.setCallback(callback_MQTT);
  while (!client_MQTT.connect(DEVICE_ID, MQTT_USER, MQTT_PASS))
  {
    Serial.println("[MQTT] Tentando conexão em 5 segundos...");
    delay(5000);
    retries--;
    if (retries == 0)
    {
      // basically die and wait for WDT to reset me
      while (1)
        delay(10);
    }
  }
  Serial.println("[MQTT] Conectado ao Broker com sucesso!");
}

void publish_Status(const char* topic, const char* payload)
{
  char s_topic[50] = "";
  strcat(s_topic, DEVICE_ID);
  strcat(s_topic, "/");
  strcat(s_topic, topic);
  client_MQTT.publish(s_topic, payload);
  Serial.printf("[MQTT] Publica no topico %s o status %s", s_topic, payload);
  Serial.println();
}

void print_Chip_ID()
{
  for (int i = 0; i < 17; i = i + 8)
  {
    CHIP_ID |= ((ESP.getEfuseMac() >> (40 - i)) & 0xff) << i;
  }
  Serial.print("[DEVICE] Chip ID: ");
  Serial.println(CHIP_ID);
}

void callback_MQTT(char* topic, byte* message, unsigned int length) 
{
  Serial.print("[MQTT] Mensagem recebida no topico: ");
  Serial.print(topic);
  Serial.print(". Mensagem: ");
  String messageTemp;
  
  for (int i = 0; i < length; i++) {
    Serial.print((char)message[i]);
    messageTemp += (char)message[i];
  }
  Serial.println();

  // Feel free to add more if statements to control more GPIOs with MQTT

  // If a message is received on the topic esp32/output, you check if the message is either "on" or "off". 
  // Changes the output state according to the message
  char s_topic[50] = "";
  strcat(s_topic, DEVICE_ID);
  strcat(s_topic, "/");

  //if (strcmp(topic, strcat(s_topic, ID_LED_BUILDIN)) == 0)
  if (String(topic) == String(strcat(s_topic, ID_LED_BUILDIN))) {
    Serial.printf("[MQTT] Changing output to : %s\n", messageTemp == "on" ? "ON" : "OFF");
    if(messageTemp == "on"){
      echo.setState(ID_LED_BUILDIN, true, 255);
    }
    else if(messageTemp == "off"){
      echo.setState(ID_LED_BUILDIN, false, 255);
    }
    digitalWrite(LED_BUILD_IN, messageTemp == "on" ? HIGH : LOW);
  }
}
#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <credentials.h>

#define pinSensorPressaoLD1 13 // Sensor de pressão do lado direito cabeça
#define pinSensorPressaoLD2 12 // Sensor de pressão do lado direito pes
#define pinSensorPressaoLE3 16 // Sensor de pressão do lado esquerdo cabeça
#define pinSensorPressaoLE4 17 // Sensor de pressão do lado esquerdo pes

WiFiClient clientWifi;
PubSubClient clientBrokerMQTT(clientWifi);

int statusLD1 = 0;
int statusLD2 = 0;
int statusLE3 = 0;
int statusLE4 = 0;

// Declaração das Funções
void reconectaConexoes();    // Garante que as conexoes com WiFi e MQTT Broker se mantenham ativas
void conectaWiFi();          // Faz conexão com WiFi
void conectaMQTT();          // Faz conexão com Broker MQTT
void enviaAlteracaoStatus(); // Envia pacotes ao broker

void setup()
{
  pinMode(pinSensorPressaoLD1, INPUT);
  pinMode(pinSensorPressaoLD2, INPUT);
  pinMode(pinSensorPressaoLE3, INPUT);
  pinMode(pinSensorPressaoLE4, INPUT);

  statusLD1 = digitalRead(pinSensorPressaoLD1);
  statusLD2 = digitalRead(pinSensorPressaoLD2);
  statusLE3 = digitalRead(pinSensorPressaoLE3);
  statusLE4 = digitalRead(pinSensorPressaoLE4);

  Serial.begin(115200);
  conectaWiFi();
  clientBrokerMQTT.setServer(MQTT_SERVER, MQTT_PORT);
}

void loop()
{
  reconectaConexoes();
  enviaAlteracaoStatus();
  clientBrokerMQTT.loop();
}

void conectaWiFi()
{

  while (WiFi.status() == WL_CONNECTED)
  {
    return;
  }

  Serial.print("\nConectando-se a rede ");
  Serial.print(WIFI_SSID);

  WiFi.begin(WIFI_SSID, WIFI_PASS);
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(100);
  }

  Serial.println("");
  Serial.print("\nConectou-se a ");
  Serial.print(WIFI_SSID);
  Serial.print("\nEndereço de IP: ");
  Serial.println(WiFi.localIP());
}

void reconectaConexoes()
{
  if (!clientBrokerMQTT.connected())
  {
    conectaMQTT();
  }
  conectaWiFi(); // se não há conexão com o WiFI, a conexão é refeita
}

void conectaMQTT()
{
  while (!clientBrokerMQTT.connected())
  {
    Serial.print("Conectando ao Broker MQTT: ");
    Serial.println(MQTT_SERVER);
    if (clientBrokerMQTT.connect(DEVICE_ID, MQTT_USER, MQTT_PASS))
    {
      Serial.println("Conectado ao Broker com sucesso!");
    }
    else
    {
      Serial.println("Noo foi possivel se conectar ao broker.");
      Serial.println("Nova tentatica de conexao em 10s");
      delay(10000);
    }
  }
}

void enviaAlteracaoStatus()
{
  static unsigned long debounce1;
  static unsigned long debounce2;
  static unsigned long debounce3;
  static unsigned long debounce4;

  int statusLeitura = digitalRead(pinSensorPressaoLD1);

  if ((millis() - debounce1) > 10000)
  { // Elimina efeito Bouncing
    // Sensor de pressão do lado direito cabeça
    if (statusLeitura != statusLD1 && statusLeitura == HIGH)
    {
      clientBrokerMQTT.publish("Cama Lado Direito Cima", "on");
      Serial.println("LD1 on");
      statusLD1 = statusLeitura;
      debounce1 = millis();
    }
    else if (statusLeitura != statusLD1 && statusLeitura == LOW)
    {
      clientBrokerMQTT.publish("Cama Lado Direito Cima", "off");
      Serial.println("LD1 off");
      statusLD1 = statusLeitura;
      debounce1 = millis();
    }
  }

  statusLeitura = digitalRead(pinSensorPressaoLD2);
  if ((millis() - debounce2) > 30)
  { // Elimina efeito Bouncing
    // Sensor de pressão do lado direito pes
    if (statusLeitura != statusLD2 && statusLD2 == HIGH)
    {
      clientBrokerMQTT.publish("Cama Lado Direito Baixo", "on");
      Serial.println("LD2 on");
      statusLD2 = statusLeitura;
      debounce2 = millis();
    }
    else if (statusLeitura != statusLD2 && statusLeitura == LOW)
    {
      clientBrokerMQTT.publish("Cama Lado Direito Baixo", "off");
      Serial.println("LD2 off");
      statusLD2 = statusLeitura;
      debounce2 = millis();
    }
  }

  // Sensor de pressão do lado esquerdo cabeça
  statusLeitura = digitalRead(pinSensorPressaoLE3);
  if ((millis() - debounce3) > 30)
  { // Elimina efeito Bouncing
    if (statusLeitura != statusLE3 && statusLeitura == HIGH)
    {
      clientBrokerMQTT.publish("Cama Lado Esquerdo Cima", "on");
      Serial.println("LE3 on");
      statusLE3 = statusLeitura;
      debounce3 = millis();
    }
    else if (statusLeitura != statusLE3 && statusLeitura == LOW)
    {
      clientBrokerMQTT.publish("Cama Lado Esquerdo Cima", "off");
      Serial.println("LE3 off");
      statusLE3 = statusLeitura;
      debounce3 = millis();
    }
  }

  // Sensor de pressão do lado esquerdo pes
  statusLeitura = digitalRead(pinSensorPressaoLE4);
  if ((millis() - debounce4) > 30)
  { // Elimina efeito Bouncing
    if (statusLeitura != statusLE4 && statusLeitura == HIGH)
    {
      clientBrokerMQTT.publish("Cama Lado Esquerdo Baixo", "on");
      Serial.println("LE4 on");
      statusLE4 = statusLeitura;
      debounce4 = millis();
    }
    else if (statusLeitura != statusLE4 && statusLeitura == LOW)
    {
      clientBrokerMQTT.publish("Cama Lado Esquerdo Baixo", "off");
      Serial.println("LE4 off");
      statusLE4 = statusLeitura;
      debounce4 = millis();
    }
  }
}
// Dispositivo ALERTA
// @akirasan diciembre 2020

#include <Arduino.h>
#include <MD_Parola.h>

#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <PubSubClient.h>

ESP8266WiFiMulti wifiMulti;
WiFiClient espClient;

// Configuración WiFi y servidor MQTT
#include "configuration.h" // Configura tus datos de conexión

PubSubClient client(espClient);

#include <MD_Parola.h>
#include <MD_MAX72xx.h>
#include <SPI.h>

#include <Ticker.h>

// Define the number of devices we have in the chain and the hardware interface
// NOTE: These pin numbers will probably not work with your hardware and may
// need to be adapted
#define HARDWARE_TYPE MD_MAX72XX::FC16_HW
#define MAX_DEVICES 1

#define CLK_PIN 3
#define DATA_PIN 2
#define CS_PIN 0

// Hardware SPI connection
//MD_Parola P = MD_Parola(HARDWARE_TYPE, CS_PIN, MAX_DEVICES);
// Arbitrary output pins
MD_Parola P = MD_Parola(HARDWARE_TYPE, DATA_PIN, CLK_PIN, CS_PIN, MAX_DEVICES);

void alive_msg();
Ticker alive_timer(alive_msg, FRECUENCIA); // Publicación mensaje dispositivo disponible

void setup_wifi()
{
// We start by connecting to a WiFi network
#ifdef DEBUG
  Serial.println();
#endif

  WiFi.mode(WIFI_STA);
  WiFi.hostname(hostname);

#ifdef DEBUG
  Serial.println("Connecting Wifi...");
#endif

  byte i_wifi = 0;
  while (ssid[i_wifi] != "")
  {
    wifiMulti.addAP(ssid[i_wifi], password[i_wifi]);
    i_wifi++;
  }

  while (wifiMulti.run() != WL_CONNECTED)
  {
    delay(500);
#ifdef DEBUG
    Serial.print(".");
#endif
  }

  randomSeed(micros());
#ifdef DEBUG
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
#endif
}

void parpadeo(char *car, unsigned int vel)
{
  P.write(car);
  P.setInvert(false);
  P.displayAnimate();
  delay(vel);
  P.write(car);
  P.setInvert(true);
  P.displayAnimate();
  delay(vel);
}

// Lectura de mensajes MQTT suscritos
void mensaje_recibido(char *topic, byte *payload, unsigned int length)
{

#ifdef DEBUG
  Serial.print("Mensaje recibido [");
  Serial.print(topic);
  Serial.print("] ");
#endif
  String topicStr = topic;
  String payloadStr = "";

  if (topicStr == topic_nivel)
  {
    for (unsigned int i = 0; i < length; i++)
    {
      payloadStr += (char)payload[i];
    }
#ifdef DEBUG
    Serial.println("----");
#endif

    P.displayClear();
    P.write(" ");
    if (payloadStr == "ON")
    {
      P.setInvert(false);
    }
    else if (payloadStr == "OFF")
    {
      P.setInvert(true);
    }

    if (payloadStr == "1")
    {
      for (byte i = 0; i < 15; i++)
      {
        parpadeo(" ", 250);
      }
    }

    if (payloadStr == "2")
    {
      for (byte i = 0; i < 45; i++)
      {
        parpadeo(" ", 50);
      }
    }
  }

  if (topicStr == topic_mensaje)
  {
    for (unsigned int i = 0; i < length; i++)
    {
      payloadStr += (char)payload[i];
    }
#ifdef DEBUG
    Serial.println(payloadStr);
    Serial.println("----");
#endif
  }
}

// Conexión al servidor MQTT
void conectarMQTT()
{
  // Loop hasta conectarnos a MQTT server y publicar/suscribir a mensajes
  while (!client.connected())
  {

#ifdef DEBUG
    Serial.print("MQTT connection...");
    // Creamos un nombre de cliente aleatorio a partir de este prefijo
#endif
    clientId += String(random(0xffff), HEX);
    // Intentamos conectarnos al servidor MQTT
    if (client.connect(clientId.c_str(), mqtt_User, mqtt_Password))
    {
#ifdef DEBUG
      Serial.println("conectado!!!");
#endif
      // Una vez conectado publicamos un mensaje con la IP de la LAN asignada
      String c_IP = WiFi.localIP().toString();
      char C_IP[20];
      c_IP.toCharArray(C_IP, c_IP.length() + 1);

      client.publish(topic_ip, C_IP);
      // ...y nos suscribimos a los mensajes que queremos leer
      client.subscribe(topic_nivel);
      client.subscribe(topic_mensaje);
    }
    else
    {

#ifdef DEBUG
      Serial.print("error, rc=");
      Serial.print(client.state());
      Serial.println(" ...5 segundos (reintento)");
#endif
      delay(5000);
    }
  }
}

void alive_msg()
{
  char C_CONT[20];
  sprintf(C_CONT, "%lu", alive_timer.counter());
  client.publish(topic_alive, C_CONT);
}

void setup()
{

#ifdef DEBUG
  Serial.begin(115200);
#endif

  // Configuación conexión WiFi
  setup_wifi();

  // Configuración conexión server MQTT y mensajes
  client.setServer(mqtt_server, mqtt_server_port);
  client.setCallback(mensaje_recibido);

  P.begin();
  P.displayReset();
  P.setSpeed(50);
  P.setInvert(true);
  P.setIntensity(15);

  alive_timer.start();
}

void loop()
{
  // Verificamos si estamos conectados al servidor MQTT
  if (!client.connected())
  {
    conectarMQTT(); // Reintentos de conexión
  }
  client.loop(); // Por si hemos recibido algún mensaje de los suscritos

  alive_timer.update();
  P.displayAnimate();
}
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <AsyncElegantOTA.h>
#include <PubSubClient.h>
#include <IRremote.h>
#include <sendRawHEX.cpp>
#include <env.cpp>

// #define DEBUG

#define HEX_POWER 0x1FEA05F
#define HEX_NIGHT_MODE 0x1FE609F
#define HEX_BRIGHTNESS_UP 0x1FE7887
#define HEX_BRIGHTNESS_DOWN 0x1FE40BF
#define HEX_SWITCH_TEMP 0x1FE807F
#define HEX_TIMER 0x1FEC03F
#define HEX_COLD 0x1FE48B7
#define HEX_WARM 0x1FE58A7

// IR Receiver config
decode_results results;
IRrecv irrecv(2);

int OutputPin = 0;

// WiFi config
const IPAddress ip(192, 168, 100, 55);
const IPAddress gateway(192, 168, 100, 1);
const IPAddress bcastAddr(192, 168, 100, 255);
const IPAddress subnet(255, 255, 255, 0);
const IPAddress dns(192, 168, 100, 1);
AsyncWebServer server(80);

// MQTT config
WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);
String clientId = "NodeMCU-MainLight";

void connectWiFi()
{
  WiFi.mode(WIFI_STA);
  WiFi.hostname("NodeMCU-MainLight");
  WiFi.config(ip, dns, gateway, subnet);
  WiFi.begin(WIFI_SSID, WIFI_PASS);

  int count = 0;
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(250);
    digitalWrite(LED_BUILTIN, HIGH);
    delay(250);
    digitalWrite(LED_BUILTIN, LOW);
#ifdef DEBUG
    Serial.print(".");
#endif

    count++;
    if (count > 20)
    {
      delay(500);
      ESP.restart();
    }
  }

#ifdef DEBUG
  Serial.println("\nWifi Connected, Ip: " + WiFi.localIP().toString());
#endif
}

void enableOTA()
{
  AsyncElegantOTA.begin(&server);
  server.begin();
#ifdef DEBUG
  Serial.println("HTTP server started");
#endif
}

void mqttMessageHandler(char *topic, byte *payload, unsigned int length)
{
  char message[length + 1];
  memcpy(message, payload, length);
  message[length] = '\0';

  if (strcmp(topic, "main-light/control") == 0)
  {
    if (strcmp(message, "power") == 0)
    {
      sendRawHEX(OutputPin, HEX_POWER, 32);
    }
    else if (strcmp(message, "night-mode") == 0)
    {
      sendRawHEX(OutputPin, HEX_NIGHT_MODE, 32);
    }
    else if (strcmp(message, "brightness-up") == 0)
    {
      sendRawHEX(OutputPin, HEX_BRIGHTNESS_UP, 32);
    }
    else if (strcmp(message, "brightness-down") == 0)
    {
      sendRawHEX(OutputPin, HEX_BRIGHTNESS_DOWN, 32);
    }
    else if (strcmp(message, "switch-temp") == 0)
    {
      sendRawHEX(OutputPin, HEX_SWITCH_TEMP, 32);
    }
    else if (strcmp(message, "timer") == 0)
    {
      sendRawHEX(OutputPin, HEX_TIMER, 32);
    }
    else if (strcmp(message, "cold") == 0)
    {
      sendRawHEX(OutputPin, HEX_COLD, 32);
    }
    else if (strcmp(message, "warm") == 0)
    {
      sendRawHEX(OutputPin, HEX_WARM, 32);
    }
  }

#ifdef DEBUG
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  Serial.println(message);
#endif
}

void connectMQTT()
{
  mqttClient.setServer(MQTT_HOST, MQTT_PORT);
  mqttClient.setCallback(mqttMessageHandler);
}

void reconnectMQTT()
{
  // Loop until we're reconnected
  while (!mqttClient.connected())
  {
#ifdef DEBUG
    Serial.print("Attempting MQTT connection...");
#endif
    // Attempt to connect
    if (mqttClient.connect(clientId.c_str(), MQTT_USERNAME, MQTT_PASSWORD))
    {
#ifdef DEBUG
      Serial.println("connected");
#endif
      //* MQTT startup
      mqttClient.publish("main-light/nodeMCU-on", "true");
      mqttClient.subscribe("main-light/control");
    }
    else
    {
#ifdef DEBUG
      Serial.print("failed, rc=");
      Serial.print(mqttClient.state());
      Serial.println(" try again in 5 seconds");
#endif
      delay(5000);
    }
  }
}

void setup()
{
#ifdef DEBUG
  Serial.begin(115200);
#endif
  pinMode(OutputPin, OUTPUT);
  digitalWrite(OutputPin, HIGH);
  irrecv.enableIRIn();

  connectWiFi();
  enableOTA();
  connectMQTT();
}

void loop()
{
  // listen for IR data
  if (IrReceiver.decode(&results))
  {
    Serial.println(results.value, HEX);
    sendRawHEX(OutputPin, results.value, 32);
    IrReceiver.resume();
  }

  // Reconnect WiFi
  if (WiFi.status() != WL_CONNECTED)
  {
    connectWiFi();
    return;
  }

  // Reconnect MQTT
  if (!mqttClient.connected())
  {
    reconnectMQTT();
  }

  mqttClient.loop();
}
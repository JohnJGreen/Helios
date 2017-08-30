#include <PubSubClient.h>
#include <ESP8266SetupHelper.h>

#include <DNSServer.h>
#include <ArduinoJson.h>

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include "FS.h"


// Access point credentials
// WiFi parameters
//String wifiParameters = "{\"ssid\":\"belkin.3a66\",\"password\":\"9724449507\"}";

ESP8266WebServer server(80);
ESP8266SetupHelper espSH;

WiFiClient wifiClient;
PubSubClient mqtt_client(wifiClient);

const int LED_PIN = 0;
const int IN_PIN = 3;
int ledStatus = LOW;

char heliosIP[16];
char clientID[128];
char token[128];
char authMethod[] = "use-token-auth";
char topic[] = "/helios/test";

#define DEVICE_TYPE "ESP8266"

void handleRoot() {
  // Just serve the index page from SPIFFS when asked for
  File page = SPIFFS.open("/index.html", "r");
  server.streamFile(page, "text/html");
  Serial.println("Handling Request For Root");
  page.close();
}

void getSetupJSON() {
  // Just serve the index page from SPIFFS when asked for
  File settings = SPIFFS.open("/setup.json", "r");
  String settingsString = settings.readString();
  Serial.println("Got Request For Settings");
  Serial.println(settingsString);
  server.send(200, "text/plain", settingsString);
  //server.streamFile(settings, "text/html");
  settings.close();
}

void handleSetup() {
  
  for ( uint8_t i = 0; i < server.args(); i++ ) {
    String message = " " + server.argName ( i ) + ": " + server.arg ( i );
    Serial.println(message);
  }

  // Just serve the index page from SPIFFS when asked for
  if (server.hasArg("wifiSSID") && server.hasArg("wifiPWD")) {
    Serial.println("New SSID is:" + server.arg("wifiSSID"));
    //espSH.setWiFiParams(server.arg("wifiSSID"), server.arg("wifiPWD"));
    espSH.setParam("wifiSSID", server.arg("wifiSSID"));
    espSH.setParam("wifiPWD", server.arg("wifiPWD"));
  }
  else
    Serial.println("Settings page requested without an attempt to set SSID and Password");
  if (server.hasArg("heliosServerIP")) {
    Serial.println("New heliosServerIP is:" + server.arg("heliosServerIP"));
    espSH.setParam("heliosServerIP", server.arg("heliosServerIP"));
  }
  else
    Serial.println("Settings page requested without an attempt to set heliosServerIP");
  File page = SPIFFS.open("/setup.html", "r");
  server.streamFile(page, "text/html");
  Serial.println("Handling Request For Settup Page");
  page.close();
}

// A function which sends the LED status back to the client
void sendStatus() {
  if (ledStatus == HIGH) server.send(200, "text/plain", "{\"state\":\"HIGH\"}");
  else server.send(200, "text/plain", "{\"state\":\"LOW\"}");
}

// Toggle the LED and back its status
void toggleLED() {
  ledStatus = ledStatus == HIGH ? LOW : HIGH;
  digitalWrite(LED_PIN, !ledStatus);

  String payload = "{\"status\":\"";
  payload += ledStatus;
  payload += "\"}";

  Serial.print("Sending payload: ");
  Serial.println(payload);

  if (mqtt_client.publish("/helios/bedroom3/lights", (char*) payload.c_str())) {
    Serial.println("Publish ok");
  } else {
    Serial.println("Publish failed");
  }
    
  sendStatus();
}

void resetSetup() {
  Serial.println("Deleting wifi credentials");
  espSH.clear();
  Serial.println("Restarting");
  server.send(200, "text/plain", "CLEARING SETUP FILE");
  ESP.restart();
}

void restartESPModule() {
  Serial.println("Restart Request Recieved");
  server.send(200, "text/plain", "RESTARTING ESP MODULE!");
  ESP.restart();
}

void setup() {

  pinMode(LED_PIN, OUTPUT);
  pinMode(IN_PIN, INPUT);
  pinMode(3, INPUT);
  digitalWrite(LED_PIN, ledStatus);

  // Start Serial
  Serial.begin(115200);
  Serial.println("Initialized");

  SPIFFS.begin();

  espSH.setup();

  if (espSH.connect())
  {
    Serial.println("WiFi settings worked. Server Connected");

    server.on("/", handleRoot);

    server.on("/toggle", toggleLED);
    server.on("/status", sendStatus);
    
  }
  else
  {
    Serial.println("Could not connect to AP. Setting up standalone server.");
    espSH.standalone();
    server.on("/", handleSetup);
  }


  // wifiConnect(loadWifiCredentials("/wifi.json"));

  // Handlers for various user-triggered events

  server.on("/setup.json", getSetupJSON);
  server.on("/setup", handleSetup);

  server.on("/clear", resetSetup);
  server.on("/restart", restartESPModule);

  server.begin();
  Serial.println("Server started");

  // Print the IP address
  Serial.println(WiFi.localIP());

  //IPAddress heliosServerIP();
  //heliosServerIP.fromString(espSH.getJSON("heliosServerIP"));

  espSH.getJSON("heliosServerIP").toCharArray(heliosIP, 16);

  Serial.print("Server IP Set to :");
  Serial.println(heliosIP);

  mqtt_client.setServer(heliosIP, 1883);
  mqtt_client.setCallback(callback);

  String clientIDString = espSH.getJSON("clientID");
  clientIDString += ":ESPID_";
  clientIDString += ESP.getChipId();

  clientIDString.toCharArray(clientID, clientIDString.length() + 1);

  String tokenID = espSH.getJSON("token");
  tokenID.toCharArray(token, tokenID.length() + 1);
}

int counter = 0;
int reading;

int current_state = LOW;

long time_val = 0;
int debounce_count = 500;

void loop() {

  if (!espSH.maintain()) {
    espSH.standalone();
    server.on("/", handleSetup);
  }
  
  if (!mqtt_client.connected()) {
    espSH.getJSON("heliosServerIP").toCharArray(heliosIP, 16);
    Serial.print("Reconnecting client to ");
    Serial.println(heliosIP);
    //Serial
    const int timeoutDelay = 1000;
    long timeout = millis() + timeoutDelay;
    while (!mqtt_client.connect(clientID, authMethod, token)) {
      Serial.print(".");
      delay(500);
      if (millis() > timeout){
        Serial.println("Wifi connection attempt timed out.");
        break;
      }
      yield();
    }
    Serial.println("Presuming connection to server");
    mqtt_client.subscribe("/helios/toggle");
    Serial.println("subscribing to topic");
  }
  
  mqtt_client.loop();

  // Check for button press

    // If we have gone on to the next millisecond
  if(millis() != time_val)
  {
    reading = digitalRead(IN_PIN);

    if(reading == current_state && counter > 0)
    {
      counter--;
    }
    if(reading != current_state)
    {
       counter++; 
    }
    // If the Input has shown the same value for long enough let's switch it
    if(counter >= debounce_count)
    {
      counter = 0;
      current_state = reading;
      Serial.println("Got sensor input, toggling");
      toggleLED();
    }
    time_val = millis();
  }
  

/*
  String payload = "{\"payload_test\":{\"name\":\"ESP8266.";
  payload += ESP.getChipId();
  payload += "\",\"counter\":\"";
  payload += counter;
  payload += "}}";

  ++counter;

  Serial.print("Sending payload: ");
  Serial.println(payload);

  if (mqtt_client.publish(topic, (char*) payload.c_str())) {
    Serial.println("Publish ok");
  } else {
    Serial.println("Publish failed");
  }*/

  server.handleClient();
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i=0;i<length;i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  if (strcmp(topic, "/helios/toggle")  == 0){
    Serial.println("Recieved toggle command via MQTT");
    toggleLED();
  }
  
}


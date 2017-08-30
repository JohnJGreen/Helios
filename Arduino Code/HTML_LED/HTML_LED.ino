#include <DNSServer.h>
#include <ArduinoJson.h>

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include "FS.h"
#include "ESP8266SetupHelper.h"

#define LED_BUILTIN 0

// Access point credentials
// WiFi parameters
//String wifiParameters = "{\"ssid\":\"belkin.3a66\",\"password\":\"9724449507\"}";

ESP8266WebServer server(80);
ESP8266SetupHelper espSH;

const int LED_PIN = LED_BUILTIN;
int ledStatus = LOW;

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

void handleSetup(){
    // Just serve the index page from SPIFFS when asked for
    if(server.hasArg("wifiSSID") && server.hasArg("wifiPWD")){
      Serial.println("New SSID is:" + server.arg("wifiSSID"));
      espSH.setWiFiParams(server.arg("wifiSSID"), server.arg("wifiPWD"));
    }
    else
      Serial.println("Settings page requested without an attempt to set SSID and Password");
    File page = SPIFFS.open("/setup.html", "r");
    server.streamFile(page, "text/html");
    Serial.println("Handling Request For Settup Page");
    page.close();
}

// A function which sends the LED status back to the client
void sendStatus() {
    if (ledStatus == HIGH) server.send(200, "text/plain", "HIGH");
    else server.send(200, "text/plain", "LOW");
}

// Toggle the LED and back its status
void toggleLED() {
    ledStatus = ledStatus == HIGH ? LOW : HIGH;
    digitalWrite(LED_PIN, ledStatus);
    sendStatus();
}

void resetSetup(){
  Serial.println("Deleting wifi credentials");
  espSH.clear();
  Serial.println("Restarting");
  server.send(200, "text/plain", "CLEARING SETUP FILE");
  ESP.restart();
}

void restartESPModule(){
  Serial.println("Restart Request Recieved");
  server.send(200, "text/plain", "RESTARTING ESP MODULE!");
  ESP.restart();
}

void setup() {
  
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, ledStatus);
    
    // Start Serial
    Serial.begin(115200);
    Serial.println();

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
}

void loop() {
    server.handleClient();
}

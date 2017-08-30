/*
   ESP8266WifiParamsLoader.cpp - Library which loads a JSON file from SPIFFS and interprets it into an SSID and Passphrase.

   Created by Arun Kalahasti, February 20, 2016
*/

#include "ESP8266SetupHelper.h"

ESP8266SetupHelper::ESP8266SetupHelper() {
  _hasSetupData = false;
  _hasWifiParams = false;
  _setupFName = "/setup.json";
  SPIFFS.begin();

}

bool ESP8266SetupHelper::setup() {
  bool setupSuccess = getSetupFile();
  return setupSuccess;
}

bool ESP8266SetupHelper::connect() {
  
  const int timeoutDelay = 30000;

  // test getJSON
  //Serial.println("Testing getJSON");
  //Serial.println(getJSON(_setupJSON,String("test")));

  const char *wifiSSID, *wifiPWD;

  StaticJsonBuffer<256> jsonBuffer;
  JsonObject& setupJSON = jsonBuffer.parseObject(_setupJSON);
  Serial.println("After JSON Construction: " + _setupJSON);

  if(setupJSON.success() && setupJSON.containsKey("wifiSSID") && setupJSON.containsKey("wifiPWD")){
    Serial.println("WiFi Params found, will attempt to use.");
    wifiSSID = setupJSON["wifiSSID"];
    wifiPWD = setupJSON["wifiPWD"];

    if(strlen(wifiSSID) == 0){
      Serial.println("No SSID set");
      return false;
    }
    else
      Serial.println("Length of SSID string:" + strlen(wifiSSID));

    WiFi.begin(wifiSSID, wifiPWD);
    long timeout = millis() + timeoutDelay;
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");

      // Handle timeout
      if (millis() > timeout){
        Serial.println("Wifi connection attempt timed out. Clearing WiFi params from JSON file so new data can be loaded and rebooting.");
        //removeWifiJSON();
        return false;
      }
    }
    Serial.println("");
    Serial.println("WiFi connected");
    return true;
  }
  else {
    Serial.println("Could not find wifi info or could not construct JSON");
    return false;
  }
 
}

bool ESP8266SetupHelper::clear() {
  if (SPIFFS.exists(_setupFName)) {
    SPIFFS.remove(_setupFName);
  }
  File setupFP = SPIFFS.open(_setupFName, "w");
  setupFP.print("{\"wifiSSID\":\"\",\"wifiPWD\":\"\", \"heliosServerIP\":\"192.168.0.0\"}");
  setupFP.close();
  return true;
}

/*
 * Sets up ESP as a access point to serve only a setup page
 *  Function needs to:
 *    1 - Create new AP
 *    2 - Load setup page from SPIFFS
 *    3 - Setup handler for settings input
 *    4 - Wait for user to connect and upload settings
 *    5 - Save settings as appropriate
 *    6 - Restart
 */
bool ESP8266SetupHelper::standalone(){
    IPAddress apIP(192, 168, 1, 1);
    char espSSIDBuf[15];
    String espSSID = "ESP_";
    espSSID += ESP.getChipId();
    espSSID.toCharArray(espSSIDBuf, 15);
    WiFi.mode(WIFI_AP_STA);
    WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
    WiFi.softAP(espSSIDBuf);
}

/*
 * Sets the SSID saved in memory
 */
bool ESP8266SetupHelper::setWiFiParams(String newSSID, String newPWD){
  Serial.println("Settings new SSID");
  String setupJSONstring;
  StaticJsonBuffer<256> jsonBuffer;
  if (SPIFFS.exists(_setupFName)) {
    File setupFP = SPIFFS.open(_setupFName, "r");
    if (!setupFP)
      return false;
    setupJSONstring = setupFP.readString();
    setupFP.close();
    JsonObject& setupJSON = jsonBuffer.parseObject(setupJSONstring);
    setupJSON["wifiSSID"] = newSSID;
    setupJSON["wifiPWD"] = newPWD;
    char buffer[256];
    setupJSON.printTo(buffer, sizeof(buffer));
    Serial.println("Testing output before storing in FS");
    Serial.println(buffer);
    Serial.println("test output complete");
    setupFP = SPIFFS.open(_setupFName, "w");
    setupFP.print(buffer);
    setupFP.close();
    return true;
  }
  return false;
}

// Load setup file into string for access
bool ESP8266SetupHelper::getSetupFile() {
  String setupJSONstring;
  StaticJsonBuffer<256> jsonBuffer;
  if (SPIFFS.exists(_setupFName)) {
    File setupFP = SPIFFS.open(_setupFName, "r");
    if (!setupFP)
      return false;
    _setupJSON = setupFP.readString();
    return true;
  }
  return false;
}

String ESP8266SetupHelper::getJSON(String jsonSource, String jsonKey) {
  StaticJsonBuffer<256> jsonBuffer;
  JsonObject& setupJSON = jsonBuffer.parseObject(jsonSource);
  Serial.println("SUCCESS: Parsed File");
  if (setupJSON.success()) {
    Serial.println("ERROR: Could not parse string:/n/t" + jsonSource);
  }
  return setupJSON[jsonKey].asString();
  /*
    if(setupJSON.containsKey(jsonKey)){
    Serial.println("SUCCESS: JSON source contains the key: /n/t");
    //Serial.println(jsonKey);
    }*/
}

bool ESP8266SetupHelper::wifiConnect() {

}


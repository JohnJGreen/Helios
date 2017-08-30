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


// returns true only if a good connection is established
bool ESP8266SetupHelper::connect() {
  
  const int timeoutDelay = 30000;

  // test getJSON
  Serial.println("Testing getJSON");
  Serial.println(getJSON(_setupJSON,"heliosServerIP"));
  Serial.println(getJSON(_setupJSON,"test"));

  const char *wifiSSID, *wifiPWD;

  StaticJsonBuffer<1024> jsonBuffer;
  JsonObject& setupJSON = jsonBuffer.parseObject(_setupJSON);
  Serial.println("After JSON Construction: " + _setupJSON);
  
  if(!setupJSON.success()){
	  Serial.println("Could not find wifi info");
	  return false;
  }

  if(setupJSON.containsKey("wifiSSID") && setupJSON.containsKey("wifiPWD")){
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
        Serial.println("Wifi connection attempt timed out.");
        //removeWifiJSON();
        return false;
      }
    }
    Serial.println("");
    Serial.println("WiFi connected");
    return true;
  }
  else {
    Serial.println("Could not find wifi info");
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
  StaticJsonBuffer<1024> jsonBuffer;
  if (SPIFFS.exists(_setupFName)) {
    File setupFP = SPIFFS.open(_setupFName, "r");
    if (!setupFP)
      return false;
    setupJSONstring = setupFP.readString();
    setupFP.close();
    JsonObject& setupJSON = jsonBuffer.parseObject(setupJSONstring);
    setupJSON["wifiSSID"] = newSSID;
    setupJSON["wifiPWD"] = newPWD;
    char buffer[1024];
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

/*
 * 
 */
bool ESP8266SetupHelper::maintain(){

  if(WiFi.status() != WL_CONNECTED){
    Serial.println("WiFi not connected. Will attempt a reconnect.");
    const int timeoutDelay = 30000;
    long timeout = millis() + timeoutDelay;
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
      
      // Handle timeout
      if (millis() > timeout){
        Serial.print("WiFi reconnect attempt timed out at " + timeoutDelay);
		    Serial.println(" milliseconds");
        return false;
      }
      yield();
    }
  }
  return true;
}

/*
 * Sets a param in setup json
 */
 bool ESP8266SetupHelper::setParam(String key, String value){
	 
	 char char_key[key.length() + 1];
	 char char_value[value.length()+1];
	 
	 key.toCharArray(char_key, key.length()+1);
	 value.toCharArray(char_value, value.length()+1);
	 
	 setParam(char_key, char_value);
 }

/*
 * Sets a param in setup json
 */
 bool ESP8266SetupHelper::setParam(char* key, char* value){
	 
	 Serial.print("Settings a param. key:");
	 Serial.print(key);
	 Serial.print(" value:");
	 Serial.println(value);
	 
	 if(getSetupFile()){
		 StaticJsonBuffer<1024> jsonBuffer;
		 JsonObject& setupJSON = jsonBuffer.parseObject(_setupJSON);
		 setupJSON.prettyPrintTo(Serial);
		 setupJSON[key] = value;
		 setupJSON.prettyPrintTo(Serial);
		 
		 char buffer[1024];
		 setupJSON.printTo(buffer, sizeof(buffer));
		 _setupJSON = String(buffer);
		 Serial.println(_setupJSON);
		 writeSetupFile();
	 }
 }

// Load setup file into string for access
bool ESP8266SetupHelper::getSetupFile() {
  String setupJSONstring;
  StaticJsonBuffer<1024> jsonBuffer;
  if (SPIFFS.exists(_setupFName)) {
    File setupFP = SPIFFS.open(_setupFName, "r");
    if (!setupFP)
      return false;
    _setupJSON = setupFP.readString();
    return true;
  }
  return false;
}

bool ESP8266SetupHelper::writeSetupFile(){
	File setupFP = SPIFFS.open(_setupFName, "w");
    setupFP.print(_setupJSON);
    setupFP.close();
}

String ESP8266SetupHelper::getJSON(char* jsonKey){
	return getJSON(_setupJSON, jsonKey);
}

String ESP8266SetupHelper::getJSON(String jsonSource, char* jsonKey) {
  StaticJsonBuffer<1024> jsonBuffer;
  JsonObject& setupJSON = jsonBuffer.parseObject(jsonSource);
  if (!setupJSON.success()) {
    Serial.println("ERROR: Could not parse string:/n/t" + jsonSource);
	return "Failed";
  }
  Serial.println("SUCCESS: Parsed File");
  Serial.println("Searching for key: " + String(jsonKey));
  if (setupJSON.containsKey(jsonKey)){
	  String jsonValue = setupJSON[jsonKey].asString();
	  Serial.println("Found value:" + jsonValue);
	  return jsonValue;  
  }
  else {
	  Serial.println("Key was not found.");
	  return "";
  }
}

bool ESP8266SetupHelper::wifiConnect() {
}


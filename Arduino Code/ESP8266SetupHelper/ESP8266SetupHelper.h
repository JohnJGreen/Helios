/*
 * ESP8266WifiParamsLoader.h - Library which loads a JSON file from SPIFFS and interprets it into an SSID and Passphrase.
 * 
 * Created by Arun Kalahasti, February 20, 2016
 */

 
#ifndef ESP8266SetupHelper_h
#define ESP8266SetupHelper_h

#include <ESP8266WiFi.h>
#include "Arduino.h"
#include "FS.h"
#include "ArduinoJson.h"

class ESP8266SetupHelper
{
  public:
    ESP8266SetupHelper();
    bool setup();
    bool connect();
    bool clear();
    bool standalone();
    bool setWiFiParams(String newSSID, String newPWD);
	bool setParam(String key, String value);
    bool maintain();
    String getJSON(char* jsonKey);

  private:
    bool _hasSetupData;
    bool _hasWifiParams;
    String _setupFName;
    String _setupJSON;
    bool getSetupFile();
	bool writeSetupFile();
    bool wifiConnect();
    String getWifiJSON();
    String getJSON(String jsonSource, char* jsonKey);
	bool setParam(char* key, char* value);
};

#endif

//////////Code setup for config wifi/////////////

#include <FS.h>                   //this needs to be first, or it all crashes and burns...
#include <WiFiManager.h>          //https://github.com/tzapu/WiFiManager

#ifdef ESP32
  #include <SPIFFS.h>
#endif

#include <ArduinoJson.h>          //https://github.com/bblanchon/ArduinoJson

//define your default values here, if there are different values in config.json, they are overwritten.
char mqtt_server[40];

//flag for saving data
bool shouldSaveConfig = false;

//callback notifying us of the need to save config
void saveConfigCallback () {
  Serial.println("Should save config");
  shouldSaveConfig = true;
}
void configModeCallback (WiFiManager *myWiFiManager) {
  Serial.println("Entered config mode");
  Serial.println(WiFi.softAPIP());
  //if you used auto generated SSID, print it
  Serial.println(myWiFiManager->getConfigPortalSSID());
}
//////////////////////
void save_data(uint8_t calen[4][4], String sukien[4][3], uint8_t homeser1, uint8_t homeser2){
  //save the custom parameters to FS
    if (shouldSaveConfig) {
      Serial.println("saving config");
   #if defined(ARDUINOJSON_VERSION_MAJOR) && ARDUINOJSON_VERSION_MAJOR >= 6
      DynamicJsonDocument json(1024);
  #else
      DynamicJsonBuffer jsonBuffer;
      JsonObject& json = jsonBuffer.createObject();
  #endif
      //json["gmt_config"] = gmt_config;
      json["mqtt_server"] = mqtt_server;    
      ///addd
      json["event_1"][0] = calen[0][0];
      json["event_1"][1] = calen[0][1];
      json["event_1"][2] = calen[0][2];
      json["event_1"][3] = calen[0][3];
      json["event_1"][4] = sukien[0][2];
      
      json["event_2"][0] = calen[1][0];
      json["event_2"][1] = calen[1][1];
      json["event_2"][2] = calen[1][2];
      json["event_2"][3] = calen[1][3];
      json["event_2"][4] = sukien[1][2];

      json["event_3"][0] = calen[2][0];
      json["event_3"][1] = calen[2][1];
      json["event_3"][2] = calen[2][2];
      json["event_3"][3] = calen[2][3];
      json["event_3"][4] = sukien[2][2];

      json["event_4"][0] = calen[3][0];
      json["event_4"][1] = calen[3][1];
      json["event_4"][2] = calen[3][2];
      json["event_4"][3] = calen[3][3];
      json["event_4"][4] = sukien[3][2];

      json["servo"][0] = homeser1;
      json["servo"][1] = homeser2;

      File configFile = SPIFFS.open("/config.json", "w");
      if (!configFile) {
        Serial.println("failed to open config file for writing");
      }
  
  #if defined(ARDUINOJSON_VERSION_MAJOR) && ARDUINOJSON_VERSION_MAJOR >= 6
      serializeJson(json, Serial);
      serializeJson(json, configFile);
  #else
      json.printTo(Serial);
      json.printTo(configFile);
  #endif
      configFile.close();
      //end save
      shouldSaveConfig = false;
    }
}

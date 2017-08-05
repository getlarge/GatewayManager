/**

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License
   version 2 as published by the Free Software Foundation.

 *******************************

  After first time flashing, push the reboot button ( fix a bootloader known issue )

*/

#include <FS.h>      // this needs to be first, or it all crashes and burns...

#include <ArduinoJson.h>          
//#include <Arduino.h>

#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <WiFiClientSecure.h>
#include <ESP8266HTTPClient.h>        
#include <ESP8266httpUpdate.h>

#include "config.h" 

#include <WiFiManager.h>
#include <Ticker.h>         
#include <Bounce2.h>

ESP8266WiFiMulti WiFiMulti;
Bounce debouncer = Bounce();
Ticker ticker;


void before() {
 // Serial.begin(MY_BAUD_RATE);
#ifndef MY_DEBUG
  Serial.setDebugOutput(false);
#endif
 // resetConfig = true;
 
  checkOtaFile();
  delay(100);
  
   if (_otaSignal == 1 ) {
     //WiFi.persistent(false);
     String ssid = WiFi.SSID();
     String pass = WiFi.psk();
     WiFiMulti.addAP(ssid.c_str(), pass.c_str());
     while (WiFiMulti.run() != WL_CONNECTED) { // use this when using ESP8266WiFiMulti.h
        Serial.println("Attempting Wifi connection.... ");     
        delay(500);    
     }
    Serial.print("WiFi connected.  IP address:");
    Serial.println(WiFi.localIP());  
 
    getUpdated();
  }
  
  Serial.printf("before heap size: %u\n", ESP.getFreeHeap());
  for (uint8_t t = 4; t > 0; t--) { 
      Serial.printf("[SETUP] WAIT %d...\n", t);
      Serial.flush();
      delay(1000);
    }
    
 
  if (wifiResetConfig) { // rajouter un bouton
    WiFiManager wifiManager;
    wifiManager.resetSettings();
  }
  
  if (resetConfig) {
    setDefault(); 
  }

 // delay(1000);
 
  setPins();

  ticker.attach(1.5, tick);

  Serial.println();
  Serial.println(F("mounting FS..."));
  if (SPIFFS.begin()) {
    Serial.println(F("mounted file system"));
    if (SPIFFS.exists(F("/config.json"))) {
      //file exists, reading and loading
      Serial.println(F("reading config file"));
      File configFile = SPIFFS.open("/config.json", "r");
      if (configFile) {
        Serial.println(F("opened config file"));
        size_t size = configFile.size();
        // Allocate a buffer to store contents of the file.
        std::unique_ptr<char[]> buf(new char[size]);
        configFile.readBytes(buf.get(), size);
        DynamicJsonBuffer jsonBuffer;
        JsonObject& json = jsonBuffer.parseObject(buf.get());
        json.printTo(Serial);
        if (json.success()) {
          strcpy(mqtt_server, json["mqtt_server"]);
          strcpy(mqtt_port, json["mqtt_port"]);
          strcpy(mqtt_client, json["mqtt_client"]);
          strcpy(mqtt_user, json["mqtt_user"]);
          strcpy(mqtt_password, json["mqtt_password"]);
//          strcpy(http_server, json["http_server"]);
//          strcpy(http_port, json["http_port"]);
          strcpy(mqtt_topic_in,mqtt_client); 
          strcat(mqtt_topic_in,in); 
          strcpy(mqtt_topic_out,mqtt_client);
          strcat(mqtt_topic_out,out);
          MY_CONTROLLER_URL_ADDRESS = mqtt_server;
          MY_PORT = atoi(mqtt_port);
          MY_MQTT_CLIENT_ID = mqtt_client;
          MY_MQTT_USER = mqtt_user;
          MY_MQTT_PASSWORD = mqtt_password;
          MY_MQTT_SUBSCRIBE_TOPIC_PREFIX = mqtt_topic_in;
          MY_MQTT_PUBLISH_TOPIC_PREFIX = mqtt_topic_out;
          httpServer = mqtt_server;
//          httpPort = atoi(http_port);
        } else {
          Serial.println(F("Failed to load json config"));
        }
      }
    }
  } else {
    Serial.println(F("Failed to mount FS"));
  }

  getDeviceId();
  ticker.detach();

  configManager();
  
}


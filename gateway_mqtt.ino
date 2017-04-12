/**

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License
   version 2 as published by the Free Software Foundation.

 *******************************

*/

#include <FS.h>      // this needs to be first, or it all crashes and burns...

#include <ArduinoJson.h>          
#include <Arduino.h>

#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>

#include <ESP8266HTTPClient.h>        
#include <ESP8266httpUpdate.h>
#include <DNSServer.h>

//#include <ESP8266WebServer.h>
#include <WiFiManager.h>          // https://github.com/tzapu/WiFiManager
#include <Wire.h>
#include <TimeLib.h>
#include <Ticker.h>
#include <Bounce2.h>
#include "config.h" // ne pas déplacer

//AlarmId id;
Bounce debouncer = Bounce();
//Bounce debouncer4 = Bounce(); 
Ticker ticker;

void tick() {
  //toggle state
 // int state = digitalRead(BUILTIN_LED); 
 // digitalWrite(BUILTIN_LED, !state); 
  //entered config mode, make led toggle faster
//  ++count;
//  // when the counter reaches a certain value, start blinking like crazy
//  if (count == 40)
//  {
//    ticker.attach(0.1, tick);
//  }
//  // when the counter reaches yet another value, stop blinking
//  else if (count == 120)
//  {
//    ticker.detach();
//  }
}

void set_pins(){
//    pinMode(BUILTIN_LED, OUTPUT);
    pinMode(MY_INCLUSION_MODE_BUTTON_PIN, INPUT_PULLUP);
    debouncer.attach(MY_INCLUSION_MODE_BUTTON_PIN);
    debouncer.interval(2000);
    //pinMode(MY_INCLUSION_MODE_BUTTON_PIN, INPUT_PULLUP);
   // pinMode(OTA_BUTTON_PIN, INPUT_PULLUP);
    //debouncer4.attach(OTA_BUTTON_PIN);
   // debouncer4.interval(2000);
    Serial.println("pins set");
}

void saveConfigCallback () {
  Serial.println("Should save config");
  shouldSaveConfig = true;
}

//gets called when WiFiManager enters configuration mode
void configModeCallback (WiFiManager *myWiFiManager) {
  Serial.println("Entered config mode");
  Serial.println(WiFi.softAPIP());
  //if you used auto generated SSID, print it
  Serial.println(myWiFiManager->getConfigPortalSSID());
}

void before() {
  for (uint8_t t = 4; t > 0; t--) { // Utile en cas d'OTA ?
      Serial.printf("[SETUP] WAIT %d...\n", t);
      Serial.flush();
      delay(1000);
    }
  set_pins();
  
  if (resetConfig) { // rajouter un bouton
    Serial.println("Resetting config to the inital state");
    SPIFFS.begin();
    delay(10);
    SPIFFS.format();
    WiFiManager wifiManager;
    wifiManager.resetSettings();
    Serial.println("System cleared");
  }
  
 // ticker.attach(0.5, tick);
  Serial.println();
  Serial.println("mounting FS...");
  if (SPIFFS.begin()) {
    Serial.println("mounted file system");
    if (SPIFFS.exists("/config.json")) {
      //file exists, reading and loading
      Serial.println("reading config file");
      File configFile = SPIFFS.open("/config.json", "r");
      if (configFile) {
        Serial.println("opened config file");
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
          Serial.println();
        } else {
          Serial.println("failed to load json config");
        }
      }
    }
  } else {
    Serial.println("failed to mount FS");
  }
  WiFiManager wifiManager;
  wifiManager.setSaveConfigCallback(saveConfigCallback);
  wifiManager.addParameter(&custom_mqtt_server);
  wifiManager.addParameter(&custom_mqtt_port);
  wifiManager.addParameter(&custom_mqtt_client);
  wifiManager.addParameter(&custom_mqtt_user);
  wifiManager.addParameter(&custom_mqtt_password);
  wifiManager.setMinimumSignalQuality();
  //wifiManager.setTimeout(300);

  if (!wifiManager.autoConnect(autoconnect_ssid, ap_pass)) {
    Serial.println("Echec de la connection --> Timeout");
    delay(3000);
    //reset and try again, or maybe put it to deep sleep
    ESP.reset();
    delay(5000);
  }
  Serial.println("connecté (auto)");
  if (shouldSaveConfig) {
    strcpy(mqtt_server, custom_mqtt_server.getValue()); 
    strcpy(mqtt_port, custom_mqtt_port.getValue());
    strcpy(mqtt_client, custom_mqtt_client.getValue()); 
    strcpy(mqtt_user, custom_mqtt_user.getValue());
    strcpy(mqtt_password, custom_mqtt_password.getValue());
    Serial.println("Sauvegarde de la configuration");
    DynamicJsonBuffer jsonBuffer;
    JsonObject& json = jsonBuffer.createObject();
    json["mqtt_server"] = mqtt_server;
    json["mqtt_port"] = mqtt_port;
    json["mqtt_client"] = mqtt_client;
    json["mqtt_user"] = mqtt_user;
    json["mqtt_password"] = mqtt_password;
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
    File configFile = SPIFFS.open("/config.json", "w");
    if (!configFile) {
      Serial.println("Echec de l'ouveture du fichier de config");
    }
    json.printTo(Serial);
    json.printTo(configFile);
    configFile.close();
  }
  Serial.print("IP locale : ");
  Serial.println(WiFi.localIP());
  Serial.print("MQTT config : ");
  Serial.print(MY_CONTROLLER_URL_ADDRESS);
  Serial.print(":");
  Serial.print(MY_PORT);
  Serial.print(" | ");
  Serial.print(MY_MQTT_CLIENT_ID);
  Serial.print(" | ");
  Serial.println(MY_MQTT_USER);
  Serial.print(MY_MQTT_PUBLISH_TOPIC_PREFIX);
  Serial.print(" | ");
  Serial.println(MY_MQTT_SUBSCRIBE_TOPIC_PREFIX);
  //delay(500);
}


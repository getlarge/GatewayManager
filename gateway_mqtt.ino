/**

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License
   version 2 as published by the Free Software Foundation.

 *******************************

*/

#include <FS.h>                   //this needs to be first, or it all crashes and burns...

#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <ESP8266httpUpdate.h>
#include <WiFiManager.h>          //https://github.com/tzapu/WiFiManager
#include <ArduinoJson.h>          //https://github.com/bblanchon/ArduinoJson
#include "config.h"
#include <DS3232RTC.h> //http://github.com/JChristensen/DS3232RTC
#include <Wire.h>
#include <TimeLib.h>
#include <TimeAlarms.h>
#include <Ticker.h>
#include <Bounce2.h>
#include <SPI.h>

AlarmId id;
Bounce debouncer = Bounce();
//Bounce debouncer4 = Bounce(); 
Ticker ticker;
WiFiUDP udp;

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
    pinMode(OTA_BUTTON_PIN, INPUT_PULLUP);
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

void getNTP() { /// Working alone but not inside this program
  WiFi.hostByName(ntpServerName, timeServerIP); 
  sendNTPpacket(timeServerIP); // send an NTP packet to a time server
  // wait to see if a reply is available
  Alarm.delay(1000);
  
  int cb = udp.parsePacket();
  if (!cb) {
    Serial.println("no packet yet");
  }
  else {
    Serial.print("packet received, length=");
    Serial.println(cb);
    // We've received a packet, read the data from it
    udp.read(packetBuffer, NTP_PACKET_SIZE); // read the packet into the buffer

    //the timestamp starts at byte 40 of the received packet and is four bytes,
    // or two words, long. First, esxtract the two words:
    unsigned long highWord = word(packetBuffer[40], packetBuffer[41]);
    unsigned long lowWord = word(packetBuffer[42], packetBuffer[43]);
    // combine the four bytes (two words) into a long integer
    // this is NTP time (seconds since Jan 1 1900):
    unsigned long secsSince1900 = highWord << 16 | lowWord;
    Serial.print("Seconds since Jan 1 1900 = " );
    Serial.println(secsSince1900);

    // now convert NTP time into everyday time:
    Serial.print("Unix time = ");
    // Unix time starts on Jan 1 1970. In seconds, that's 2208988800:
    const unsigned long seventyYears = 2208988800UL;
    // subtract seventy years:
    unsigned long epoch = secsSince1900 - seventyYears;
    // print Unix time:
    Serial.println(epoch);
  }
}

// send an NTP request to the time server at the given address
unsigned long sendNTPpacket(IPAddress& address) {
  Serial.println("sending NTP packet...");
  // set all bytes in the buffer to 0
  memset(packetBuffer, 0, NTP_PACKET_SIZE);
  // Initialize values needed to form NTP request
  // (see URL above for details on the packets)
  packetBuffer[0] = 0b11100011;   // LI, Version, Mode
  packetBuffer[1] = 0;     // Stratum, or type of clock
  packetBuffer[2] = 6;     // Polling Interval
  packetBuffer[3] = 0xEC;  // Peer Clock Precision
  // 8 bytes of zero for Root Delay & Root Dispersion
  packetBuffer[12]  = 49;
  packetBuffer[13]  = 0x4E;
  packetBuffer[14]  = 49;
  packetBuffer[15]  = 52;

  // all NTP fields have been given values, now
  // you can send a packet requesting a timestamp:
  udp.beginPacket(address, 123); //NTP requests are to port 123
  udp.write(packetBuffer, NTP_PACKET_SIZE);
  udp.endPacket();
}

void before() {
  //  setTime(12, 41, 0, 25, 2, 2017);   //set the system time to ...
  //  RTC.set(now());                    //set the RTC from the system time
    setSyncProvider(RTC.get);   // the function to get the time from the RTC}
    if(timeStatus() != timeSet) 
        Serial.println("Unable to sync with the RTC");
    else
        Serial.println("RTC has set the system time");  
//  id = Alarm.timerRepeat(60, Repeats);           // timer for every 15 second
  set_pins();
  //set_debounce();
 // ticker.attach(0.5, tick);
  Serial.println();
 // Alarm.delay(100);
 // SPIFFS.format();
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
          strcpy(mqtt_topic_in,mqtt_client); /// à séquencer 
          strcat(mqtt_topic_in,in); 
          strcpy(mqtt_topic_out,mqtt_client);
          strcat(mqtt_topic_out,out);
          MY_CONTROLLER_URL_ADDRESS = mqtt_server;
          MY_PORT = atoi(mqtt_port);
          //mqttPort = atoi(mqtt_port);
          MY_MQTT_CLIENT_ID = mqtt_client;
          MY_MQTT_USER = mqtt_user;
          MY_MQTT_PASSWORD = mqtt_password;
        //  MY_MQTT_SUBSCRIBE_TOPIC_PREFIX = mqtt_topic_in;
          mqttTopicIn = mqtt_topic_in;
          MY_MQTT_PUBLISH_TOPIC_PREFIX = mqtt_topic_out;
          Serial.println(mqttTopicIn);
          Serial.println(MY_PORT);
        } else {
          Serial.println("failed to load json config");
        }
      }
    }
  } else {
    Serial.println("failed to mount FS");
  }
  
  WiFiManager wifiManager;
  //reset saved settings
  //wifiManager.resetSettings();
  //set config save notify callback
  wifiManager.setSaveConfigCallback(saveConfigCallback);
  wifiManager.addParameter(&custom_mqtt_server);
  wifiManager.addParameter(&custom_mqtt_port);
  wifiManager.addParameter(&custom_mqtt_client);
  wifiManager.addParameter(&custom_mqtt_user);
  wifiManager.addParameter(&custom_mqtt_password);
  //set custom ip for portal
  //wifiManager.setAPConfig(IPAddress(192,168,1,5), IPAddress(192,168,1,1), IPAddress(255,255,255,0));
  //set callback that gets called when connecting to previous WiFi fails, and enters Access Point mode
  //wifiManager.setAPCallback(configModeCallback);
  //set minimum quality of signal so it ignores AP's under that quality
  //defaults to 8%
  wifiManager.setMinimumSignalQuality();
  //wifiManager.setTimeout(180);

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
    //MY_MQTT_SUBSCRIBE_TOPIC_PREFIX = mqtt_topic_in;
    mqttTopicIn = mqtt_topic_in;
    MY_MQTT_PUBLISH_TOPIC_PREFIX = mqtt_topic_out;
    File configFile = SPIFFS.open("/config.json", "w");
    if (!configFile) {
      Serial.println("Echec de l'ouveture du fichier de config");
    }
    json.printTo(Serial);
    json.printTo(configFile);
    configFile.close();
  }
  Serial.println("IP locale");
  Serial.println(WiFi.localIP());
  Serial.println("MQTT config");
  Serial.println(MY_CONTROLLER_URL_ADDRESS);
  Serial.println(MY_PORT);
  Serial.println(MY_MQTT_CLIENT_ID);
  Serial.println(MY_MQTT_USER);
  Serial.println(MY_MQTT_PUBLISH_TOPIC_PREFIX);
  Serial.println(mqttTopicIn);
 // ticker.detach();
}



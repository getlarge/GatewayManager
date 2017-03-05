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

void getNTP() {
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
          //strcpy(mqtt_port, json["mqtt_port"]);
          strcpy(mqtt_client, json["mqtt_client"]);
          strcpy(mqtt_user, json["mqtt_user"]);
          strcpy(mqtt_password, json["mqtt_password"]);
//          strcpy(mqtt_topic_in,mqtt_client); /// à séquencer 
//          strcat(mqtt_topic_in,in); 
          strcpy(mqtt_topic_out,mqtt_client);
          strcat(mqtt_topic_out,out);
          MY_CONTROLLER_URL_ADDRESS = mqtt_server;
          //MY_PORT = mqtt_port;
          MY_MQTT_CLIENT_ID = mqtt_client;
          MY_MQTT_USER = mqtt_user;
          MY_MQTT_PASSWORD = mqtt_password;
        //  MY_MQTT_SUBSCRIBE_TOPIC_PREFIX = mqtt_topic_in;
          MY_MQTT_PUBLISH_TOPIC_PREFIX = mqtt_topic_out;
          Serial.println(mqttTopicOut);
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
  //wifiManager.addParameter(&custom_mqtt_port);
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
    //strcpy(mqtt_port, custom_mqtt_port.getValue());
    strcpy(mqtt_client, custom_mqtt_client.getValue()); 
    strcpy(mqtt_user, custom_mqtt_user.getValue());
    strcpy(mqtt_password, custom_mqtt_password.getValue());
    Serial.println("Sauvegarde de la configuration");
    DynamicJsonBuffer jsonBuffer;
    JsonObject& json = jsonBuffer.createObject();
    json["mqtt_server"] = mqtt_server;
    //json["mqtt_port"] = mqtt_port;
    json["mqtt_client"] = mqtt_client;
    json["mqtt_user"] = mqtt_user;
    json["mqtt_password"] = mqtt_password;
    //strcpy(mqtt_topic_in,mqtt_client);
    //strcat(mqtt_topic_in,in);
    strcpy(mqtt_topic_out,mqtt_client);
    strcat(mqtt_topic_out,out);
    MY_CONTROLLER_URL_ADDRESS = mqtt_server;
    //MY_PORT = mqtt_port;
    MY_MQTT_CLIENT_ID = mqtt_client;
    MY_MQTT_USER = mqtt_user;
    MY_MQTT_PASSWORD = mqtt_password;
    //MY_MQTT_SUBSCRIBE_TOPIC_PREFIX = mqtt_topic_in;
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
  ticker.detach();
}

#include <MySensors.h>

void printDigits(int digits) {
    // utility function for digital clock display: prints preceding colon and leading 0
    Serial.print(':');
    if(digits < 10)
      Serial.print('0');
      Serial.print(digits);
}

void digitalClockDisplay(void) {
    // digital clock display of the time
    Serial.print(hour());
    printDigits(minute());
    printDigits(second());
    Serial.print(' ');
    Serial.print(day());
    Serial.print(' ');
    Serial.print(month());
    Serial.print(' ');
    Serial.print(year()); 
    Serial.println(); 
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrivé [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
  if ((char)payload[0] == '1') {
    //digitalWrite(BUILTIN_LED, LOW);   
    //digitalWrite(D7, HIGH); 
  }
  
  if ((char)payload[0]=='0') {
    //digitalWrite(BUILTIN_LED, HIGH); 
   // digitalWrite(D7, LOW); 
  }
}

boolean reconnect() {
  // variables de la librairie MQTT du core MySensor 
  if (_MQTT_client.connect(MY_MQTT_CLIENT_ID),(MY_MQTT_USER),(MY_MQTT_PASSWORD)) {
    _MQTT_client.setServer(MY_CONTROLLER_URL_ADDRESS, MY_PORT);
  //  _MQTT_client.setCallback(callback);
    _MQTT_client.publish(MY_MQTT_PUBLISH_TOPIC_PREFIX, "Client connecté");
    _MQTT_client.subscribe(MY_MQTT_SUBSCRIBE_TOPIC_PREFIX);
   // ticker.detach();
  }
  return _MQTT_client.connected();
}

void setup(void) {
    Serial.println();
    Serial.println("Starting UDP");
    udp.begin(localPort);
    Serial.print("Local port: ");
    Serial.println(udp.localPort());
    //Alarm.alarmRepeat(p1_on_hour,p1_on_min,p1_on_sec,Prog1_On);  
    //Alarm.alarmRepeat(p1_off_hour,p1_off_min,p1_off_sec,Prog1_Off); 
    //Alarm.alarmRepeat(dowSaturday,8,30,30,WeeklyAlarm);  // 8:30:30 every Saturday
    //Alarm.alarmRepeat(16,0,0,WeeklyAlarm);  // 8:30:30 every Saturday
    //id = Alarm.timerRepeat(60, Repeats);           // timer for every 15 seconds
    //id = Alarm.timerRepeat(120, Repeats2);      // timer for every 2 seconds
    //Alarm.timerOnce(10, OnceOnly);            // called once after 10 seconds
    //set_pins();
    digitalClockDisplay();
    //getNTP();
    //requestTime();  
    for (uint8_t t = 4; t > 0; t--) { // Utile en cas d'OTA ?
      Serial.printf("[SETUP] WAIT %d...\n", t);
      Serial.flush();
      Alarm.delay(1000);
    }
}

void presentation() {
  sendSketchInfo(SKETCH_NAME, SKETCH_MAJOR_VER "." SKETCH_MINOR_VER);
  
}

void receiveTime(unsigned long controllerTime) {
  // Ok, set incoming time 
  Serial.print("Time value received: ");
  Serial.println(controllerTime);
  RTC.set(controllerTime);
  setSyncProvider(RTC.get);   // the function to get the time from the RTC
  timeReceived = true;
}

void loop(void) {
   debouncer.update();
   int value1 = debouncer.read();
  if ( ! executeOnce) {
  // getNTP();
  //  digitalClockDisplay(); 
    executeOnce = true; 
  }
  

   if (value1 == LOW) {
    Serial.println("Appui long détecté, demande de config wifi");
    value1 = HIGH;
    wifimanager_ondemand();
  }
  
   
  if (digitalRead(OTA_BUTTON_PIN) == LOW ) {
   // ticker.attach(0.2, tick);
    t_httpUpdate_return ret = ESPhttpUpdate.update("https://getlarge.eu/firmware/Gatewy_MySensor.ino.bin");
    switch (ret) {
      case HTTP_UPDATE_FAILED: // à corriger, blocking loop
        Serial.printf("HTTP_UPDATE_FAILD Error (%d): %s", ESPhttpUpdate.getLastError(), ESPhttpUpdate.getLastErrorString().c_str());
        Serial.println();
        return;
       // break;

      case HTTP_UPDATE_NO_UPDATES:
        Serial.println("HTTP_UPDATE_NO_UPDATES");
        break;

      case HTTP_UPDATE_OK:
        Serial.println("HTTP_UPDATE_OK");
        ticker.detach();
        break;
    }
  }
 // unsigned long Now=millis();
  // If no time has been received yet, request it every 10 second from controller
  // When time has been received, request update every hour
//  if ((!timeReceived && (Now-lastRequest) > (10UL*1000UL))
//    || (timeReceived && (Now-lastRequest) > (60UL*1000UL*60UL))) {
//    // Request time from controller. 
//    Serial.println("requesting time");
//    requestTime(); 
//    lastRequest = Now;
//  }
//  if (Now-lastUpdate > 10000) {
//    digitalClockDisplay();
//    lastUpdate = Now;
//   }
   //Alarm.delay(500);  //?
   
  
  if ( WiFi.status() != WL_CONNECTED) {
    long Now = millis();
    if (Now - lastWifiReconnectAttempt > 5 * 1000) {
      ++count;
      lastWifiReconnectAttempt = Now;
      if (count == 1) {
      }
      else if (count == 5) {
        Serial.println("Wifi perdu, réessaye au bout de 5 seconds sans connection --> mode config");
        wifimanager_ondemand();
        lastWifiReconnectAttempt = 0;
        count = 0;
      }
//    long Now = millis();
//    if (Now - lastWifiReconnectAttempt > 5 * 1000) {
//      lastWifiReconnectAttempt = Now;
//      Serial.println("Wifi perdu, réessaye au bout de 5 seconds sans connection --> mode config");
//      wifimanager_ondemand();
//      lastWifiReconnectAttempt = 0;
     }
   }

   if (!_MQTT_client.connected()) {
    //ticker.attach(0.2, tick);
    Serial.println("reconnexion mqtt (loop)");
    long Now = millis();
    if (Now - lastMqttReconnectAttempt > 5 * 1000) {
      lastMqttReconnectAttempt = Now;
      // Attempt to reconnect
      if (reconnect()) {
        lastMqttReconnectAttempt = 0;
      }
    }
  }
  
    else if ((value1 == HIGH) && (_MQTT_client.connected()) && (WiFi.status() != WL_CONNECTED)) {
  Alarm.delay(0);
  }
}



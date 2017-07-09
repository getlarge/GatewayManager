void tick() {
  int state = digitalRead(STATE_LED); 
  digitalWrite(STATE_LED, !state); 
}

void setPins() {
    pinMode(STATE_LED, OUTPUT);
    digitalWrite(STATE_LED, HIGH);
    pinMode(MY_INCLUSION_MODE_BUTTON_PIN, INPUT_PULLUP);
    debouncer.attach(MY_INCLUSION_MODE_BUTTON_PIN);
    debouncer.interval(3000);
    Serial.println(F("Pins set"));
}

void checkOtaFile() {
  SPIFFS.begin();
  delay(10);
  // check for properties file
  File f = SPIFFS.open(fName, "r");
  if (!f ) {
    f = SPIFFS.open(fName, "w");
    if (!f) {
      Serial.println(F("OTA file open failed"));
    }
    else {
      Serial.println(F("====== Writing to OTA file ========="));
      f.println(_otaSignal);
      f.close();
    }
  }
  else {
    Serial.println(F("OTA file exists. Reading."));
    while (f.available()) {
      // read line by line from the file
      String str = f.readStringUntil('\n');
      Serial.println(str);
      _otaSignal = str.toInt();
    }
    f.close();
  }
}
   
void updateOtaFile() {
  File f = SPIFFS.open(fName, "w");
  if (!f) {
    Serial.println(F("OTA file open failed"));
  }
  else {
    Serial.println(F("====== Writing to OTA file ========="));

    f.println(_otaSignal);
    Serial.println(F("OTA file updated"));
    f.close();
  }
}

void connectWifi() {
 //   WiFiManager wifiManager;
    String ssid = WiFi.SSID();
    String pass = WiFi.psk();
    WiFi.begin(ssid.c_str(), pass.c_str());
   
    //while (WiFiMulti.run() != WL_CONNECTED) { //use this when using ESP8266WiFiMulti.h
    while (WiFi.status() != WL_CONNECTED) { 
       Serial.print("Attempting Wifi connection....");     
       delay(1000);    
    }
    Serial.println();
    Serial.print("WiFi connected.  IP address:");
    Serial.println(WiFi.localIP());    
}

void getUpdated() {
//  if((WiFi.status() == WL_CONNECTED)) {
//     _MQTT_ethClient.stop();
//     _MQTT_client.disconnect();
    _otaSignal = 0;
    updateOtaFile();
   // Serial.printf("before httpUpdate heap size: %u\n", ESP.getFreeHeap());
    Serial.println(F("Update sketch..."));

    t_httpUpdate_return ret = ESPhttpUpdate.update(otaUrl,"", httpsFingerprint);
    //t_httpUpdate_return ret = ESPhttpUpdate.update("https://app.getlarge.eu/firmware/GatewayMQTT.ino.bin","","1D AE 00 E7 68 70 87 09 A6 1D 27 76 F5 85 C0 F3 AB F2 60 9F");
    //t_httpUpdate_return ret = ESPhttpUpdate.update(httpServer, httpPort, url, currentVersion, httpsFingerprint);
   
    switch(ret) {
      case HTTP_UPDATE_FAILED:
          Serial.printf("HTTP_UPDATE_FAILD Error (%d): %s", ESPhttpUpdate.getLastError(), ESPhttpUpdate.getLastErrorString().c_str());
          Serial.println();
          break;
      case HTTP_UPDATE_NO_UPDATES:
          Serial.println(F("HTTP_UPDATE_NO_UPDATES"));
          break;
      case HTTP_UPDATE_OK:
          Serial.println(F("HTTP_UPDATE_OK"));
          break;
     }    
//  }
  // ticker.detach();
}

void checkButton() {
  debouncer.update();
  int value = debouncer.read();
  if (value == LOW) {
      Serial.println(F("Long push detected, ask for config"));
      manualConfig = true;
      configManager();
  }
}

void getDeviceId() {
  char msgBuffer[8];         
  char *espChipId;
  float chipId = ESP.getChipId();
  espChipId = dtostrf(chipId, 8, 0, msgBuffer);
  strcpy(deviceId,devicePrefix); 
  strcat(deviceId,espChipId);
}

void setReboot() { // Boot to sketch
//    pinMode(STATE_LED, OUTPUT);
//    digitalWrite(STATE_LED, HIGH);
//    pinMode(MY_INCLUSION_MODE_BUTTON_PIN, OUTPUT);
//    digitalWrite(MY_INCLUSION_MODE_BUTTON_PIN, HIGH);
//    pinMode(D8, OUTPUT);
//    digitalWrite(D8, LOW);
    Serial.println(F("Pins set for reboot"));
//    Serial.flush();
//    yield(); yield(); delay(500);
    delay(5000);
    ESP.reset(); //ESP.restart();
    delay(2000);
}

void setPinsRebootUart() { // Boot to Uart
    pinMode(STATE_LED, OUTPUT);
    digitalWrite(STATE_LED, HIGH);
    pinMode(MY_INCLUSION_MODE_BUTTON_PIN, OUTPUT);
    digitalWrite(MY_INCLUSION_MODE_BUTTON_PIN, HIGH);
    pinMode(D8, OUTPUT);
    digitalWrite(D8, LOW);
    Serial.println(F("Pins set for reboot"));
}

void setDefault() { 
    ticker.attach(2, tick);
    Serial.println(F("Resetting config to the inital state"));
    resetConfig = false;
    SPIFFS.begin();
    delay(10);
    SPIFFS.format();
    WiFiManager wifiManager;
    wifiManager.resetSettings();
    delay(100);
    Serial.println(F("System cleared"));
    ticker.detach();
    Serial.println(ESP.eraseConfig());
    setReboot();
}

void saveConfigCallback() {
  Serial.println(F("Should save config"));
  shouldSaveConfig = true;
}

////gets called when WiFiManager enters configuration mode
//void configModeCallback(WiFiManager *myWiFiManager) {
// // delay(1000);
//  Serial.println(F("Entered config mode"));
//  Serial.println(WiFi.softAPIP());
//    checkButton();
//}


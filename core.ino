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
  File f = SPIFFS.open(otaFile, "r");
  if (!f ) {
    f = SPIFFS.open(otaFile, "w");
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
  File f = SPIFFS.open(otaFile, "w");
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
      value = HIGH;
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

//gets called when WiFiManager enters configuration mode
void configModeCallback(WiFiManager *myWiFiManager) {
 // delay(1000);
  Serial.println(F("Entered config mode"));
  Serial.println(WiFi.softAPIP());
  checkButton();
}


void digitalClockDisplay() {
  // digital clock display of the time
  Serial.print(hour());
  printDigits(minute());
  printDigits(second());
  Serial.print(" ");
  Serial.print(day());
  Serial.print(".");
  Serial.print(month());
  Serial.print(".");
  Serial.print(year());
  Serial.println();
}

void printDigits(int digits) {
  // utility for digital clock display: prints preceding colon and leading 0
  Serial.print(":");
  if (digits < 10)
    Serial.print('0');
  Serial.print(digits);
}

/*-------- NTP code ----------*/

const int NTP_PACKET_SIZE = 48; // NTP time is in the first 48 bytes of message
byte packetBuffer[NTP_PACKET_SIZE]; //buffer to hold incoming & outgoing packets

time_t getNtpTime() {
  IPAddress ntpServerIP; // NTP server's ip address

  while (Udp.parsePacket() > 0) ; // discard any previously received packets
  Serial.println("Transmit NTP Request");
  // get a random server from the pool
  WiFi.hostByName(ntpServerName, ntpServerIP);
  Serial.print(ntpServerName);
  Serial.print(": ");
  Serial.println(ntpServerIP);
  sendNTPpacket(ntpServerIP);
  uint32_t beginWait = millis();
  while (millis() - beginWait < 1500) {
    int size = Udp.parsePacket();
    if (size >= NTP_PACKET_SIZE) {
      Serial.println("Receive NTP Response");
      Udp.read(packetBuffer, NTP_PACKET_SIZE);  // read packet into the buffer
      unsigned long secsSince1900;
      // convert four bytes starting at location 40 to a long integer
      secsSince1900 =  (unsigned long)packetBuffer[40] << 24;
      secsSince1900 |= (unsigned long)packetBuffer[41] << 16;
      secsSince1900 |= (unsigned long)packetBuffer[42] << 8;
      secsSince1900 |= (unsigned long)packetBuffer[43];
      return secsSince1900 - 2208988800UL + timeZone * SECS_PER_HOUR;
    }
  }
  Serial.println("No NTP Response :-(");
  return 0; // return 0 if unable to get the time
}

// send an NTP request to the time server at the given address
void sendNTPpacket(IPAddress& address)
{
  Serial.println(F("sending NTP packet..."));
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
  Udp.beginPacket(address, 123); //NTP requests are to port 123
  Udp.write(packetBuffer, NTP_PACKET_SIZE);
  Udp.endPacket();
}

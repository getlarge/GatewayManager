///////////////////////////////////////////////////////////////////////////////////
//once settings for connection to wifi and server done, call MySensors lib     //
///////////////////////////////////////////////////////////////////////////////////


void receiveOtaSignal(int otaSignal) {
  Serial.print(F("OTA signal received: "));
  Serial.println(otaSignal);
  _otaSignal = otaSignal;
  updateOtaFile();
  Serial.printf("otaSignal heap size: %u\n", ESP.getFreeHeap());
}

void presentation() {
  sendSketchInfo(SKETCH_NAME, SKETCH_VERSION);
}

//void receive() {
//  //message custom, reset signal
//}

void setup() {

  //ticker.detach();
//  Serial.flush();
//  yield(); delay(500);
  Serial.println();

  
}

time_t prevDisplay = 0; // when the digital clock was displayed

void loop(void) {

   if ( ! executeOnce) {
     executeOnce = true;
    // gatewayTransportInit();
  }   
  
  checkButton();

  if (_otaSignal == 1 || resetConfig) {
    setReboot();
  }
   
  if (WiFi.status() != WL_CONNECTED && _otaSignal == 0) {
    ticker.attach(0.1, tick);
    ++wifiFailCount;
    if (wifiFailCount == 10) {
      ticker.detach();
      configManager();
    }  
  //checkButton();
  }

  if (!_MQTT_client.connected() && WiFi.status() == WL_CONNECTED && _otaSignal == 0) {
    ticker.attach(0.3, tick);
    checkButton();
    ++mqttFailCount;
    if ( mqttFailCount == 6  ) {
      configManager();
    }
  }    

  if (timeStatus() != timeNotSet) {
    if (now() != prevDisplay) { //update the display only if time has changed
      prevDisplay = now();
      digitalClockDisplay();
    }
  }

  if (_MQTT_client.connected() && WiFi.status() == WL_CONNECTED && _otaSignal == 0) {
    wifiFailCount = 0;
    mqttFailCount = 0;
    ticker.detach();
    digitalWrite(STATE_LED, HIGH);
    unsigned long now = millis();
    if (now-lastUpdate > 30000) {
      Serial.printf("loop heap size: %u\n", ESP.getFreeHeap());
      lastUpdate = now;      
    }
//    if (now-lastYield > 20000) {
//      Serial.println(F("Cleaning memory"));
//      yield();
//      Serial.flush();
//      delay(500);
//      lastYield = now;      
//    }
     
  }
}

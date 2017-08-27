///////////////////////////////////////////////////////////////////////////////////
//once settings for connection to wifi and server done, call MySensors lib     //
///////////////////////////////////////////////////////////////////////////////////

#include <MySensors.h>


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
    // Synchronize time useing SNTP. This is necessary to verify that
  // the TLS certificates offered by the server are currently valid.
  Serial.print(F("Setting time using SNTP"));
  configTime(8 * 3600, 0, "pool.ntp.org", "time.nist.gov");
  time_t now = time(nullptr);
  while (now < 1000) {
    delay(500);
    Serial.print(".");
    now = time(nullptr);
  }
  Serial.println("");
  struct tm timeinfo;
  gmtime_r(&now, &timeinfo);
  Serial.print("Current time: ");
  Serial.print(asctime(&timeinfo));
  
  //ticker.detach();
//  Serial.flush();
//  yield(); delay(500);
  Serial.println();

}

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
    if ( mqttFailCount == 10  ) {
      configManager();
    }
  }    
  
  if (_MQTT_client.connected() && WiFi.status() != WL_CONNECTED && _otaSignal == 0) {
    wifiFailCount = 0;
    mqttFailCount = 0;
    unsigned long now = millis();
    if (now-lastUpdate > 10000) {
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
     
    if (STATE_LED == LOW) {
      digitalWrite(STATE_LED, HIGH);
    }
  }
}

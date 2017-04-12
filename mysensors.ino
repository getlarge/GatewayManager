#include <SPI.h>
#include <MySensors.h>

void setup(void) {
    Serial.println();
//    for (uint8_t t = 4; t > 0; t--) { // Utile en cas d'OTA ?
//      Serial.printf("[SETUP] WAIT %d...\n", t);
//      Serial.flush();
//      delay(1000);
//    }
    set_pins();   
}

void presentation() {
  sendSketchInfo(SKETCH_NAME, SKETCH_MAJOR_VER "." SKETCH_MINOR_VER);
  
}

void receiveOtaSignal(int otaSignal) { 
  Serial.print("OTA signal received: ");
  Serial.println(otaSignal);
  if (otaSignal == 1 ) {
    t_httpUpdate_return ret = ESPhttpUpdate.update("http://ivi.exostic/firmware/gateway_mqtt.ino.bin");
    switch (ret) {
      case HTTP_UPDATE_FAILED:
        Serial.printf("HTTP_UPDATE_FAILD Error (%d): %s", ESPhttpUpdate.getLastError(), ESPhttpUpdate.getLastErrorString().c_str());
        Serial.println();
       // return;
        break;
      case HTTP_UPDATE_NO_UPDATES:
        Serial.println("HTTP_UPDATE_NO_UPDATES");
        break;
      case HTTP_UPDATE_OK:
        Serial.println("HTTP_UPDATE_OK");
        //resetConfig = true;
        //ticker.detach();
        break;
    }
  }
  otaSignalReceived = true;
}

void loop(void) {
  debouncer.update();
  int value1 = debouncer.read();
  if (value1 == LOW) {
    Serial.println("Appui long détecté, demande de config wifi");
    value1 == HIGH; /// à modifier par l'exemple "retrigger" de la lib bounce2
    wifimanager_ondemand();
  }
  
  if ( ! executeOnce) {
    executeOnce = true;
  }

  if ( WiFi.status() != WL_CONNECTED) {
    long Now = millis();
    if (Now - lastWifiReconnectAttempt > interval) {
      ++wifiCount;
      lastWifiReconnectAttempt = Now;
      if (wifiCount == 1) {
        Serial.println("Connexion Wifi indisponible, test dans 5 secondes");
      }
      else if (wifiCount == 5) {
        Serial.println("Connexion Wifi infructueuse après 5 essais --> mode config");
        wifimanager_ondemand();
        lastWifiReconnectAttempt = 0;
        wifiCount = 0;
      }
     }
   }

  if (!_MQTT_client.connected()) {
   // mqttReconnect();
    ++mqttCount;
    if (mqttCount == 1) {
      Serial.println("Connexion MQTT indisponible, test dans 5 secondes");
    }
    else if (mqttCount == 5) {
      Serial.println("Connexion MQTT infructueuse après 5 essais --> mode config");
      wifimanager_ondemand();
      mqttCount = 0;
    }
  }    
  
  else if ((value1 == HIGH) && (_MQTT_client.connected()) && (WiFi.status() != WL_CONNECTED)) {
  //   delay(0);
     digitalClockDisplay();
  }
}


#include <SPI.h>
#include <MySensors.h>
//WiFiClientSecure MqttWifiClient;
//PubSubClient MqttClient(MqttWifiClient);

//void mqttInit() {
//  MqttClient.setServer(mqttServer, mqttPort);
//  MqttClient.setCallback(incomingMQTT);
//  //MqttClient.connect((mqttClient),(mqttUser),(mqttPassword));
//  //MqttClient.subscribe(mqttTopicIn);
//  Serial.println("Connecté au serveur MQTT : ");
//  Serial.println(mqttServer);
//}
//
//void mqttReconnect() { //blocking loop
//  while (!MqttClient.connected()) {
//      Serial.print("Attempting MQTT connection 11...");
//      if (MqttClient.connect(MY_MQTT_CLIENT_ID, MY_MQTT_USER, MY_MQTT_PASSWORD)) {
//          delay(500);
//          lastMqttReconnectAttempt=0;
//          mqttCount = 0;
//          Serial.println("connected");
//          //MqttClient.publish(mqttTopicOut, "Check 1-2 1-2");
//          MqttClient.subscribe(mqttTopicIn);
//       }
//       else {
//         Serial.print("failed, rc=");
//         Serial.print(MqttClient.state());
//         Serial.println(" try again in 5 seconds");
//         ++mqttCount;
//       }
//       if (mqttCount == 5) {
//         Serial.println("Connexion MQTT infructueuse après 5 essais --> mode config");
//         lastMqttReconnectAttempt = 0;
//         mqttCount = 0;
//         wifimanager_ondemand();    
//       }
//  } 
//}


void setup(void) {
    Serial.println();
//    for (uint8_t t = 4; t > 0; t--) { // Utile en cas d'OTA ?
//      Serial.printf("[SETUP] WAIT %d...\n", t);
//      Serial.flush();
//      delay(1000);
//    }
////    Serial.println("Starting UDP");
//    udp.begin(localPort);
//    Serial.print("Local port: ");
//    Serial.println(udp.localPort());
  //  sync_time();
    set_pins();
    //requestTime();  
   
}

void presentation() {
  sendSketchInfo(SKETCH_NAME, SKETCH_MAJOR_VER "." SKETCH_MINOR_VER);
  
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
      Serial.print("MQTT config : ");
  Serial.print(MY_CONTROLLER_URL_ADDRESS);
  Serial.print(":");
  Serial.print(MY_PORT);
  Serial.print(" | ");
  Serial.print(MY_MQTT_CLIENT_ID);
  Serial.print(" | ");
  Serial.print(MY_MQTT_USER);
  Serial.print(" | ");
  Serial.println(MY_MQTT_PASSWORD);
  Serial.print(MY_MQTT_PUBLISH_TOPIC_PREFIX);
  Serial.print(" | ");
  Serial.println(MY_MQTT_SUBSCRIBE_TOPIC_PREFIX);
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

//  if (!_MQTT_client.connected()) {
//   // mqttReconnect();
//    ++mqttCount;
//    if (mqttCount == 1) {
//      Serial.println("Connexion MQTT indisponible, test dans 5 secondes");
//    }
//    else if (mqttCount == 5) {
//      Serial.println("Connexion MQTT infructueuse après 5 essais --> mode config");
//      wifimanager_ondemand();
//      mqttCount = 0;
//    }
//  }    
 
  //if (value1 == LOW) {
  if (digitalRead(OTA_BUTTON_PIN) == LOW ) {
   // ticker.attach(0.2, tick);
    t_httpUpdate_return ret = ESPhttpUpdate.update("http://ivi.exostic/firmware/gateway_mqtt.ino.bin");
    switch (ret) {
      case HTTP_UPDATE_FAILED: // à corriger, blocking loop
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
        ticker.detach();
        break;
    }
  }
  
  else if ((value1 == HIGH) && (_MQTT_client.connected()) && (WiFi.status() != WL_CONNECTED)) {
  //   delay(0);
  }
}


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
   // _MQTT_client.subscribe(MY_MQTT_SUBSCRIBE_TOPIC_PREFIX "/+/+/+/+/+");
    _MQTT_client.subscribe(mqttTopicIn);

   // ticker.detach();
  }
  return _MQTT_client.connected();
}

void setup(void) {
    Serial.println();
    if (reconnect()) {
        lastMqttReconnectAttempt = 0;}
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
    reconnect();
  // getNTP();
  //  digitalClockDisplay(); 
    executeOnce = true; 
  }
  
  if (value1 == LOW) {
    Serial.println("Appui long détecté, demande de config wifi");
    value1 == HIGH;
    wifimanager_ondemand();
  }
   if ( WiFi.status() != WL_CONNECTED) {
    long Now = millis();
    if (Now - lastWifiReconnectAttempt > 5 * 1000) {
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
//    long Now = millis();
//    if (Now - lastWifiReconnectAttempt > 5 * 1000) {
//      lastWifiReconnectAttempt = Now;
//      Serial.println("Wifi perdu, réessaye au bout de 5 seconds sans connection --> mode config");
//      wifimanager_ondemand();
//      lastWifiReconnectAttempt = 0;
     }
   }

  if (!_MQTT_client.connected()) {
    long Now = millis();
    if (Now - lastMqttReconnectAttempt > 5 * 1000) { 
      ++mqttCount;
      lastMqttReconnectAttempt = Now;
      if (mqttCount == 1) {
        Serial.println("Connexion MQTT indisponible, test dans 5 secondes");
        if (reconnect()) {
          lastMqttReconnectAttempt = 0;
        }
      }
    }
    else if (mqttCount == 5) {
      Serial.println("Connexion MQTT infructueuse après 5 essais --> mode config");
      wifimanager_ondemand();
      lastMqttReconnectAttempt = 0;
      mqttCount = 0;
      }
  }    
  
  if (digitalRead(OTA_BUTTON_PIN) == LOW ) {
   // ticker.attach(0.2, tick);
    t_httpUpdate_return ret = ESPhttpUpdate.update("https://getlarge.eu/firmware/Gateway_MySensor.ino.bin");
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
  
  else if ((value1 == HIGH) && (_MQTT_client.connected()) && (WiFi.status() != WL_CONNECTED)) {
     Alarm.delay(0);
  }
}


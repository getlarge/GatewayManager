///////////////////////////////////////////////////////////////////////////////////
//    function to modifiy, save, settings for connection to wifi and server      //
///////////////////////////////////////////////////////////////////////////////////

void configManager() {
  
  WiFiManagerParameter custom_mqtt_server("server", "mqtt server", mqtt_server, 20);
  WiFiManagerParameter custom_mqtt_port("port", "mqtt port", mqtt_port, 6);
  WiFiManagerParameter custom_mqtt_client("client", "mqtt client", mqtt_client, 25);
  WiFiManagerParameter custom_mqtt_user("user", "mqtt user", mqtt_user, 15);
  WiFiManagerParameter custom_mqtt_password("password", "mqtt password", mqtt_password, 20);
  //WiFiManagerParameter custom_http_server("httpServer", "http server", http_server, 40);
  //WiFiManagerParameter custom_http_port("httpPort", "http port", http_port, 6);

  Serial.println(F("Wifi config mode opening"));
  Serial.printf("before config mode heap size: %u\n", ESP.getFreeHeap());
  //_MQTT_client.disconnect();
  //_MQTT_ethClient.stop();
  ticker.attach(0.5, tick);
  configCount++;
  
  WiFiManager wifiManager;
#ifdef MY_DEBUG 
  wifiManager.setDebugOutput(true);
#endif
#ifndef MY_DEBUG 
  wifiManager.setDebugOutput(false);
#endif
//  wifiManager.setAPCallback(configModeCallback);
//  wifiManager.setBreakAfterConfig(true);
  wifiManager.setSaveConfigCallback(saveConfigCallback);
  wifiManager.setMinimumSignalQuality(10);
  //wifiManager.setConnectTimeout(30);
  
  String script;
  script += "<script>";
  script += "document.addEventListener('DOMContentLoaded', function() {";
  script +=     "var params = window.location.search.substring(1).split('&');";
  script +=     "for (var param of params) {";
  script +=         "param = param.split('=');";
  script +=         "try {";
  script +=             "document.getElementById( param[0] ).value = param[1];";
  script +=         "} catch (e) {";
  script +=             "console.log('WARNING param', param[0], 'not found in page');";
  script +=         "}";
  script +=     "}";
  script += "});";
  script += "</script>";
  wifiManager.setCustomHeadElement(script.c_str());
  /// WILL ADD STYLES IN THE FUTURE ...
  //WiFiManagerParameter custom_text("<p>This is just a text paragraph</p>");
  //wifiManager.addParameter(&custom_text);
  //wifiManager.setCustomHeadElement("<style>html{filter: invert(100%); -webkit-filter: invert(100%);}</style>");
  
  wifiManager.addParameter(&custom_mqtt_server);
  wifiManager.addParameter(&custom_mqtt_port);
  wifiManager.addParameter(&custom_mqtt_client);
  wifiManager.addParameter(&custom_mqtt_user);
  wifiManager.addParameter(&custom_mqtt_password);
//  wifiManager.addParameter(&custom_http_server);
//  wifiManager.addParameter(&custom_http_port);
  
  configMode = 1;

  // When no credentials or asking ...
  if ((configCount > 1 && manualConfig == true) || ( mqttServer == "")) {
    Serial.println(F("Manual config access"));
    wifiManager.setConfigPortalTimeout(configTimeout);
    //wifiManager.startConfigPortal(deviceId, devicePass);
    wifiManager.startConfigPortal(deviceId);  
  }

  // When wifi is already connected but connection got interrupted ...
  if (((configCount > 1 && _MQTT_client.connected() && WiFi.status() == WL_CONNECTED) || (configCount > 1 && !_MQTT_client.connected() && WiFi.status() == WL_CONNECTED)) && manualConfig == false) { 
  //if ((configCount > 1 && _MQTT_client.connected() && WiFi.status() == WL_CONNECTED) || (configCount > 1 && !_MQTT_client.connected() && WiFi.status() == WL_CONNECTED)) {
    Serial.println(F("User config access"));
    wifiManager.setConfigPortalTimeout(configTimeout);
    wifiManager.autoConnect(deviceId); 
  }
  
  // After first start or hard reset...
  //else if (!wifiManager.autoConnect(deviceId, devicePass)) {
  if (!wifiManager.autoConnect(deviceId)) {
    Serial.println(F("Connection failure --> Timeout"));
    delay(3000);
   //reset and try again, or maybe put it to deep sleep
    setReboot();
  }

  else {
    if (shouldSaveConfig) {
      strcpy(mqtt_server, custom_mqtt_server.getValue()); 
      strcpy(mqtt_port, custom_mqtt_port.getValue());
      strcpy(mqtt_client, custom_mqtt_client.getValue()); 
      strcpy(mqtt_user, custom_mqtt_user.getValue());
      strcpy(mqtt_password, custom_mqtt_password.getValue());
  //    strcpy(http_server, custom_http_server.getValue()); 
  //    strcpy(http_port, custom_http_port.getValue());
      Serial.println(F("Saving config"));
      DynamicJsonBuffer jsonBuffer;
      JsonObject& json = jsonBuffer.createObject();
      json["mqtt_server"] = mqtt_server;
      json["mqtt_port"] = mqtt_port;
      json["mqtt_client"] = mqtt_client;
      json["mqtt_user"] = mqtt_user;
      json["mqtt_password"] = mqtt_password;
  //    json["http_server"] = http_server;
  //    json["http_port"] = http_port;
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
      httpServer = mqtt_server;
      //httpPort = atoi(http_port);
      File configFile = SPIFFS.open("/config.json", "w");
      if (!configFile) {
        Serial.println(F("Failed to open config file"));
      }
    json.printTo(Serial);
    json.printTo(configFile);
    configFile.close();
  }
  

  Serial.print(F("Config mode counter : "));
  Serial.println(configCount);
  Serial.println(F("Config mode closed"));
  configMode = 0;
  manualConfig = false;
  ticker.detach();
  digitalWrite(STATE_LED, HIGH);
 // delay(100);
 
  Serial.println(F("====== Connections successful ========="));
  Serial.printf("connection heap size: %u\n", ESP.getFreeHeap());
  Serial.println();
  Serial.print(F("MQTT config : "));
  Serial.print(MY_CONTROLLER_URL_ADDRESS);
  Serial.print(F(":"));
  Serial.print(MY_PORT);
  Serial.print(F(" | "));
  Serial.print(MY_MQTT_CLIENT_ID);
  Serial.print(F(" | "));
  Serial.println(MY_MQTT_USER);
  Serial.print(MY_MQTT_PUBLISH_TOPIC_PREFIX);
  Serial.print(F(" | "));
  Serial.println(MY_MQTT_SUBSCRIBE_TOPIC_PREFIX);
  Serial.println(F("Config HTTP: "));
  Serial.print(httpServer);
  Serial.print(F(":"));
  Serial.println(httpPort);
  Serial.println(F("=================="));
  Serial.println();
  
  }
  
}

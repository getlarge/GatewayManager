void wifimanager_ondemand() {
  ticker.attach(0.5, tick);

  WiFiManager wifiManager;
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
  wifiManager.setSaveConfigCallback(saveConfigCallback);
  wifiManager.addParameter(&custom_mqtt_server);
  wifiManager.addParameter(&custom_mqtt_port);
  wifiManager.addParameter(&custom_mqtt_client);
  wifiManager.addParameter(&custom_mqtt_user);
  wifiManager.addParameter(&custom_mqtt_password);
  wifiManager.setMinimumSignalQuality();
  //wifiManager.setTimeout(180);
  char msgBuffer[10];          
  char *espChipId;
  float chipId = ESP.getChipId();
  espChipId = dtostrf(chipId, 10, 0, msgBuffer);
  strcpy(deviceId,devicePrefix); 
  strcat(deviceId,espChipId); 
  wifiManager.startConfigPortal(deviceId, devicePass);

  Serial.println("Connecté (on demand)");
  
  if (shouldSaveConfig) {
    strcpy(mqtt_server, custom_mqtt_server.getValue());
    strcpy(mqtt_port, custom_mqtt_port.getValue());
    strcpy(mqtt_client, custom_mqtt_client.getValue());
    strcpy(mqtt_user, custom_mqtt_user.getValue());
    strcpy(mqtt_password, custom_mqtt_password.getValue());
    Serial.println("saving config");
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
    MY_MQTT_SUBSCRIBE_TOPIC_PREFIX = mqtt_topic_in;
    MY_MQTT_PUBLISH_TOPIC_PREFIX = mqtt_topic_out;
    Serial.println(mqttTopicIn);
    File configFile = SPIFFS.open("/config.json", "w");
    if (!configFile) {
      Serial.println("failed to open config file for writing");
    }
    json.printTo(Serial);
    json.printTo(configFile);
    configFile.close();
  }
  Serial.print("IP locale : ");
  Serial.println(WiFi.localIP());
  Serial.print("MQTT config : ");
  Serial.print(MY_CONTROLLER_URL_ADDRESS);
  Serial.print(":");
  Serial.print(MY_PORT);
  Serial.print(" | ");
  Serial.print(MY_MQTT_CLIENT_ID);
  Serial.print(" | ");
  Serial.println(MY_MQTT_USER);
  Serial.print(MY_MQTT_PUBLISH_TOPIC_PREFIX);
  Serial.print(" | ");
  Serial.println(MY_MQTT_SUBSCRIBE_TOPIC_PREFIX);
  ticker.detach();
  //set_pins();
  setup();
}
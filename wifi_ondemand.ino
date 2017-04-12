void wifimanager_ondemand() {
  set_pins();
  //ticker.detach();
  //ticker.attach(0.5, tick);

  WiFiManager wifiManager;
  wifiManager.setSaveConfigCallback(saveConfigCallback);
  //wifiManager.setAPCallback(configModeCallback);
  //wifiManager.setSTAStaticIPConfig(IPAddress(192, 168, 1, 57), IPAddress(192, 168, 1, 1), IPAddress(255, 255, 255, 0)); /// Rajout d'n DNS local?
  wifiManager.addParameter(&custom_mqtt_server);
  wifiManager.addParameter(&custom_mqtt_port);
  wifiManager.addParameter(&custom_mqtt_client);
  wifiManager.addParameter(&custom_mqtt_user);
  wifiManager.addParameter(&custom_mqtt_password);
  //wifiManager.setAPConfig(IPAddress(192,168,1,5), IPAddress(192,168,1,1), IPAddress(255,255,255,0));
  wifiManager.setMinimumSignalQuality();
//  wifiManager.setTimeout(300);
  wifiManager.startConfigPortal(connect_ssid, ap_pass);
  
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
  set_pins();
  //ticker.detach();
}


/**********************************
 * Sketch configuration
 */

#define SKETCH_NAME "GatewayMQTT_RF69"
#define SKETCH_VERSION "1.0"


/**********************************
 * MySensors gateway configuration
 */
 
#define MY_DEBUG
#define MY_BAUD_RATE 115200

#define MY_GATEWAY_MQTT_CLIENT
#define MY_GATEWAY_ESP8266
//#define MY_GATEWAY_MAX_CLIENTS 15

#define MY_ESP8266_SSID "" // connection handled by wifimanager
#define MY_ESP8266_PASSWORD ""

#define MY_MQTT_PUBLISH_TOPIC_PREFIX mqttTopicOut
#define MY_MQTT_SUBSCRIBE_TOPIC_PREFIX mqttTopicIn
#define MY_MQTT_CLIENT_ID mqttClient // deviceID
// Enable these if your MQTT broker requires username/password
#define MY_MQTT_USER mqttUser 
#define MY_MQTT_PASSWORD mqttPassword 
//#define MY_CONTROLLER_IP_ADDRESS 1,1,1,1
#define MY_CONTROLLER_URL_ADDRESS mqttServer 
#define MY_PORT mqttPort

//#define MY_INCLUSION_MODE_FEATURE
//#define MY_INCLUSION_BUTTON_FEATURE
//#define MY_INCLUSION_MODE_DURATION 30
#define MY_INCLUSION_MODE_BUTTON_PIN  D3
//#define MY_INCLUSION_BUTTON_PRESSED LOW

#define STATE_LED D4


/**********************************
 * MySensors RF configuration
 */
 
// NRF24 radio settings
#define MY_RADIO_NRF24
//#define MY_RF24_ENABLE_ENCRYPTION
#define MY_RF24_PA_LEVEL RF24_PA_HIGH
#define MY_RF24_CHANNEL  108
//#define MY_RF24_DATARATE RF24_250KBPS
#define MY_RF24_DATARATE RF24_1MBPS
//#define MY_DEBUG_VERBOSE_RF24

// RFM69 radio settings
//#define MY_RADIO_RFM69
//#define MY_RFM69_FREQUENCY RFM69_433MHZ
//#define MY_IS_RFM69HW
//#define MY_DEBUG_VERBOSE_RFM69
//#define MY_RFM69_NEW_DRIVER
//#define MY_RFM69_ENABLE_ENCRYPTION
//#define MY_RFM69_NETWORKID 100
// for emc2cube shield
//#define MY_RF69_IRQ_PIN D2 
//#define MY_RF69_IRQ_NUM MY_RF69_IRQ_PIN
//#define MY_RF69_SPI_CS D8
//
//#define MY_RFM69_ATC_MODE_DISABLED
//#define MY_RFM69_TX_POWER_DBM 20

//#define MY_SENSOR_NETWORK

#define MY_TRANSPORT_WAIT_READY_MS (15*1000ul)
//#define MY_TRANSPORT_TIMEOUT_EXT_FAILURE_STATE (15*60*1000ul)
//#define MY_TRANSPORT_SANITY_CHECK
// already set as default
//#define MY_TRANSPORT_SANITY_CHECK_INTERVAL_MS (15*60*1000ul)
//#define MY_TRANSPORT_STATE_RETRIES 2


/**********************************
 * Global configuration
 */

char devicePass[30]="motdepasse", deviceId[20], devicePrefix[10] = "Gateway";
char mqtt_client[25], mqtt_user[15], mqtt_password[20], mqtt_server[20], mqtt_port[6], http_server[40], http_port[6]; 
char mqtt_topic_out[70], mqtt_topic_in[80], out[10]= "-out", in[20]= "-in/+/+/+/+/+" ;
int mqttPort, httpPort = 443, port = 8080;
const char* mqttServer;
const char* mqttClient;
const char* mqttUser;
const char* mqttPassword;
const char* mqttTopicOut;
const char* mqttTopicIn;
const char* httpServer;

//const char* Host = "app.getlarge.eu";
//const char* host = "http://192.168.1.46";
//const char* url = "/Gateway_HTTP_Update_test.ino.bin" ;
const char* otaUrl = "https://app.getlarge.eu/firmware/GatewayMQTT.ino.bin";
const char* currentVersion = "4712";
const char* httpsFingerprint = "1D AE 00 E7 68 70 87 09 A6 1D 27 76 F5 85 C0 F3 AB F2 60 9F"; 

bool resetConfig = false, wifiResetConfig = false; // set to true to reset FS and/or Wifimanager, don't forget to set this to false after
bool shouldSaveConfig = false, executeOnce = false, otaSignalReceived = false, manualConfig = false;
unsigned long configTimeout = 180, lastUpdate = 0, lastYield = 0, lastWifiReconnectAttempt = 0;
int configCount = 0, wifiFailCount = 0, mqttFailCount = 0, configMode = 0, _otaSignal = 0;

static const int fileSpaceOffset = 700000;
const String otaFile = "ota.txt";

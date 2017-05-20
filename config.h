
/**********************************
 * Sketch configuration
 */

#define SKETCH_NAME "Gateway_MQTT"
#define SKETCH_VERSION "0.7"

/**********************************
 * MySensors configuration
 */
 
// Enable debug prints to serial monitor
#define MY_DEBUG
// Use a bit lower baudrate for serial prints on ESP8266 than default in MyConfig.h
#define MY_BAUD_RATE 115200

#define MY_GATEWAY_MQTT_CLIENT
#define MY_GATEWAY_ESP8266
//#define MY_GATEWAY_MAX_CLIENTS 15

// Set WIFI SSID and password
#define MY_ESP8266_SSID ""
#define MY_ESP8266_PASSWORD ""
// Set the hostname for the WiFi Client. This is the hostname
// it will pass to the DHCP server if not static.
//#define MY_ESP8266_HOSTNAME "gateway_mqtt"

// Set this node's subscribe and publish topic prefix
#define MY_MQTT_PUBLISH_TOPIC_PREFIX mqttTopicOut
#define MY_MQTT_SUBSCRIBE_TOPIC_PREFIX mqttTopicIn

#define MY_MQTT_CLIENT_ID mqttClient // deviceID
// Enable these if your MQTT broker requires username/password
#define MY_MQTT_USER mqttUser // identifiant
#define MY_MQTT_PASSWORD mqttPassword // login
// MQTT broker ip address.
//#define MY_CONTROLLER_IP_ADDRESS 164, 132, 103, 148
#define MY_CONTROLLER_URL_ADDRESS mqttServer // adresse du serveur MQTT
// The MQTT broker port to open
#define MY_PORT mqttPort

// Enables and select radio type (if attached)
#define MY_RADIO_NRF24
//#define MY_RADIO_RFM69
#define MY_RF24_PA_LEVEL RF24_PA_MAX
#define MY_RF24_CHANNEL  108
//#define MY_RF24_DATARATE RF24_250KBPS
//#define MY_TRANSPORT_WAIT_READY_MS (3*1000ul)
//#define MY_TRANSPORT_TIMEOUT_EXT_FAILURE_STATE (15*60*1000ul)
//#define MY_TRANSPORT_SANITY_CHECK
// already set as default
//#define MY_TRANSPORT_SANITY_CHECK_INTERVAL_MS (15*60*1000ul)
//#define MY_TRANSPORT_STATE_RETRIES 2

// Enable inclusion mode
#define MY_INCLUSION_MODE_FEATURE
// Enable Inclusion mode button on gateway
//#define MY_INCLUSION_BUTTON_FEATURE
// Set inclusion mode duration (in seconds)
#define MY_INCLUSION_MODE_DURATION 30
// Digital pin used for inclusion mode button
#define MY_INCLUSION_MODE_BUTTON_PIN  D3
#define MY_INCLUSION_BUTTON_PRESSED LOW

// Set blinking period
//#define MY_DEFAULT_LED_BLINK_PERIOD 300
//
//// Flash leds on rx/tx/err
//#define MY_DEFAULT_ERR_LED_PIN D4  // Error led pin
//#define MY_DEFAULT_RX_LED_PIN  D4  // Receive led pin
//#define MY_DEFAULT_TX_LED_PIN  D4  // the PCB, on board LED

bool resetConfig = false; // set to true to reset FS and Wifimanager, don't forget to set this to false after

char devicePass[30]="motdepasse", deviceId[20], devicePrefix[10] = "gateway";
char mqtt_client[60], mqtt_user[20], mqtt_password[30], mqtt_server[40], mqtt_port[6];
char mqtt_topic_out[70], mqtt_topic_in[80], out[10]= "-out", in[20]= "-in/+/+/+/+/+" ;
int mqttPort;
const char* mqttServer;
const char* mqttClient;
const char* mqttUser;
const char* mqttPassword;
const char* mqttTopicOut;
const char* mqttTopicIn;
const char* otaUrl = "https://dr.courget.com/firmware/gateway_mqtt.ino.bin";
WiFiManagerParameter custom_mqtt_server("server", "mqtt server", mqtt_server, 40);
WiFiManagerParameter custom_mqtt_port("port", "mqtt port", mqtt_port, 6);
WiFiManagerParameter custom_mqtt_client("client", "mqtt client", mqtt_client, 60);
WiFiManagerParameter custom_mqtt_user("user", "mqtt user", mqtt_user, 20);
WiFiManagerParameter custom_mqtt_password("password", "mqtt password", mqtt_password, 30);

bool shouldSaveConfig = false, timeReceived = false, executeOnce = false, otaSignalReceived = false;
unsigned long lastMqttReconnectAttempt = 0, lastWifiReconnectAttempt = 0;
unsigned long lastUpdate=0, lastRequest=0, Timestamp=0;
int wifiCount = 0, mqttCount = 0, interval = 5000;

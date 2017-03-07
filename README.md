# BIODIAG - GATEWAY

## Requirements

Arduino IDE - download the latest from arduino

- https://www.arduino.cc/en/Main/Software

Packages for ESP8266 development on Arduino IDE

- http://arduino.esp8266.com/stable/package_esp8266com_index.json

following libraries are required :

- FS
- WifiManager
- MySensors
- Bounce2
- ArduinoJson
- Ticker

## Installation

```
git clone git@framagit.org:getlarge/gateway_mqtt.git
```

Then in `config.h.sample` file you may edit the following :


- Name your device for your wifi router ( DHCP )
```
#define MY_ESP8266_HOSTNAME "your_device_name"
```

- Protect the acces point
```
char ap_pass[30]="yourpassword",
```

## Usage

Open any .ino file of the folder with Arduino IDE
Edit your preferences
Uncomment FS.Format the first time you upload
Comment out FS.format
Upload the code on your ESP8266 board

Topic structure: MY_MQTT_PUBLISH_TOPIC_PREFIX/NODE-ID/SENSOR-ID/CMD-TYPE/ACK-FLAG/SUB-TYPE

## Dev

Go to the dev branch for the latest and unstable development

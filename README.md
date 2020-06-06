# nanoAkka for ESP8266 based on ESP8266_RTOS_SDK
Features :
- Reactive programming model
- OTA via mqtt at blazing speed : 1MB binary message via mosquitto_pub, push takes 1 sec, flash reboot via OTA about 20 sec. 
- Uses ideas from Actors, Streams
- Properties can be saved to NVS, changed via MQTT 
# BASE version
Features 
- publishes Wifi : IP, MAC, RSSI,ssid 
- publishes System : upTime, heapSize , build date, Last-will-testament alive every sec
# TREESHAKER 
To scare away birds in my cherry tree, the device activates a motor in the tree.
Features 
- activates a Triac or Relay during a fixed time
- Pulses are centrally send from a program using MQTT
- Relay on via : *dst/<host>/shaker/shake* = true
- Relay onTme via : *dst/<host>/shaker/shakeTime* = 3000 => 3 seconds 

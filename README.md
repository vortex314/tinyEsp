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
# OTA update through MQTT
Features 
- did I mention it is suprising fast ?
- no cloud server needed, just use existing MQTT connection
- no specific tools needed just use mosquitto_sub
- command ```
mosquitto_pub -h limero.ddns.net -t dst/$HOST/ota/data -q 1  -f build/tinyEsp.ota.bin
```
- Data arrives guaranteed through QOS1 in MQTT client of ESP8266
```
 case MQTT_EVENT_DATA:
    {
        //		INFO("MQTT_EVENT_DATA");
        static std::string data;
        if (event->current_data_offset == 0)
        {
            me._lastTopic = std::string(event->topic, event->topic_len);
            me._lastTopic = me._lastTopic.substr(me._hostPrefix.length());
        }
        bool isOtaData = me._lastTopic.find("ota/data") != std::string::npos;
        INFO(" MQTT_EVENT_DATA %s offset:%d length:%d total:%d ", me._lastTopic.c_str(), event->current_data_offset, event->data_len, event->total_data_len);
        if (isOtaData)
        {
            if (event->current_data_offset == 0)
            {
                me.mqttOta.init();
                me.mqttOta.initUpgrade();
            }
            me.mqttOta.writeUpgrade((uint8_t *)(event->data), event->data_len);
            if (event->current_data_offset + event->data_len == event->total_data_len)
            {
                me.mqttOta.endUpgrade();
                me.mqttOta.execUpgrade();
            }
        }
        else { /* threat all other Mqtt messages */ }
        ```
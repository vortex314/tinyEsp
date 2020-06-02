/* Hello World Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_spi_flash.h"

#include "NanoAkka.h"
template <class T>
class BiFlow : public Flow<T, T>
{
    T _t[2];
    int _idx = 0;

public:
    BiFlow() {}
    BiFlow(T t) { _t[0] = std::move(t); }
    void request() { this->emit(_t[_idx & 1]); }

    void on(const T &in)
    {
        _t[(_idx + 1) & 1] = std::move(in);
        _idx++;
    }
};

template <class T>
class RequestFlow : public Flow<T, T>
{
    Source<T> &_source;

public:
    RequestFlow(Source<T> &source) : _source(source) {}
    void request() { _source.request(); }
    void on(const T &t) { this->emit(t); }
};
//____________________________________________________________________________________
//
class Poller : public Actor
{
    TimerSource _pollInterval;
    std::vector<Requestable *> _requestables;
    uint32_t _idx = 0;

public:
    ValueFlow<bool> connected;
    ValueFlow<uint32_t> interval = 500;
    Poller(Thread &t) : Actor(t), _pollInterval(t, 1, 500, true)
    {
        _pollInterval >> [&](const TimerMsg tm) {
            if (_requestables.size() && connected())
                _requestables[_idx++ % _requestables.size()]->request();
        };
        interval >> [&](const uint32_t iv) { _pollInterval.interval(iv); };
    };
    /*
    template <class T>Poller
    Source<T>& cache(Source<T>& input){
      BiFlow<T>* bf = new BiFlow<T>();
      input >>
      _requestables.push_back(bf);

    }*/

    template <class T>
    Source<T> &poll(Source<T> &source)
    {
        RequestFlow<T> *rf = new RequestFlow<T>(source);
        source >> rf;
        _requestables.push_back(rf);
        return *rf;
    }

    template <class T>
    Flow<T, T> &cache()
    {
        BiFlow<T> *vf = new BiFlow<T>();
        _requestables.push_back(vf);
        return *vf;
    }

    Poller &operator()(Requestable &rq)
    {
        _requestables.push_back(&rq);
        return *this;
    }
};
#include <LedBlinker.h>
#include <Wifi.h>
#include <MqttWifi.h>
#include <MqttOta.h>
Log logger(256);
//------------------------------------------------------- THREADS
Thread mainThread("main");
Thread mqttThread("mqtt");
TimerSource ts(mainThread, 1, 1000, true);

//------------------------------------------------------ ACTORS
Wifi wifi(mqttThread);
MqttWifi mqtt(mqttThread);
Poller poller(mqttThread);
MqttOta mqttOta(mainThread);
LedBlinker ledBlue(mainThread, 2, 1000);
LedBlinker ledRed(mainThread, 16, UINT32_MAX);

ValueSource<std::string> systemBuild("NOT SET");
ValueSource<std::string> systemHostname("NOT SET");
ValueSource<bool> systemAlive = true;
LambdaSource<uint32_t> systemHeap([]() { return Sys::getFreeHeap(); });
LambdaSource<uint64_t> systemUptime([]() { return Sys::millis(); });
#include <sys/time.h>

extern "C" void app_main()
{
    Sys::init();

#ifdef HOSTNAME
    Sys::hostname(S(HOSTNAME));
#else
    std::string hn;
    union {
        uint8_t macBytes[6];
        uint64_t macInt;
    };
    macInt = 0L;
    if (esp_read_mac(macBytes, ESP_MAC_WIFI_STA) != ESP_OK)
        WARN(" esp_base_mac_addr_get() failed.");
    string_format(hn, "ESP82-%d", macInt & 0xFFFF);
    Sys::hostname(hn.c_str());
#endif
    systemHostname = Sys::hostname();
    systemBuild = __DATE__ " " __TIME__;
#ifdef MQTT_SERIAL
    mqtt.init();
#else
    wifi.init();
    mqtt.init();
    wifi.connected >> mqtt.wifiConnected;
    mqtt.connected >> poller.connected;
    //-----------------------------------------------------------------  WIFI
    // props
    poller.poll(wifi.macAddress) >> mqtt.toTopic<std::string>("wifi/mac");
    poller.poll(wifi.ipAddress) >> mqtt.toTopic<std::string>("wifi/ip");
    poller.poll(wifi.ssid) >> mqtt.toTopic<std::string>("wifi/ssid");
    poller.poll(wifi.rssi) >> mqtt.toTopic<int>("wifi/rssi");
#endif

    systemUptime >> mqtt.toTopic<uint64_t>("system/upTime");
    systemHeap >> mqtt.toTopic<uint32_t>("system/heap");
    systemHostname >> mqtt.toTopic<std::string>("system/hostname");
    systemBuild >> mqtt.toTopic<std::string>("system/build");
    systemAlive >> mqtt.toTopic<bool>("system/alive");
    poller(systemUptime)(systemHeap)(systemHostname)(systemBuild)(systemAlive);
    //------------------------------------------------------------ OTA
    mqttOta.init();
    mqtt.fromTopic<std::string>("ota/cmd") >> mqttOta.command;
    mqtt.fromTopic<std::string>("ota/data") >> mqttOta.data;

    ledRed.blinkSlow.on(true);
    INFO(" ESP8266_RTOS_SDK " __DATE__ " " __TIME__ "V: %x", esp_get_idf_version());
    ts >> [](const TimerMsg &tm) {
        INFO(" heap : %u min : %u ",
             esp_get_free_heap_size(),
             esp_get_minimum_free_heap_size());
        ledRed.pulse.on(true);
    };
    ledBlue.init();
    ledRed.init();
    mainThread.start(); // wifi init fails if this doesn't end
    mqttThread.start();
}

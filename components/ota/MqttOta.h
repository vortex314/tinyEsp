#include <NanoAkka.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <esp_https_ota.h>
#include <esp_ota_ops.h>
#include "sdkconfig.h"
#include <Bytes.h>
#include <Base64.h>
#define OTA_BUF_SIZE 300

class MqttOta : public Actor
{
    esp_partition_t* update_partition = 0;
    esp_ota_handle_t update_handle = 0;
    Bytes _data;
    uint32_t _lengthWritten=0;
    typedef enum {READY,INIT,WRITE,END} State;
    State _state=READY;
public:
    Sink<std::string, 3> command;
    Sink<std::string, 3> data;
    MqttOta(Thread &thread);
    void init();
    int initUpgrade();
    int writeUpgrade(Bytes &data);
    int endUpgrade();
    void checkUpgrade();
    int execUpgrade();
};
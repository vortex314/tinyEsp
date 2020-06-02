#include <NanoAkka.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <esp_https_ota.h>
#include <esp_ota_ops.h>
#include "sdkconfig.h"
#define OTA_BUF_SIZE CONFIG_OTA_BUF_SIZE

class HttpOta : public Actor
{
    esp_partition_t update_partition;
    bool _busy;

public:
    ValueFlow<std::string> message;
    Sink<std::string,2> otaUrl;
    HttpOta(Thread &thread);
    void init();
    void http_cleanup(esp_http_client_handle_t client);
    esp_err_t esp_https_ota(const esp_http_client_config_t *config);
    void do_firmware_upgrade(std::string url);
};
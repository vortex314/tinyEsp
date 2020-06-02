#include <HttpOta.h>

HttpOta::HttpOta(Thread &thread) : Actor(thread) {}

void HttpOta::init()
{
    message.pass(true);
    otaUrl.async(thread(), [&](const std::string &str) { do_firmware_upgrade(str); });
}

void HttpOta::http_cleanup(esp_http_client_handle_t client)
{
    esp_http_client_close(client);
    esp_http_client_cleanup(client);
}

#define M_INFO(xxx) INFO(xxx)
#define M_ERROR(xxx) ERROR(xxx)

void HttpOta::do_firmware_upgrade(std::string url)
{

    esp_http_client_config_t config;
    BZERO(config);
    config.url = "http://192.168.0.197:8080/build/tinyEsp.app1.bin";
    esp_err_t ret = esp_https_ota(&config);
    if (ret == ESP_OK)
    {
        message = "restarting ";
        M_INFO(" restarting ");
        esp_restart();
    }
    message = "upgrade failed";
    M_ERROR(" upgrade failed ");
}

esp_err_t HttpOta::esp_https_ota(const esp_http_client_config_t *config)
{
    INFO(" starting ota ");
    std::string buffer;
    if (!config)
    {
        M_ERROR("esp_http_client config not found");
        return ESP_ERR_INVALID_ARG;
    }

    esp_http_client_handle_t client = esp_http_client_init(config);
    if (client == NULL)
    {
        M_ERROR("Failed to initialise HTTP connection");
        return ESP_FAIL;
    }

    esp_err_t err = esp_http_client_open(client, 0);
    if (err != ESP_OK)
    {
        esp_http_client_cleanup(client);
        ERROR("Failed to open HTTP connection: %d", err);
        return err;
    }
    esp_http_client_fetch_headers(client);

    esp_ota_handle_t update_handle = 0;
    const esp_partition_t *update_partition = NULL;
    M_INFO("Starting OTA...");
    message = "Starting OTA...";

    update_partition = esp_ota_get_next_update_partition(NULL);
    if (update_partition == NULL)
    {
        M_ERROR("Passive OTA partition not found");
        http_cleanup(client);
        return ESP_FAIL;
    }
    INFO("Writing to partition subtype %d at offset 0x%x",
         update_partition->subtype, update_partition->address);

    err = esp_ota_begin(update_partition, OTA_SIZE_UNKNOWN, &update_handle);
    if (err != ESP_OK)
    {
        ERROR("esp_ota_begin failed, error=%d", err);
        http_cleanup(client);
        return err;
    }
    M_INFO("esp_ota_begin succeeded");
    M_INFO("Please Wait. This may take time");

    esp_err_t ota_write_err = ESP_OK;
    char *upgrade_data_buf = (char *)malloc(OTA_BUF_SIZE);
    if (!upgrade_data_buf)
    {
        ERROR("Couldn't allocate memory to upgrade data buffer");
        return ESP_ERR_NO_MEM;
    }
    int binary_file_len = 0;
    while (1)
    {
        int data_read = esp_http_client_read(client, upgrade_data_buf, OTA_BUF_SIZE);
        if (data_read == 0)
        {
            M_INFO("Connection closed,all data received");
            break;
        }
        if (data_read < 0)
        {
            M_ERROR("Error: SSL data read error");
            break;
        }
        if (data_read > 0)
        {
            ota_write_err = esp_ota_write(update_handle, (const void *)upgrade_data_buf, data_read);
            if (ota_write_err != ESP_OK)
            {
                break;
            }
            binary_file_len += data_read;
            INFO("Written image length %d", binary_file_len);
        }
    }
    free(upgrade_data_buf);
    http_cleanup(client);
    INFO("Total binary data length writen: %d", binary_file_len);

    esp_err_t ota_end_err = esp_ota_end(update_handle);
    if (ota_write_err != ESP_OK)
    {
        ERROR("Error: esp_ota_write failed! err=0x%d", err);
        return ota_write_err;
    }
    else if (ota_end_err != ESP_OK)
    {
        ERROR("Error: esp_ota_end failed! err=0x%d. Image is invalid", ota_end_err);
        return ota_end_err;
    }

    err = esp_ota_set_boot_partition(update_partition);
    if (err != ESP_OK)
    {
        ERROR("esp_ota_set_boot_partition failed! err=0x%d", err);
        return err;
    }
    M_INFO("esp_ota_set_boot_partition succeeded");

    return ESP_OK;
}
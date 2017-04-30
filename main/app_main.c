#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>

#include "soc/rtc_cntl_reg.h"

#include "esp_wifi.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "esp_event_loop.h"


#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"

#include "lwip/sockets.h"
#include "lwip/dns.h"
#include "lwip/netdb.h"

#include "config.h"
#include "debug.h"

#include "mqtt.h"
#include "os.h"
#include "onenet.h"

static bool onenet_initialised = false;
static TaskHandle_t xOneNetTask = NULL;

void onenet_task(void *param)
{
   mqtt_client* client = (mqtt_client *)param;
   uint32_t val;

   while(1) {
        /**
         * Replace this *val* with real captured value by your sensor,
         * Here, we just use a rand number which ranges form 15 to 35. 
         */
        val = os_random() % 20 + 15;

        char buf[128];
        memset(buf, 0, sizeof(buf));
        sprintf(&buf[3], "{\"%s\":%d}", ONENET_DATA_STREAM, val);
        uint16_t len = strlen(&buf[3]);
        buf[0] = data_type_simple_json_without_time;
        buf[1] = len >> 8;
        buf[2] = len & 0xFF;
        mqtt_publish(client, "$dp", buf, len + 3, 0, 0);
    
        vTaskDelay((unsigned long long)ONENET_PUB_INTERVAL* 1000 / portTICK_RATE_MS);
   }
}


void onenet_start(mqtt_client *client)
{
    if(!onenet_initialised) {
        xTaskCreate(&onenet_task, "onenet_task", 2048, client, CONFIG_MQTT_PRIORITY + 1, &xOneNetTask);
        onenet_initialised = true;
    }
}

void onenet_stop(mqtt_client *client)
{
    if(onenet_initialised) {
        if(xOneNetTask) {
            vTaskDelete(xOneNetTask);
        }
        onenet_initialised = false;
    }
}

void connected_cb(void *self, void *params)
{
    mqtt_client *client = (mqtt_client *)self;
    //mqtt_subscribe(client, "/test", 0);
    //mqtt_publish(client, "/test", "howdy!", 6, 0, 0);
    onenet_start(client);
}
void disconnected_cb(void *self, void *params)
{
     mqtt_client *client = (mqtt_client *)self;
     onenet_stop(client);
}
void reconnect_cb(void *self, void *params)
{
    mqtt_client *client = (mqtt_client *)self;
    onenet_start(client);
}
void subscribe_cb(void *self, void *params)
{
    INFO("[APP] Subscribe ok, test publish msg\n");
    mqtt_client *client = (mqtt_client *)self;
    //mqtt_publish(client, "/test", "abcde", 5, 0, 0);
}

void publish_cb(void *self, void *params)
{

}
void data_cb(void *self, void *params)
{
    (void)self;
    mqtt_event_data_t *event_data = (mqtt_event_data_t *)params;

    if (event_data->data_offset == 0) {

        char *topic = malloc(event_data->topic_length + 1);
        memcpy(topic, event_data->topic, event_data->topic_length);
        topic[event_data->topic_length] = 0;
        INFO("[APP] Publish topic: %s\n", topic);
        free(topic);
    }

    // char *data = malloc(event_data->data_length + 1);
    // memcpy(data, event_data->data, event_data->data_length);
    // data[event_data->data_length] = 0;
    INFO("[APP] Publish data[%d/%d bytes]\n",
         event_data->data_length + event_data->data_offset,
         event_data->data_total_length);
         // data);

    // free(data);

}

mqtt_settings settings = {
    .host = ONENET_HOST,
    .port = ONENET_PORT,
    .client_id = ONENET_DEVICE_ID,
    .username = ONENET_PROJECT_ID,
    .password = ONENET_AUTH_INFO,
    .clean_session = 0,
    .keepalive = 120,
    .lwt_topic = "/lwt",
    .lwt_msg = "offline",
    .lwt_qos = 0,
    .lwt_retain = 0,
    .connected_cb = connected_cb,
    .disconnected_cb = disconnected_cb,
    .reconnect_cb = reconnect_cb,
    .subscribe_cb = subscribe_cb,
    .publish_cb = publish_cb,
    .data_cb = data_cb
};

static esp_err_t wifi_event_handler(void *ctx, system_event_t *event)
{

    switch(event->event_id) {
    case SYSTEM_EVENT_STA_START:
        ESP_ERROR_CHECK(esp_wifi_connect());
        break;

    case SYSTEM_EVENT_STA_GOT_IP:

        mqtt_start(&settings);
        // Notice that, all callback will called in mqtt_task
        // All function publish, subscribe
        break;
    case SYSTEM_EVENT_STA_DISCONNECTED:
        /* This is a workaround as ESP32 WiFi libs don't currently
           auto-reassociate. */
        
        mqtt_stop();
        ESP_ERROR_CHECK(esp_wifi_connect());
        break;
    default:
        break;
    }
    return ESP_OK;


}

void wifi_conn_init(void)
{
    INFO("[APP] Start, connect to Wifi network: %s ..\n", WIFI_SSID);

    tcpip_adapter_init();

    ESP_ERROR_CHECK( esp_event_loop_init(wifi_event_handler, NULL) );

    wifi_init_config_t icfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK( esp_wifi_init(&icfg) );
    ESP_ERROR_CHECK( esp_wifi_set_storage(WIFI_STORAGE_RAM) );

    wifi_config_t wifi_config = {
        .sta = {
            .ssid = WIFI_SSID,
            .password = WIFI_PASS
        },
    };

    ESP_ERROR_CHECK( esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK( esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK( esp_wifi_start());
}

void app_main()
{
    INFO("[APP] Startup..\n");
    INFO("[APP] Free memory: %d bytes\n", esp_get_free_heap_size());
    INFO("[APP] Build time: %s\n", BUID_TIME);

#ifdef CPU_FREQ_160MHZ
    INFO("[APP] Setup CPU run as 160MHz\n");
    SET_PERI_REG_BITS(RTC_CLK_CONF, RTC_CNTL_SOC_CLK_SEL, 0x1, RTC_CNTL_SOC_CLK_SEL_S);
    WRITE_PERI_REG(CPU_PER_CONF_REG, 0x01);
    INFO("[APP] Setup CPU run as 160MHz - Done\n");
#endif
 
    nvs_flash_init();
    wifi_conn_init();
}

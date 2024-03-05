#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include "esp_wifi.h"
#include "esp_mac.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "esp_event.h"
#include "esp_netif.h"

#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"

#include "lwip/sockets.h"
#include "lwip/dns.h"
#include "lwip/netdb.h"

#include "esp_log.h"
#include "mqtt_client.h"
#include "driver/spi_master.h"

#include "light2812.h"
#include "lighteffect2812.h"

static const char *TAG = "MQTT_EXAMPLE";
// static uint8_t color[LIGHT_NUM][3] = {0};


static void log_error_if_nonzero(const char *message, int error_code)
{
    if (error_code != 0) {
        ESP_LOGE(TAG, "Last error %s: 0x%x", message, error_code);
    }
}

/*
 * @brief Event handler registered to receive MQTT events
 *
 *  This function is called by the MQTT client event loop.
 *
 * @param handler_args user data registered to the event.
 * @param base Event base for the handler(always MQTT Base in this example).
 * @param event_id The id for the received event.
 * @param event_data The data for the event, esp_mqtt_event_handle_t.
 */
static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
    ESP_LOGD(TAG, "Event dispatched from event loop base=%s, event_id=%" PRIi32 "", base, event_id);
    esp_mqtt_event_handle_t event = event_data;
    esp_mqtt_client_handle_t client = event->client;
    int msg_id;
    switch ((esp_mqtt_event_id_t)event_id) {
    case MQTT_EVENT_CONNECTED:
        ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");
        msg_id = esp_mqtt_client_publish(client, "/topic/qos1", "esp32-s3 conncet", 0, 1, 0);
        ESP_LOGI(TAG, "sent publish successful, msg_id=%d", msg_id);

        msg_id = esp_mqtt_client_subscribe(client, "ws2812b/switch", 0);
        ESP_LOGI(TAG, "sent subscribe successful, msg_id=%d", msg_id);

        msg_id = esp_mqtt_client_subscribe(client, "ws2812b/effect", 0);
        ESP_LOGI(TAG, "sent subscribe successful, msg_id=%d", msg_id);

        // msg_id = esp_mqtt_client_subscribe(client, "/topic/qos1", 1);
        // ESP_LOGI(TAG, "sent subscribe successful, msg_id=%d", msg_id);

        // msg_id = esp_mqtt_client_unsubscribe(client, "/topic/qos1");
        // ESP_LOGI(TAG, "sent unsubscribe successful, msg_id=%d", msg_id);
        break;
    case MQTT_EVENT_DISCONNECTED:
        ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");
        break;

    case MQTT_EVENT_SUBSCRIBED:
        ESP_LOGI(TAG, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
        // msg_id = esp_mqtt_client_publish(client, "/topic/qos0", "data", 0, 0, 0);
        // ESP_LOGI(TAG, "sent publish successful, msg_id=%d", msg_id);
        break;
    case MQTT_EVENT_UNSUBSCRIBED:
        ESP_LOGI(TAG, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
        break;
    case MQTT_EVENT_PUBLISHED:
        ESP_LOGI(TAG, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
        break;
    case MQTT_EVENT_DATA:
    {
        ESP_LOGI(TAG, "MQTT_EVENT_DATA");
        printf("TOPIC=%.*s\r\n", event->topic_len, event->topic);
        printf("DATA=%.*s\r\n", event->data_len, event->data);
        static lightEffect_t preState = LIGHTEFFECT_IDLE;
        if(!strncmp(event->topic, "ws2812b/switch", event->topic_len)) {
            printf("top len=%d\r\n", event->topic_len);
            printf("data len=%d\r\n", event->data_len);
            if(event->data[0] == '1') {
                setLightEffect(preState);
            } else if(event->data[0] == '0') {
                setLightEffect(LIGHTEFFECT_IDLE);
            }
        } else if(!strncmp(event->topic, "ws2812b/effect", event->topic_len)) {
            switch (event->data[0])
            {
            case '1':
                setLightEffect(LIGHTEFFECT_RAINBOW_CYCLE);
                preState = LIGHTEFFECT_RAINBOW_CYCLE;
                break;
            case '2':
                setLightEffect(LIGHTEFFECT_RAINBOW_BREATH);
                preState = LIGHTEFFECT_RAINBOW_BREATH;
                break;
            case '3':
                setLightEffect(LIGHTEFFECT_BULLET);
                preState = LIGHTEFFECT_BULLET;
                break;
            default:
                break;
            }
        // if(strcmp(event->data, "1")) {
        //     setLightEffect(LIGHTEFFECT_RAINBOW_BREATH);
        // } else {
        //     setLightEffect(LIGHTEFFECT_RAINBOW_CYCLE);
        // }
        }
    }
        break;
    case MQTT_EVENT_ERROR:
        ESP_LOGI(TAG, "MQTT_EVENT_ERROR");
        if (event->error_handle->error_type == MQTT_ERROR_TYPE_TCP_TRANSPORT) {
            log_error_if_nonzero("reported from esp-tls", event->error_handle->esp_tls_last_esp_err);
            log_error_if_nonzero("reported from tls stack", event->error_handle->esp_tls_stack_err);
            log_error_if_nonzero("captured as transport's socket errno",  event->error_handle->esp_transport_sock_errno);
            ESP_LOGI(TAG, "Last errno string (%s)", strerror(event->error_handle->esp_transport_sock_errno));

        }
        break;
    default:
        ESP_LOGI(TAG, "Other event id:%d", event->event_id);
        break;
    }
}

#define CONFIG_WIFI_SSID        "o_o"           //"o_o"    RabbitHouse
#define CONFIG_WIFI_PASSWORD    "12345679"

#define CONNECTED_BIT       BIT0
#define CONNECT_FAIL_BIT    BIT1
#define AUTH_FAIL_BIT       BIT2

#if 0
/*wifi 的中断回调函数，检测wifi的事件标志位*/
static esp_err_t wifi_event_handler(void *ctx, system_event_t *event)
{
    // switch (event->event_id) {
    //     case SYSTEM_EVENT_STA_START://开始执行station 
    //         esp_wifi_connect();//根据wifi配置，连接wifi
    //         break;
    //     case SYSTEM_EVENT_STA_GOT_IP://成功获取到ip，表示联网成功
    //         xEventGroupSetBits(wifi_event_group, CONNECTED_BIT);//设置事件标志位，程序继续运行

    //         break;
    //     case SYSTEM_EVENT_STA_DISCONNECTED://station 已经断开了，重新连接wifi
    //         esp_wifi_connect();
    //         xEventGroupClearBits(wifi_event_group, CONNECTED_BIT);
    //         break;
    //     default:
    //         break;
    // }
    return ESP_OK;
}
#endif

static int s_retry_num = 0;
/* FreeRTOS event group to signal when we are connected*/
static EventGroupHandle_t wifi_event_group;

static void wifi_event_handler(void* event_handler_arg, esp_event_base_t event_base, int32_t event_id, void* event_data)
{    
    /***********************
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        ESP_ERROR_CHECK(esp_supp_dpp_start_listen());
        ESP_LOGI(TAG, "Started listening for DPP Authentication");
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        if (s_retry_num < 5) {
            esp_wifi_connect();
            s_retry_num++;
            ESP_LOGI(TAG, "retry to connect to the AP");
        } else {
            xEventGroupSetBits(s_dpp_event_group, DPP_CONNECT_FAIL_BIT);
        }
        ESP_LOGI(TAG, "connect to the AP fail");
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t *event = (ip_event_got_ip_t *) event_data;
        ESP_LOGI(TAG, "got ip:" IPSTR, IP2STR(&event->ip_info.ip));
        s_retry_num = 0;
        xEventGroupSetBits(s_dpp_event_group, DPP_CONNECTED_BIT);
    }
                                    ****************************/
    #if 1
    ESP_LOGI(TAG,"event_base:%s, event_id: %ld\r\n", event_base, event_id);
    // wifi_event_ap_staconnected_t *wifi_event_data;
    if (event_base == WIFI_EVENT){
        switch (event_id)
        {
            case WIFI_EVENT_STA_START:                  //STA模式启动
                // ESP_ERROR_CHECK(esp_supp_dpp_start_listen());
                ESP_ERROR_CHECK(esp_wifi_connect());
                ESP_LOGI(TAG, "Started listening for DPP Authentication");
                break;
            case WIFI_EVENT_STA_STOP:                   //STA模式关闭
                /* code */
                break;
            case WIFI_EVENT_STA_CONNECTED:              //wifi连接
                // xEventGroupSetBits(wifi_event_group, CONNECT_FAIL_BIT);
                /* code */
                break;
            case WIFI_EVENT_STA_DISCONNECTED:           //STA模式断开连接
                if (s_retry_num < 5) {
                    esp_wifi_connect();
                    s_retry_num++;
                    ESP_LOGI(TAG, "retry to connect to the AP");
                } else {
                    xEventGroupSetBits(wifi_event_group, CONNECT_FAIL_BIT);
                }
                ESP_LOGI(TAG, "connect to the AP fail");
                break;
            case WIFI_EVENT_AP_START:                   //AP模式启动
                /* code */
                break;
            case WIFI_EVENT_AP_STOP:                    //AP模式关闭
                /* code */
                break;
            case WIFI_EVENT_AP_STACONNECTED:            //一台设备连接到esp32
                wifi_event_ap_staconnected_t *AP_STACONNECTED_EVENT_DATA = (wifi_event_ap_staconnected_t *)event_data;  //获取事件信息
                ESP_LOGI(TAG, "station:" MACSTR " join, AID=%d", MAC2STR(AP_STACONNECTED_EVENT_DATA->mac), AP_STACONNECTED_EVENT_DATA->aid);
                // (void)AP_STACONNECTED_EVENT_DATA;
                break;
            case WIFI_EVENT_AP_STADISCONNECTED:         //一台设备断开与esp32的连接
                wifi_event_ap_stadisconnected_t *AP_STADISCONNECTED_EVENT_DATA = (wifi_event_ap_stadisconnected_t *)event_data;  //获取事件信息
                ESP_LOGI(TAG, "station:" MACSTR "leave, AID=%d", MAC2STR(AP_STADISCONNECTED_EVENT_DATA->mac), AP_STADISCONNECTED_EVENT_DATA->aid);
                break;
            default:
                break;
        }
    }else if(event_base == IP_EVENT){
        switch (event_id)
        {
        case IP_EVENT_STA_GOT_IP:                       //esp32从路由器获取到ip
        {
            ip_event_got_ip_t *event = (ip_event_got_ip_t *) event_data;
            ESP_LOGI(TAG, "got ip:" IPSTR, IP2STR(&event->ip_info.ip));
            s_retry_num = 0;
            xEventGroupSetBits(wifi_event_group, CONNECTED_BIT);
        }
            break;
        case IP_EVENT_STA_LOST_IP:                      //esp32失去ip
            /* code */
            break;
        case IP_EVENT_AP_STAIPASSIGNED:                 //esp32给设备分配了ip
            /* code */
            break;
        default:
            break;
        }
    }
    #endif
}

static void wifiInit(void)
{

    #if 0
    tcpip_adapter_init();//tcpip 协议栈初始化，使用网络时必须调用此函数
    /*创建一个freeRTOS的事件标志组，用于当wifi没有连接时将程序停下，只有wifi连接成功了才能继续运行程序*/
    wifi_event_group = xEventGroupCreate();
    /*配置 wifi的回调函数，用于连接wifi*/
    /*
    * ESP_ERROR_CHECK检查函数返回值
    */
    ESP_ERROR_CHECK(esp_event_loop_init(wifi_event_handler, NULL));
    /*wifi配置*/
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));
    wifi_config_t wifi_config = {
        .sta = {
            .ssid = CONFIG_WIFI_SSID,
            .password = CONFIG_WIFI_PASSWORD,
        },
    };
    /*设置wifi 为sta模式*/
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config));
    ESP_LOGI(TAG, "start the WIFI SSID:[%s]", CONFIG_WIFI_SSID);
    /*开始运行wifi*/
    ESP_ERROR_CHECK(esp_wifi_start());
    ESP_LOGI(TAG, "Waiting for wifi");
    /*等待事件标志，成功获取到事件标志位后才继续执行，否则一直等在这里*/
    xEventGroupWaitBits(wifi_event_group, CONNECTED_BIT, false, true, portMAX_DELAY);
    #endif
     
    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();

    esp_event_handler_instance_t instance_any_id; 
    esp_event_handler_instance_t instance_got_ip;
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        &wifi_event_handler,
                                                        NULL,
                                                        &instance_any_id));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
                                                        IP_EVENT_STA_GOT_IP,
                                                        &wifi_event_handler,
                                                        NULL,
                                                        &instance_got_ip));
    // wifi初始化
    wifi_init_config_t wifiInitCfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&wifiInitCfg));
    // 设置wifi STA模式
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    // wifi配置
    wifi_config_t wifiCfg = {
        .sta = {
            .ssid = CONFIG_WIFI_SSID,
            .password = CONFIG_WIFI_PASSWORD,
            .threshold.authmode = WIFI_AUTH_WPA2_PSK,
            .pmf_cfg = {
                .capable = true,
                .required = false,
            },
        },
    };
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifiCfg));
    // 启动wifi
    ESP_ERROR_CHECK(esp_wifi_start());
    // 等待wifi连接
    wifi_event_group = xEventGroupCreate();
    EventBits_t bits = xEventGroupWaitBits(wifi_event_group,
                                           CONNECTED_BIT | CONNECT_FAIL_BIT | AUTH_FAIL_BIT,
                                           pdFALSE,
                                           pdFALSE,
                                           portMAX_DELAY);
    (void)bits;
}

static void mqtt_app_start(void)
{
    esp_mqtt_client_config_t mqtt_cfg = {
        .broker.address = {
            .uri = CONFIG_BROKER_URL, //"mqtt://192.168.3.140:5812",         // Madoka:849379776zxc?
            // .hostname = "192.168.3.140",
            // .port = 5812,
            // .transport = MQTT_TRANSPORT_OVER_TCP,
        },
        .credentials = {
            .username = CONFIG_USERNAME,    //"Madoka",
            .authentication.password = CONFIG_PASSWORD,    //"849379776zxc?",
            // .client_id = "mqttx_9c2dfd00",
        },
    };
#if CONFIG_BROKER_URL_FROM_STDIN
    char line[128];

    if (strcmp(mqtt_cfg.broker.address.uri, "FROM_STDIN") == 0) {
        int count = 0;
        printf("Please enter url of mqtt broker\n");
        while (count < 128) {
            int c = fgetc(stdin);
            if (c == '\n') {
                line[count] = '\0';
                break;
            } else if (c > 0 && c < 127) {
                line[count] = c;
                ++count;
            }
            vTaskDelay(10 / portTICK_PERIOD_MS);
        }
        mqtt_cfg.broker.address.uri = line;
        printf("Broker url: %s\n", line);
    } else {
        ESP_LOGE(TAG, "Configuration mismatch: wrong broker url");
        abort();
    }
#endif /* CONFIG_BROKER_URL_FROM_STDIN */

    esp_mqtt_client_handle_t client = esp_mqtt_client_init(&mqtt_cfg);
    /* The last argument may be used to pass data to the event handler, in this example mqtt_event_handler */
    esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_event_handler, NULL);
    esp_mqtt_client_start(client);
}

void app_main(void)
{

    ESP_LOGI(TAG, "[APP] Startup..");
    ESP_LOGI(TAG, "[APP] Free memory: %" PRIu32 " bytes", esp_get_free_heap_size());
    ESP_LOGI(TAG, "[APP] IDF version: %s", esp_get_idf_version());

    esp_log_level_set("*", ESP_LOG_INFO);
    esp_log_level_set("mqtt_client", ESP_LOG_VERBOSE);
    esp_log_level_set("MQTT_EXAMPLE", ESP_LOG_VERBOSE);
    esp_log_level_set("TRANSPORT_BASE", ESP_LOG_VERBOSE);
    esp_log_level_set("esp-tls", ESP_LOG_VERBOSE);
    esp_log_level_set("TRANSPORT", ESP_LOG_VERBOSE);
    esp_log_level_set("outbox", ESP_LOG_VERBOSE);

    lightInit();
    wifiInit();
    
    mqtt_app_start();

    lightEffectInit();
}

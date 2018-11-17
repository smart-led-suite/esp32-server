#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "driver/ledc.h"

#include "esp_system.h"
#include "esp_err.h"
#include "esp_wifi.h"
#include "esp_event_loop.h"
#include "esp_log.h"
#include "nvs_flash.h"

#include "lwip/err.h"
#include "lwip/sys.h"

#include "main.h"
#include "config.h"
#include "pass.h"

#include <stdint.h>
#include <stddef.h>

#include "freertos/semphr.h"
#include "freertos/queue.h"

#include "lwip/sockets.h"
#include "lwip/dns.h"
#include "lwip/netdb.h"

#include "esp_log.h"
#include "mqtt_client.h"

#include "mqtt.h"
//-------------------
//WIFI STUFF
/* FreeRTOS event group to signal when we are connected*/
static EventGroupHandle_t wifi_event_group;

/* The event group allows multiple bits for each event,
   but we only care about one event - are we connected
   to the AP with an IP? */

const char *TAG = "MQTTS_SAMPLE";
const static int CONNECTED_BIT = BIT0;

struct rgb rgb_1;



void update() {

    uint32_t red = (rgb_1.red * 32 / 100) * rgb_1.brightness;
    uint32_t green = (rgb_1.green * 32 / 100) * rgb_1.brightness;
    uint32_t blue = (rgb_1.blue * 32 / 100) * rgb_1.brightness;
    
    printf("new Value for rgb: %d %d %d", red, green, blue);
    
    setTargetBrightness(0, red, 600);
    setTargetBrightness(1, green, 600);
    setTargetBrightness(2, blue, 600);

    executeFade();
}







ledc_channel_config_t ledc_channel[LED_CH_USED] = {
        {
            .channel    = LED_1_R_CH,
            .duty       = 0,
            .gpio_num   = LED_1_R,
            .speed_mode = LED_HS_MODE,
            .timer_sel  = LED_HS_TIMER
        },
        {
            .channel    = LED_1_G_CH,
            .duty       = 0,
            .gpio_num   = LED_1_G,
            .speed_mode = LED_HS_MODE,
            .timer_sel  = LED_HS_TIMER
        },
        {
            .channel    = LED_1_B_CH,
            .duty       = 0,
            .gpio_num   = LED_1_B,
            .speed_mode = LED_HS_MODE,
            .timer_sel  = LED_HS_TIMER
        }
};






//------------------------------------------------------------------------------------------------------------------



void app_main()
{
    initialize();
    printf("Initializitaion complete!\n");


    rgb_1.brightness = 100;
    rgb_1.red = 255;
    rgb_1.green = 253;
    rgb_1.blue = 50;
    rgb_1.state = true;
    update();
}





bool initialize() {

    /*
     * Prepare and set configuration of timers
     * that will be used by LED Controller
     */
    ledc_timer_config_t ledc_timer = {
        .duty_resolution = LEDC_TIMER_13_BIT, // resolution of PWM duty             MAX-DUTY: 8192
        .freq_hz = 5000,                      // frequency of PWM signal
        .speed_mode = LED_HS_MODE,           // timer mode
        .timer_num = LED_HS_TIMER            // timer index
    };
    // Set configuration of timer0 for high speed channels
    ledc_timer_config(&ledc_timer);


    /*
     * Prepare individual configuration
     * for each channel of LED Controller
     * by selecting:
     * - controller's channel number
     * - output duty cycle, set initially to 0
     * - GPIO number where LED is connected to
     * - speed mode, either high or low
     * - timer servicing selected channel
     *   Note: if different channels use one timer,
     *         then frequency and bit_num of these channels
     *         will be the same
     */

    // Set LED Controller with previously prepared configuration
    for ( __uint16_t ch = 0; ch < LED_CH_USED; ch++) {
        ledc_channel_config(&ledc_channel[ch]);
    }

    // Initialize fade service.
    ledc_fade_func_install(0);
    

    ESP_LOGI(TAG, "[APP] Startup..");
    ESP_LOGI(TAG, "[APP] Free memory: %d bytes", esp_get_free_heap_size());
    ESP_LOGI(TAG, "[APP] IDF version: %s", esp_get_idf_version());

    esp_log_level_set("*", ESP_LOG_INFO);
    esp_log_level_set("MQTT_CLIENT", ESP_LOG_VERBOSE);
    esp_log_level_set("TRANSPORT_TCP", ESP_LOG_VERBOSE);
    esp_log_level_set("TRANSPORT_SSL", ESP_LOG_VERBOSE);
    esp_log_level_set("TRANSPORT", ESP_LOG_VERBOSE);
    esp_log_level_set("OUTBOX", ESP_LOG_VERBOSE);

    nvs_flash_init();
    wifi_init();
    mqtt_app_start();

 /*   
      //Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      ESP_ERROR_CHECK(nvs_flash_erase());
      ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    ESP_LOGI(TAG, "ESP_WIFI_MODE_STA");
    wifi_init_sta();
*/

    return true;
}


//---------------------------------------------------------
//WIFI STUFF
//---------------------------------------------------------


esp_err_t wifi_event_handler(void *ctx, system_event_t *event)
{
    switch (event->event_id) {
        case SYSTEM_EVENT_STA_START:
            esp_wifi_connect();
            break;
        case SYSTEM_EVENT_STA_GOT_IP:
            xEventGroupSetBits(wifi_event_group, CONNECTED_BIT);

            break;
        case SYSTEM_EVENT_STA_DISCONNECTED:
            esp_wifi_connect();
            xEventGroupClearBits(wifi_event_group, CONNECTED_BIT);
            break;
        default:
            break;
    }
    return ESP_OK;
}

void wifi_init(void)
{
    tcpip_adapter_init();
    wifi_event_group = xEventGroupCreate();
    ESP_ERROR_CHECK(esp_event_loop_init(wifi_event_handler, NULL));
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));

    wifi_config_t wifi_config = {
        .sta = {
            .ssid = WIFI_SSID,
            .password = WIFI_PASS,
        },
    };

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config));
    ESP_LOGI(TAG, "start the WIFI SSID:[%s] password:[%s]", WIFI_SSID, "******");
    ESP_ERROR_CHECK(esp_wifi_start());
    ESP_LOGI(TAG, "Waiting for wifi");
    xEventGroupWaitBits(wifi_event_group, CONNECTED_BIT, false, true, portMAX_DELAY);
}

// extern const uint8_t iot_eclipse_org_pem_start[] asm("_binary_iot_eclipse_org_pem_start");
// extern const uint8_t iot_eclipse_org_pem_end[]   asm("_binary_iot_eclipse_org_pem_end");






//---------------------------------------------------------
//LED STUFF
//---------------------------------------------------------


bool setTargetBrightness(__uint8_t rgb, __uint64_t newBrightness, __uint32_t fadetime) {
    
        ledc_set_fade_with_time(
            ledc_channel[rgb].speed_mode,
            ledc_channel[rgb].channel, 
            newBrightness, 
            fadetime
        );
    
    return true;
}


bool executeFade() {

    for(__uint8_t i = 0; i < LED_CH_USED; i++) {
        ledc_fade_start(
            ledc_channel[i].speed_mode,
            ledc_channel[i].channel, 
            LEDC_FADE_NO_WAIT   //Doesnt block the fading
        );
    }

    return true;
}


void random(__uint32_t fadetime) {

    __uint64_t red = (rand() % LED_MAX_DUTY) * 1000;
    __uint64_t green = (rand() % LED_MAX_DUTY) * 1000;
    __uint64_t blue = (rand() % LED_MAX_DUTY) * 1000;

    if(red + green + blue == 0)
        red = 3000;

    if(red == green)
        if( red == blue)
            blue = (blue - 2000) % (LED_MAX_DUTY * 1000);

    setTargetBrightness(0, red, fadetime);
    setTargetBrightness(1, green, fadetime);
    setTargetBrightness(2, blue, fadetime);

    executeFade();

    vTaskDelay((fadetime + 400) / portTICK_PERIOD_MS);
}


void party(__uint32_t fadetime) {

    while(1) {
        random(fadetime);
    }
}

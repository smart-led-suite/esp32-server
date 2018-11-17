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
#include "led.h"
#include "wifi.h"


const char *TAG = "MQTTS_SAMPLE";

struct rgb rgb_1;

//------------------------------------------------------------------------------------------------------------------



void app_main()
{
    initialize();
    printf("Initializitaion complete!\n");
    rgb_1.brightness = 0;
    rgb_1.red = 253;
    rgb_1.green = 245;
    rgb_1.blue = 230;
    rgb_1.state = false;
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







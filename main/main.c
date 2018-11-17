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

//#include "mqtt.h"
//-------------------
//WIFI STUFF
/* FreeRTOS event group to signal when we are connected*/
static EventGroupHandle_t wifi_event_group;

/* The event group allows multiple bits for each event,
   but we only care about one event - are we connected
   to the AP with an IP? */

static const char *TAG = "MQTTS_SAMPLE";
const static int CONNECTED_BIT = BIT0;

struct rgb rgb_1;

// buffer used to send/receive data with MQTT
#define MSG_BUFFER_SIZE 20
char msg_buffer[MSG_BUFFER_SIZE]; 
esp_mqtt_client_handle_t client;

void update() {

    uint32_t red = (rgb_1.red * 32 / 100) * rgb_1.brightness;
    uint32_t green = (rgb_1.green * 32 / 100) * rgb_1.brightness;
    uint32_t blue = (rgb_1.blue * 32 / 100) * rgb_1.brightness;

    setTargetBrightness(0, red, 5);
    setTargetBrightness(1, green, 5);
    setTargetBrightness(2, blue, 5);
}


void handleMessage(char * topic_src, uint32_t topic_len, char * data_src, uint32_t data_len) {
    // first we want to copy the strings passed to this function to get null terminated strings
    char topic[topic_len + 1];
    strncpy(topic, topic_src, topic_len);
    topic[topic_len] = 0; // enter terminating null byte
    char data[data_len + 1];
    strncpy(data, data_src, data_len);
    data[data_len] = 0; // terminating null byte
    
    // handle message topic
  if (strcmp(MQTT_LIGHT_COMMAND_TOPIC, topic) == 0) {
    ESP_LOGI(TAG, "switch detected!\n");
    // test if the payload is equal to "ON" or "OFF"
    if (strcmp(data, LIGHT_ON) == 0) {
        printf("LIGHTS ON\n");
      if (rgb_1.state != true) {
        rgb_1.state = true;
        update();
        //printf("update color worked\n");
        publishRGBState();
        //printf("publish worked\n");
      }
    } else if (strcmp(data,LIGHT_OFF) == 0) {
      if (rgb_1.state != false) {
        rgb_1.state = false;
        printf("LIGHTS ON\n");
        update();
        publishRGBState();
      }
    }
  } else if (strcmp(MQTT_LIGHT_BRIGHTNESS_COMMAND_TOPIC, topic) == 0) {
    uint8_t brightness = atoi(data);
    if (brightness < 0 || brightness > 100) {
      // do nothing...
      return;
    } else {
        printf("new brightness: %d\n", brightness);
      rgb_1.brightness = brightness;
      update();
      publishRGBBrightness();
    }
  } else if (strcmp(MQTT_LIGHT_RGB_COMMAND_TOPIC, topic) == 0) {
        char * input = data;
        ESP_LOGI(TAG, "Parsing the input rgb string '%s'\n", input);
        char *token = strtok(input, ",");
        int counter = 0;
        while(token) {
            //ESP_LOGI(TAG, "element: %s\n", token);
            switch (counter) {
                case 0:
                    rgb_1.red = atoi(token);
                    break;
                case 1:
                    rgb_1.green = atoi(token);
                    break;
                case 2: 
                    rgb_1.blue = atoi(token);
                    break;
                default:
                    ESP_LOGI(TAG, "error while parsingrgb values, too many arguments\n");	
            }
            counter++;
            token = strtok(NULL, ",");
        }
        ESP_LOGI(TAG, "rgb: %d, %d, %d\n", rgb_1.red, rgb_1.green, rgb_1.blue);
    }
}


// function called to publish the state of the led (on/off)
void publishRGBState() {
  if (rgb_1.state) {
    int msg_id = esp_mqtt_client_publish(client, MQTT_LIGHT_STATE_TOPIC, LIGHT_ON, 0, 2, 0);
    ESP_LOGI(TAG, "sent publish light on successful, msg_id=%d", msg_id);
  } else {
    int msg_id = esp_mqtt_client_publish(client, MQTT_LIGHT_STATE_TOPIC, LIGHT_OFF, 0, 2, 0);
    ESP_LOGI(TAG, "sent publish light off successful, msg_id=%d", msg_id);
  }
}

// function called to publish the brightness of the led (0-100)
void publishRGBBrightness() {
    snprintf(msg_buffer, MSG_BUFFER_SIZE, "%d", rgb_1.brightness);
    int msg_id = esp_mqtt_client_publish(client, MQTT_LIGHT_BRIGHTNESS_STATE_TOPIC, msg_buffer, 0, 2, 0);
    ESP_LOGI(TAG, "sent publish successful, msg_id=%d", msg_id);
}

// function called to publish the colors of the led (xx(x),xx(x),xx(x))
void publishRGBColor() {
    snprintf(msg_buffer, MSG_BUFFER_SIZE, "%d,%d,%d", rgb_1.red, rgb_1.green, rgb_1.blue);
  
    int msg_id = esp_mqtt_client_publish(client, MQTT_LIGHT_RGB_STATE_TOPIC, msg_buffer, 0, 2, 0);
    ESP_LOGI(TAG, "sent publish successful, msg_id=%d", msg_id);

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

   // party(2000);

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


static esp_err_t wifi_event_handler(void *ctx, system_event_t *event)
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

static void wifi_init(void)
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


static esp_err_t mqtt_event_handler(esp_mqtt_event_handle_t event)
{
    client = event->client;
    int msg_id;
    // your_context_t *context = event->context;
    switch (event->event_id) {
        case MQTT_EVENT_CONNECTED:
            ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");

            // STATE
            msg_id = esp_mqtt_client_subscribe(client, MQTT_LIGHT_COMMAND_TOPIC, 2);
            ESP_LOGI(TAG, "sent subscribe successful, msg_id=%d", msg_id);
           
            // BRIGHTNESS
            msg_id = esp_mqtt_client_subscribe(client, MQTT_LIGHT_BRIGHTNESS_COMMAND_TOPIC, 2);
            ESP_LOGI(TAG, "sent subscribe successful, msg_id=%d", msg_id);

            // COLORS
            msg_id = esp_mqtt_client_subscribe(client, MQTT_LIGHT_RGB_COMMAND_TOPIC, 2);
            ESP_LOGI(TAG, "sent subscribe successful, msg_id=%d", msg_id);

//            msg_id = esp_mqtt_client_unsubscribe(client, "/topic/qos1");
//            ESP_LOGI(TAG, "sent unsubscribe successful, msg_id=%d", msg_id);
            break;
        case MQTT_EVENT_DISCONNECTED:
            ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");
            break;

        case MQTT_EVENT_SUBSCRIBED:
            ESP_LOGI(TAG, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
            msg_id = esp_mqtt_client_publish(client, "/topic/qos0", "data", 0, 0, 0);
            ESP_LOGI(TAG, "sent publish successful, msg_id=%d", msg_id);
            break;
        case MQTT_EVENT_UNSUBSCRIBED:
            ESP_LOGI(TAG, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
            break;
        case MQTT_EVENT_PUBLISHED:
            ESP_LOGI(TAG, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
            break;
        case MQTT_EVENT_DATA:
            ESP_LOGI(TAG, "MQTT_EVENT_DATA");
            printf("TOPIC=%.*s\r\n", event->topic_len, event->topic);
            printf("DATA=%.*s\r\n", event->data_len, event->data);

            handleMessage(event->topic, event->topic_len, event->data, event->data_len);
            if(strcmp(event->topic, "homeassistant/rgb/led1/status") == 0) {
                
            } else if(strcmp(event->topic, "homeassistant/rgb/led1/set") == 0) {
                
            }

            break;
        case MQTT_EVENT_ERROR:
            ESP_LOGI(TAG, "MQTT_EVENT_ERROR");
            break;
    }
    return ESP_OK;
}

static void mqtt_app_start(void)
{
    const esp_mqtt_client_config_t mqtt_cfg = {
 //      .uri = "mqtt://homeassistant:hello@192.168.26.133:1883",
        .event_handle = mqtt_event_handler,
        .host = MQTT_HOST,
        .port = MQTT_PORT,
        .username = MQTT_USERNAME,
        .password = MQTT_PASS,
       // .cert_pem = (const char *)iot_eclipse_org_pem_start,
    };

    ESP_LOGI(TAG, "[APP] Free memory: %d bytes", esp_get_free_heap_size());
    client = esp_mqtt_client_init(&mqtt_cfg);
    esp_mqtt_client_start(client);
}




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

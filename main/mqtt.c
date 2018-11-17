#include "config.h"
#include "mqtt.h"
#include "main.h"
#include "string.h"

// buffer used to send/receive data with MQTT
#define MSG_BUFFER_SIZE 20
char msg_buffer[MSG_BUFFER_SIZE]; 
esp_mqtt_client_handle_t client;


void handleMessage(char * topic_src, uint32_t topic_len, char * data_src, uint32_t data_len) {
    printf("handle message\n");
    // first we want to copy the strings passed to this function to get null terminated strings
    char topic[topic_len + 1];
    strncpy(topic, topic_src, topic_len);
    topic[topic_len] = 0; // enter terminating null byte
    char data[data_len + 1];
    strncpy(data, data_src, data_len);
    data[data_len] = 0; // terminating null byte
    printf("stuff\n");
    // handle message topic
  if (strcmp(MQTT_LIGHT_COMMAND_TOPIC, topic) == 0) {
    ESP_LOGI(TAG, "switch detected!\n");
    // test if the payload is equal to "ON" or "OFF"
    if (strcmp(data, LIGHT_ON) == 0) {
        printf("LIGHTS ON\n");
      if (rgb_1.state != true) {
        rgb_1.state = true;
        rgb_1.brightness = 80;
        update();
        //printf("update color worked\n");
        publishRGBState();
        //printf("publish worked\n");
      }
    } else if (strcmp(data,LIGHT_OFF) == 0) {
      if (rgb_1.state != false) {
        rgb_1.state = false;
        rgb_1.brightness = 0;
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
        update();
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



 esp_err_t mqtt_event_handler(esp_mqtt_event_handle_t event)
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
            publishRGBState();
            publishRGBBrightness();
            publishRGBColor();
            // ESP_LOGI(TAG, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
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

 void mqtt_app_start(void)
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
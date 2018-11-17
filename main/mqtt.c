#include "config.h"
#include "mqtt.h"
#include "main.h"
#include "string.h"



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
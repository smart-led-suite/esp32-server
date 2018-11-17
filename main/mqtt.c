#include "config.h"
#include "mqtt.h"
#include "main.h"
#include "string.h"

void handleMessage(char * topic, char * data) {

    // handle message topic
  if (strcmp(MQTT_LIGHT_COMMAND_TOPIC, topic) == 0) {
    ESP_LOGI(TAG, "switch detected!\n");
    // test if the payload is equal to "ON" or "OFF"
    if (strcmp(data, LIGHT_ON) == 0) {
      if (rgb_1.state != true) {
        rgb_1.state = true;
        update_color(rgb_1.red, rgb_1.green, rgb_1.blue);
        publishRGBState();
      }
    } else if (strcmp(data,LIGHT_OFF) == 0) {
      if (rgb_1.state != false) {
        rgb_1.state = false;
        update_color(0, 0, 0);
        publishRGBState();
      }
    }
  } else if (strcmp(MQTT_LIGHT_BRIGHTNESS_COMMAND_TOPIC, topic) == 0) {
    uint8_t brightness = atoi(data);
    if (brightness < 0 || brightness > 100) {
      // do nothing...
      return;
    } else {
      rgb_1.brightness = brightness;
      update_color(rgb_1.red, rgb_1.green, rgb_1.blue);
      publishRGBBrightness();
    }
  } else if (strcmp(MQTT_LIGHT_RGB_COMMAND_TOPIC, topic) == 0) {
        char * input = data;
        ESP_LOGI(TAG, "Parsing the input rgb string '%s'\n", input);
        char *token = strtok(input, ",");
        int counter = 0;
        while(token) {
            ESP_LOGI(TAG, "element: %s\n", token);
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
        
        // update_color(rgb_1.red, rgb_1.green, rgb_1.blue);
        // publishRGBColor();
    }
}

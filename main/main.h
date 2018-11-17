#ifndef MAIN_H
#define MAIN_H


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

#include <stdint.h>
#include <stddef.h>

#include "freertos/semphr.h"
#include "freertos/queue.h"

#include "lwip/sockets.h"
#include "lwip/dns.h"
#include "lwip/netdb.h"

#include "esp_log.h"
#include "mqtt_client.h"

struct rgb {
    bool state;
    uint8_t brightness;
    uint8_t red;
    uint8_t green;
    uint8_t blue;
};

const char *TAG;

struct rgb rgb_1;

void update_color(uint32_t r, uint32_t g, uint32_t b);

void publishRGBState();
void publishRGBBrightness();
void publishRGBColor();

bool setTargetBrightness(__uint8_t RGB, __uint64_t newBrightness, __uint32_t fadetime);
bool executeFade();
bool executeFadeWithBlock();
bool initialize();

void update();

void wifi_init();
esp_err_t mqtt_event_handler(esp_mqtt_event_handle_t event);
void mqtt_app_start(void);

void random(__uint32_t fadetime);
void party(__uint32_t fadetime);


#endif

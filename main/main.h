#ifndef MAIN_H
#define MAIN_H


#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "driver/ledc.h"
#include "esp_err.h"
#include "config.h"


bool setTargetBrightness(__uint8_t RGB, __uint64_t newBrightness, __uint32_t fadetime);
bool executeFade();
bool executeFadeWithBlock();
bool initialize();

static esp_err_t event_handler(void *ctx, system_event_t *event);
void wifi_init_sta();

void random(__uint32_t fadetime);
void party(__uint32_t fadetime);


#endif
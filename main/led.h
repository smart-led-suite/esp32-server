//---------------------------------------------------------
//LED STUFF
//---------------------------------------------------------
#ifndef LED_H 
#define LED_H
#include "stdint.h"
#include "stdbool.h"
#include "main.h"

bool setTargetBrightness(__uint8_t RGB, __uint64_t newBrightness, __uint32_t fadetime);
bool executeFade();
bool executeFadeWithBlock();
bool initialize();

void update();

void random(__uint32_t fadetime);
void party(__uint32_t fadetime);

ledc_channel_config_t ledc_channel[LED_CH_USED];
#endif
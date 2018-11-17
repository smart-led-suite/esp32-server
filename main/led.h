//---------------------------------------------------------
//LED STUFF
//---------------------------------------------------------
#include "stdint.h"
#include "stdbool.h"
#include "main.h"

bool setTargetBrightness(__uint8_t RGB, __uint64_t newBrightness, __uint32_t fadetime);
bool executeFade();
bool executeFadeWithBlock();
bool initialize();

void update();


ledc_channel_config_t ledc_channel[LED_CH_USED];
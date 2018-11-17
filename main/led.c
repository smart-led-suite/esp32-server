//---------------------------------------------------------
//LED STUFF
//---------------------------------------------------------
#include "led.h"
#include "main.h"


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
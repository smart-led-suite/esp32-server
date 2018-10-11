#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "driver/ledc.h"
#include "esp_err.h"
#include "main.h"
#include "config.h"

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


bool initialize() {

    /*
     * Prepare and set configuration of timers
     * that will be used by LED Controller
     */
    ledc_timer_config_t ledc_timer = {
        .duty_resolution = LEDC_TIMER_13_BIT, // resolution of PWM duty
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

    return true;
}


void app_main()
{
    initialize();
    printf("Initializitaion complete!\n");

    party(3000);

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

bool executeFadeWithBlock() {

    for(__uint8_t i = 0; i < LED_CH_USED; i++) {
        ledc_fade_start(
            ledc_channel[i].speed_mode,
            ledc_channel[i].channel, 
            LEDC_FADE_WAIT_DONE   //Doesnt block the fading
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

    executeFadeWithBlock();
}


void party(__uint32_t fadetime) {

    while(1) {
        random(fadetime);
    }
}

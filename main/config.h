//WIFI
#define WIFI_MODE_AP        FALSE //TRUE:AP FALSE:STA
#define WIFI_MAX_STA_CONN   (4)

#define MQTT_LIGHT_STATE_TOPIC "homeassistant/rgb/led1/status"
#define MQTT_LIGHT_BRIGHTNESS_STATE_TOPIC "homeassistant/rgb/led1/set"

#define LED_1_R             (17)
#define LED_1_R_CH          LEDC_CHANNEL_0
#define LED_1_G             (18)
#define LED_1_G_CH          LEDC_CHANNEL_1
#define LED_1_B             (21)
#define LED_1_B_CH          LEDC_CHANNEL_2


#define LED_2_R             (32)
#define LED_2_R_CH          LEDC_CHANNEL_3
#define LED_2_G             (35)
#define LED_2_G_CH          LEDC_CHANNEL_4
#define LED_2_B             (34)
#define LED_2_B_CH          LEDC_CHANNEL_5

#define LED_CH_USED         (3)
#define LED_HS_TIMER        LEDC_TIMER_0
#define LED_HS_MODE         LEDC_HIGH_SPEED_MODE
#define LED_MAX_DUTY        (8)



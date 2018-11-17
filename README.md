# esp32-server
Software for receiving commands via MQTT from the Home Assistant (https://www.home-assistant.io) Framework. 
You have to add to the configurations.yaml in the ~/.homeassistant/configurations.yaml:

mqtt:
  port: 1883 (standart)
  username: homeassistant(standart)
  password: hello(standart)

light:
  platform: mqtt
  name: "LED Stripe"
  state_topic: "homeassistant/rgb1/light/status"
  command_topic: "homeassistant/rgb1/light/switch"
  brightness_state_topic: "homeassistant/rgb1/brightness/status"
  brightness_command_topic: "homeassistant/rgb1/brightness/set"
  rgb_state_topic: "homeassistant/rgb1/rgb/status"
  rgb_command_topic: "homeassistant/rgb1/rgb/set"
  brightness_scale: 100
  optimistic: false
  
  

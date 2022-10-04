#include <ESP8266WiFi.h>

// Global settings
#define WIFI_SSID "M+Bs-Booze-Bar-Button"
#define WIFI_SECRET "need-drinks-faster"
#define ALIVE_PING_INTERVAL_MS 1000
#define UDP_LISTEN_PORT 2342
#define BUTTON_DELAY 100

// Signals
#define SIG_ALIVE 1
#define SIG_LIGHTS_ON 2

// Button settings
IPAddress BUTTON_IP(192, 168, 2, 1);
IPAddress BUTTON_AP_GATEWAY_IP(192, 168, 2, 1);
IPAddress BUTTON_AP_SUBNET(255, 255, 255, 0);
#define BUTTON_PIN D4
#define BUTTON_ROTARY_BUTTON_PIN D3
#define BUTTON_STATUS_LED_PIN D2

// Client/Light settings
#define LIGHT_LED_PIN D4
#define LIGHT_NUM_LEDS 1
IPAddress LIGHT_IP(192, 168, 2, 2);
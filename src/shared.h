#include <ESP8266WiFi.h>

// Global settings
#define WIFI_SSID "M+Bs-Booze-Bar-Button"
#define WIFI_SECRET "need-drinks-faster"

// Button settings
IPAddress BUTTON_IP(192, 168, 2, 1);
IPAddress BUTTON_AP_GATEWAY_IP(192, 168, 2, 1);
IPAddress BUTTON_AP_SUBNET(255, 255, 255, 0);
#define BUTTON_UDP_LISTEN_PORT 2342
#define BUTTON_PIN 2

// Client/Light settings
#define LIGHT_LED_ALARM_PIN 4
#define LIGHT_LED_WIFI_STATUS_PIN 2
IPAddress LIGHT_IP(192, 168, 2, 2);
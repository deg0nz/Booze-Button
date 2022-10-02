#include <ESP8266WiFi.h>

// AP/Button Settings
#define WIFI_SSID "M+Bs-Booze-Bar-Button"
#define WIFI_SECRET "need-drinks-faster"

IPAddress AP_IP(192, 168, 2, 1);
IPAddress AP_GATEWAY_IP(192, 168, 2, 1);
IPAddress AP_SUBNET(255, 255, 255, 0);

#define BUTTON_UDP_LISTEN_PORT 2390
#define BUTTON_PIN 2

// Client/Light settings
#define LED_ALARM_PIN 4
#define LED_WIFI_STATUS_PIN 2
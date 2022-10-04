//-- Libraries Included --------------------------------------------------------------
#include <ESP8266WiFi.h>
#include <WiFiUDP.h>
#include <FastLED.h>

//-- Local includes ------------------------------------------------------------------
#include "../shared.h"

//------------------------------------------------------------------------------------
// Define I/O Pins
#define LED0 2 // WIFI Module LED

//------------------------------------------------------------------------------------
// Authentication Variables
const String Devicename = "Light_1";

WiFiUDP Udp;
CRGB leds[LIGHT_NUM_LEDS];

//------------------------------------------------------------------------------------
// Some Variables
char packetBufferIn[255]; // buffer for incoming data
char packetBufferOut[255];
unsigned long sendPingTimeCounter = 0;
unsigned int currentColor;

//====================================================================================

void sendAlivePing()
{
  if ((millis() - sendPingTimeCounter) > ALIVE_PING_INTERVAL_MS)
  {
    Serial.println("Sending alive Ping");
    Udp.beginPacket(BUTTON_AP_GATEWAY_IP, UDP_LISTEN_PORT); // the IP Adress must be known
    // Udp.beginPacket(Udp.remoteIP(), Udp.remotePort());         // this can be used, to answer to a peer, if data was received first
    itoa(SIG_ALIVE, packetBufferOut, 10);
    Udp.write(packetBufferOut);
    Udp.endPacket(); //
    sendPingTimeCounter = millis();
  }
}

void flashLED(CRGB color)
{
  Serial.print("Color: ");
  Serial.println(color);
  // Turn the LED on, then pause
  leds[0] = color;
  FastLED.show();
  delay(BUTTON_DELAY);
  // Now turn the LED off, then pause
  leds[0] = CRGB::Black;
  FastLED.show();
}

void handleUdpPacket()
{
  int packetSize = Udp.parsePacket();
  if (packetSize)
  {
    Serial.print("Received packet of size ");
    Serial.println(packetSize);

    Serial.print("From ");
    Serial.print(Udp.remoteIP());
    Serial.print(", port ");
    Serial.println(Udp.remotePort());

    // read the packet into packetBufffer
    int len = Udp.read(packetBufferIn, 255);
    if (len > 0)
    {
      packetBufferIn[len] = 0;
    }
    Serial.println("Contents:");
    Serial.println(packetBufferIn);

    sscanf(packetBufferIn, "%d", &currentColor);
    flashLED(currentColor);
  }
}

void checkWifiAndConnect()
{
  if (WiFi.status() != WL_CONNECTED)
  {
    WiFi.disconnect();          // probably not necessary due to WiFi.status() != WL_CONNECTED
    WiFi.config(LIGHT_IP, BUTTON_AP_GATEWAY_IP, BUTTON_AP_SUBNET);
    WiFi.begin(WIFI_SSID, WIFI_SECRET); // reconnect to the Network
    Serial.println();
    Serial.print("Waiting for WiFi");

    while (WiFi.status() != WL_CONNECTED)
    {
      delay(500);
      Serial.print(".");
    }
    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: " + WiFi.localIP().toString());

    Udp.begin(UDP_LISTEN_PORT);
  }
}

void setup()
{
  // Set ping time counter
  sendPingTimeCounter = millis();
  
  // Init LEDs
  FastLED.addLeds<NEOPIXEL, LIGHT_LED_PIN>(leds, LIGHT_NUM_LEDS);
  leds[0] = CRGB::Black;
  FastLED.show();

  // Start Serial
  Serial.begin(115200);
  
  // Connect to Wifi
  checkWifiAndConnect();
  
  // Start UDP Server
  Udp.begin(UDP_LISTEN_PORT);
}

void loop()
{
  checkWifiAndConnect();
  sendAlivePing();
  handleUdpPacket();
}
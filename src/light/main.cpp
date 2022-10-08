//-- Libraries Included --------------------------------------------------------------
#include <ESP8266WiFi.h>
#include <WiFiUDP.h>
#include <FastLED.h>
#include <RotaryEncoder.h>

//-- Local includes ------------------------------------------------------------------
#include "../shared.h"
#define ROTARY_PIN1 D6
#define ROTARY_PIN2 D7

//------------------------------------------------------------------------------------
WiFiUDP Udp;
CRGB leds[LIGHT_NUM_LEDS];
RotaryEncoder encoder(ROTARY_PIN1, ROTARY_PIN2, RotaryEncoder::LatchMode::TWO03);

//------------------------------------------------------------------------------------
// Some Variables
char packetBufferIn[255]; // buffer for incoming data
char packetBufferOut[255];
unsigned long sendPingTimeCounter = 0;
unsigned int currentColor = CRGB::Green;
unsigned int currentBrightness = 255;
unsigned int showCurrentBrightnessActivityTracker = 0;

//====================================================================================

void sendAlivePing()
{

  if ((millis() - sendPingTimeCounter) > ALIVE_PING_INTERVAL_MS)
  {
    Serial.print("Signal strength: ");
    Serial.println(WiFi.RSSI());

    Serial.println("Sending alive Ping");
    Udp.beginPacket(BUTTON_AP_GATEWAY_IP, UDP_LISTEN_PORT); // the IP Adress must be known
    // Udp.beginPacket(Udp.remoteIP(), Udp.remotePort());         // this can be used, to answer to a peer, if data was received first
    itoa(SIG_ALIVE, packetBufferOut, 10);
    Udp.write(packetBufferOut);
    Udp.endPacket(); //
    sendPingTimeCounter = millis();
  }
}

void setColor(CRGB color)
{
  for (uint i = 0; i < LIGHT_NUM_LEDS - 1; i++)
  {
    leds[i] = color;
  }
}

void pulsate(CRGB color)
{
  setColor(color);

  for (unsigned int i = 1; i < 255; i++)
  {
    FastLED.setBrightness(i);
    FastLED.show();
    delay(5);
  }
  
  for (unsigned int i = 255; i > 1; i--)
  {
    FastLED.setBrightness(i);
    FastLED.show();
    delay(5);
  }

  setColor(CRGB::Black);
  FastLED.show();
}

void flashLED(CRGB color, unsigned int timeMs)
{
  Serial.print("Color: ");
  Serial.println(color);
  // Turn the LED on, then pause
  setColor(color);
  FastLED.show();
  delay(timeMs);
  // Now turn the LED off, then pause
  setColor(CRGB::Black);
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
    flashLED(currentColor, BUTTON_DELAY);
  }
}

void handleRotaryEncoder()
{
  static int pos = 0;
  encoder.tick();

  int directionValue = (int)(encoder.getDirection());
  // We only want to react when the encoder is rotated
  if (directionValue != 0) {
    int nextBrightness = currentBrightness + directionValue;

    if (nextBrightness < 1 || nextBrightness > 255)
    {
      return;
    } 

    currentBrightness = nextBrightness;

    Serial.print("Current Brightness: ");
    Serial.println(currentBrightness);

    FastLED.setBrightness(currentBrightness);
    setColor(currentColor);
    FastLED.show();
    showCurrentBrightnessActivityTracker = millis();
  }

  // Check for color change mode timeout
  if ((millis() - showCurrentBrightnessActivityTracker) > 1000)
  {
    showCurrentBrightnessActivityTracker = 0;
    setColor(CRGB::Black);
    FastLED.show();
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
      pulsate(CRGB::DarkViolet);
      FastLED.setBrightness(255);
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
  setColor(CRGB::Black);
  FastLED.setBrightness(currentBrightness);
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
  handleRotaryEncoder();
}
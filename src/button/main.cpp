//-- Libraries Included --------------------------------------------------------------
#include <ESP8266WiFi.h>
#include <WiFiUDP.h>
#include <FastLED.h>
#include <RotaryEncoder.h>

//-- Local includes ------------------------------------------------------------------
#include "../shared.h"

//------------------------------------------------------------------------------------
// Define I/O Pins
#define LED0 2 // WIFI Module LED
#define PIN_IN1 D5
#define PIN_IN2 D6

//------------------------------------------------------------------------------------
WiFiUDP Udp;
CRGB leds[1];
RotaryEncoder encoder(PIN_IN1, PIN_IN2, RotaryEncoder::LatchMode::TWO03);

//------------------------------------------------------------------------------------
// Some Variables
char packetBufferIn[255]; // buffer to hold incoming packet
char packetBufferOut[255];
unsigned int currentColor;
char result[10];
int buttonState = 0;
unsigned long clientAliveTracker = 0;

int colorIndex = 0;
unsigned int colors[] = { CRGB::GreenYellow, CRGB::OrangeRed, CRGB::Red };
int colorsSize = sizeof(colors) / sizeof(CRGB);
bool colorChangeActive = false;
unsigned int colorChangeActivityTracker = 0;

//====================================================================================

void handleSignals(long sig)
{
  if (colorChangeActive) 
  {
    return;
  }

  switch (sig)
  {
  case SIG_ALIVE:
    Serial.println("Got alive signal.");
    clientAliveTracker = millis();
    leds[0] = CRGB::Green;
    break;

  default:
    break;
  }

  if ((millis() - clientAliveTracker) > ALIVE_PING_INTERVAL_MS * 2)
  {
    leds[0] = CRGB::Black;
  }

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

    // read the packet into packetBuffer
    int len = Udp.read(packetBufferIn, 255);
    if (len > 0)
    {
      packetBufferIn[len] = 0;
    }
    Serial.println("Contents:");
    Serial.println(packetBufferIn);

    int signal;
    sscanf(packetBufferIn, "%d", &signal);
    handleSignals(signal);
  }
}

void sendLightPacket()
{
  Serial.print("Sending light packet - ");
  Udp.beginPacket(LIGHT_IP, UDP_LISTEN_PORT);
  itoa(colors[colorIndex], packetBufferOut, 10);
  Serial.print("Current color: ");
  Serial.print(colors[colorIndex]);
  Serial.print(" | Out: ");
  Serial.println(packetBufferOut);
  Udp.write(packetBufferOut);
  Udp.endPacket();
}

void handleLightButton()
{
  buttonState = digitalRead(BUTTON_PIN);
  if (buttonState == LOW) {
    Serial.println("Light button pressed.");
    sendLightPacket();
    delay(BUTTON_DELAY);
  }
}

void handleRotaryEncoder()
{
  static int pos = 0;
  encoder.tick();

  int directionValue = (int)(encoder.getDirection());

  if (directionValue != 0) {
    int nextIndex = colorIndex + directionValue;

    if (nextIndex < 0)
    {
      colorIndex = colorsSize - 1;
    } 
    else if (nextIndex > colorsSize - 1)
    {
      colorIndex = 0;
    }
    else
    {
      colorIndex += directionValue;
    }


// TODO: eigene Funktion
    leds[0] = colors[colorIndex];
    FastLED.show();

    colorChangeActive = true;
    colorChangeActivityTracker = millis();
  }

  if ((millis() - colorChangeActivityTracker) > 3000)
  {
    colorChangeActive = false;
    colorChangeActivityTracker = 0;
  }
}

void WifiSetup()
{
  // Stop active previous WIFI
  WiFi.disconnect();

  // Setting the WiFi mode
  WiFi.mode(WIFI_AP_STA);
  Serial.println("WiFi Mode : AccessPoint Station");

  // Starting the AccessPoint
  WiFi.softAPConfig(BUTTON_IP, BUTTON_AP_GATEWAY_IP, BUTTON_AP_SUBNET); // softAPConfig (local_ip, gateway, subnet)
  WiFi.softAP(WIFI_SSID, WIFI_SECRET);   // WiFi.softAP(ssid, password, channel, hidden, max_connection)
  Serial.println("WIFI < " + String(WIFI_SSID) + " > ... Started");

  // Wait a bit
  delay(50);

  // Getting server IP
  IPAddress IP = WiFi.softAPIP();

  // Printing the server IP Address
  Serial.print("AccessPoint IP : ");
  Serial.println(IP);
}

void setup()
{
  Serial.begin(115200);

  // Init LEDs
  FastLED.addLeds<NEOPIXEL, BUTTON_STATUS_LED_PIN>(leds, 1);
  leds[0] = CRGB::Black;
  FastLED.setBrightness(10);
  FastLED.show();

  // Init button
  pinMode(BUTTON_PIN, INPUT);

  // Starting UDP Server
  Udp.begin(UDP_LISTEN_PORT);
  Serial.println("UDP Server Started");

  // Setting Up A Wifi Access Point
  WifiSetup();
}

void loop()
{
  handleLightButton();
  handleUdpPacket();
  handleRotaryEncoder();
}
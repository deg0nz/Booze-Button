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
char result[10];
int buttonState = 0;
unsigned long clientAliveTracker = 0;

unsigned int colors[] = { CRGB::Green, CRGB::Yellow, CRGB::Red };
int colorsLength = sizeof(colors) / sizeof(unsigned int);
int colorsIndex = 0;
bool showCurrentColorActive = false;
unsigned int showCurrentColorActivityTracker = 0;

//====================================================================================

void handleSignals(long sig)
{
  switch (sig)
  {
  case SIG_ALIVE:
    if (showCurrentColorActive) 
    {
      return;
    }

    Serial.println("Got alive signal.");
    clientAliveTracker = millis();
    leds[0] = CRGB::Violet;
    break;

  default:
    break;
  }

  // Check for light aliveness ping timeout
  if ((millis() - clientAliveTracker) > ALIVE_PING_INTERVAL_MS * 2)
  {
    leds[0] = CRGB::Black;
  }

  FastLED.show();
}

void chores()
{
  // Check for color change mode timeout
  if ((millis() - showCurrentColorActivityTracker) > 3000)
  {
    showCurrentColorActive = false;
    showCurrentColorActivityTracker = 0;
  }
}

void handleUdpPacket()
{
  int packetSize = Udp.parsePacket();
  if (packetSize)
  {
    // Serial.print("Received packet of size ");
    // Serial.println(packetSize);

    // Serial.print("From ");
    // Serial.print(Udp.remoteIP());
    // Serial.print(", port ");
    // Serial.println(Udp.remotePort());

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
  Udp.beginPacket(LIGHT_IP, UDP_LISTEN_PORT);
  itoa(colors[colorsIndex], packetBufferOut, 10);
  Udp.write(packetBufferOut);
  Udp.endPacket();

  Serial.print("Sending light packet - ");
  Serial.print("Current color: ");
  Serial.print(colors[colorsIndex]);
  Serial.print(" | Out: ");
  Serial.println(packetBufferOut);
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

void showCurrentColor()
{
  leds[0] = colors[colorsIndex];
  FastLED.show();

  showCurrentColorActive = true;
  showCurrentColorActivityTracker = millis();
}

void handleRotaryEncoder()
{
  static int pos = 0;
  encoder.tick();

  int directionValue = (int)(encoder.getDirection());

  // We only want to react when the encoder is rotated
  if (directionValue != 0) {
    int nextIndex = colorsIndex + directionValue;

    // Limit index boundaries to colors array size
    if (nextIndex < 0)
    {
      colorsIndex = colorsLength - 1;
    } 
    else if (nextIndex > colorsLength - 1)
    {
      colorsIndex = 0;
    }
    else
    {
      colorsIndex = nextIndex;
    }

    Serial.print("New colorsIndex: ");
    Serial.println(colorsIndex);

    showCurrentColor();
  }

  // Handle pressed rotary encoder
  buttonState = digitalRead(BUTTON_ROTARY_BUTTON_PIN);
  if (buttonState == LOW) {
    Serial.println("Rotary button pressed.");
    showCurrentColor();
    delay(BUTTON_DELAY);
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
  // Init serial
  Serial.begin(115200);

  // Init LEDs
  FastLED.addLeds<NEOPIXEL, BUTTON_STATUS_LED_PIN>(leds, 1);
  leds[0] = CRGB::Black;
  FastLED.setBrightness(20);
  FastLED.show();

  // Init main button (the one that activates the light)
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  // Init rotary button (shows currently selected color)
  pinMode(BUTTON_ROTARY_BUTTON_PIN, INPUT_PULLUP);

  // Setting Up A Wifi Access Point
  WifiSetup();

  // Starting UDP Server
  Udp.begin(UDP_LISTEN_PORT);
  Serial.println("UDP Server Started");
}

void loop()
{
  handleLightButton();
  handleUdpPacket();
  handleRotaryEncoder();
  chores();
}
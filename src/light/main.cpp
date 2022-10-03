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

//------------------------------------------------------------------------------------
// WIFI Module Role & Port
IPAddress APlocal_IP(192, 168, 4, 1);
// IPAddress    apIP(10, 10, 10, 1);
IPAddress APgateway(192, 168, 4, 1);
IPAddress APsubnet(255, 255, 255, 0);

WiFiUDP Udp;

//------------------------------------------------------------------------------------
// Some Variables
char result[16];        // Buffer big enough for 7-character float
char packetBuffer[255]; // buffer for incoming data
const String ping = "Ping!";
unsigned long pingTimeCounter = 0;
CRGB leds[LIGHT_NUM_LEDS];

//====================================================================================

//====================================================================================

void Send_Data_To_Server()
{
  unsigned long tNow;

  tNow = millis();             // get the current runtime
  dtostrf(tNow, 8, 0, result); // translate it to a char array.

  Udp.beginPacket(BUTTON_AP_GATEWAY_IP, UDP_LISTEN_PORT); // the IP Adress must be known
  // Udp.beginPacket(Udp.remoteIP(), Udp.remotePort());         // this can be used, to answer to a peer, if data was received first
  Udp.write(result);
  Udp.endPacket(); // this will automatically send the data

  while (1)
  {
    int packetSize = Udp.parsePacket();
    if (packetSize)
    {
      Serial.print("Received packet of size ");
      Serial.println(packetSize);
      Serial.print("From ");
      IPAddress remoteIp = Udp.remoteIP();
      Serial.print(remoteIp);
      Serial.print(", port ");
      Serial.println(Udp.remotePort());

      // read the packet into packetBufffer
      int len = Udp.read(packetBuffer, 255);
      if (len > 0)
      {
        packetBuffer[len] = 0;
      }
      Serial.print("Contents:");
      Serial.println(packetBuffer);
      break; // exit the while-loop
    }
    if ((millis() - tNow) > ALIVE_PING_INTERVAL_MS)
    { // if more then 1 second no reply -> exit
      Serial.println("timeout");
      break; // exit
    }

    delay(500);
  }
}

void sendAlivePing()
{
  if ((millis() - pingTimeCounter) > ALIVE_PING_INTERVAL_MS)
  {
    Serial.println("Sending alive Ping");
    Udp.beginPacket(BUTTON_AP_GATEWAY_IP, UDP_LISTEN_PORT); // the IP Adress must be known
    // Udp.beginPacket(Udp.remoteIP(), Udp.remotePort());         // this can be used, to answer to a peer, if data was received first
    Udp.write(ping.c_str());
    Udp.endPacket(); //
    pingTimeCounter = millis();
  }
}

void flashLED()
{
  // Turn the LED on, then pause
  leds[0] = CRGB::Red;
  FastLED.show();
  delay(200);
  // Now turn the LED off, then pause
  leds[0] = CRGB::Black;
  FastLED.show();
}

void handleUdpPacket()
{
  // unsigned long tNow;

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
    int len = Udp.read(packetBuffer, 255);
    if (len > 0)
    {
      packetBuffer[len] = 0;
    }
    Serial.println("Contents:");
    Serial.println(packetBuffer);

    if (packetBuffer == "Lights!") {
      flashLED();
    }

    // tNow = millis();
    // dtostrf(tNow, 8, 0, result);

    // Udp.beginPacket(Udp.remoteIP(), Udp.remotePort());
    // Udp.write(result);
    // Udp.endPacket();
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
  pingTimeCounter = millis();
  
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
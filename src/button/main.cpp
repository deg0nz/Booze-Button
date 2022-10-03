//-- Libraries Included --------------------------------------------------------------
#include <ESP8266WiFi.h>
#include <WiFiUDP.h>

//-- Local includes ------------------------------------------------------------------
#include "../shared.h"

//------------------------------------------------------------------------------------
// Define I/O Pins
#define LED0 2 // WIFI Module LED

//------------------------------------------------------------------------------------
WiFiUDP Udp;

//------------------------------------------------------------------------------------
// Some Variables
char packetBuffer[255]; // buffer to hold incoming packet
char result[10];
int buttonState = 0;
const String light = "Light!";

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

    // tNow = millis();
    // dtostrf(tNow, 8, 0, result);

    // Udp.beginPacket(Udp.remoteIP(), Udp.remotePort());
    // Udp.write(result);
    // Udp.endPacket();
  }
}

void sendLightPacket()
{
  Serial.println("Sending light packet.");
  Udp.beginPacket(LIGHT_IP, UDP_LISTEN_PORT); // the IP Adress must be known
  // Udp.beginPacket(Udp.remoteIP(), Udp.remotePort());         // this can be used, to answer to a peer, if data was received first
  Udp.write(light.c_str());
  Udp.endPacket(); //
}

void handleButton()
{
  buttonState = digitalRead(BUTTON_PIN);
  if (buttonState == LOW) {
    // Send UDP packet
    Serial.println("Button pressed.");
    sendLightPacket();
    delay(200);
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

  // Starting UDP Server
  Udp.begin(UDP_LISTEN_PORT);

  Serial.println("Server Started");
}

void setup()
{
  Serial.begin(115200);

  // Init button
  pinMode(BUTTON_PIN, INPUT);

  // Setting Up A Wifi Access Point
  WifiSetup();
}

void loop()
{
  handleButton();
  handleUdpPacket();
}
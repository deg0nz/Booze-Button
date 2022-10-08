# Booze-Button

A small ESP8266 device-to-device program where one device acts as a button and the other works as a signalling light. I created this as a gag project to joke with the bartenders in my favourite bar to hurry up if an order for new cocktails takes too long (a.k.a. the bar is pretty busy).

## How it works

There are two NodeMCU modules. One is called "Light", the other one is called "Button".

### Button

* Creates WiFi AP
* Has a rotary encoder to change colors
* Has a status LED to indicate current color and if the Light is connected and sends keepalive packets
* Has a UDP server to receive keepalive packets
* On button press, it sends UDP packets to the light containing the color

### Light

* Connects to button AP
* Sends UDP keepalive packets
* LED pulsates when disconnected from the button
* Has rotary encoder to dim the light
* Has a UDP server that listens for light up/color packets

## Credits

* ESP-to-ESP UDP communication example taken from https://www.hackster.io/hammadiqbal12/esp8266-to-esp8266-direct-communication-bf96ce
* Multi-Board Platform.io example taken from https://xcorr.net/2016/12/13/multi-arduino-projects-with-platformio/

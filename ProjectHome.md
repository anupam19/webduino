# The current version is 1.7 available from https://github.com/sirleech/Webduino.  Future development is hosted on GitHub with this site being a historical archive of the system at version 1.4.1. #

![http://farm3.static.flickr.com/2580/3749936397_3920a0cb46_m_d.jpg](http://farm3.static.flickr.com/2580/3749936397_3920a0cb46_m_d.jpg)

This is an Arduino-based Web Server library, originally developed for a class at NYC Resistor.  It's called Webduino, and it's an extensible web server library for the Arduino using the Wiznet-based Ethernet shields.  It's released under the MIT license allowing all sorts of reuse.  I've also put my not-quite-complete presentation up for viewing at http://j.mp/WebduinoPresentation .

I've got a few examples up right now -- the Buzz example interfaces with Zach's Danger Shield allowing you to remotely start and stop the speaker.  We had a room full of students pinging my Arduino board on Saturday, buzzing me while I was lecturing.  It was fun.  The big Demo example shows how to setup an HTML form running on the device where you can read and write pins.

I hope to add a few more examples in the next few weeks, including a web-enabled fridge sign using a serial LCD and how to serve things other than just HTML text from the device.  I also need to work on more documentation and finishing up my slides.

## Installation Notes ##

With Arduino 0016 or earlier, put the WebServer.h file in the
`hardware/libraries/webduino` subdirectory of the arduino tree.

With Arduino 0017 or later, add the Webduino folder to the "libraries"
folder of your sketchbook directory.  See
http://arduino.cc/blog/?p=313 for more details on the new setup.

You can put the examples in your own sketchbook directory, or in
hardware/libraries/webduino/examples, as you prefer.

If you get an error message when building the examples similar to
"WebServer.h not found", it's a problem with where you put the
Webduino folder.  The server won't work if the header is directly in
the libraries folder.

## Supplemental Libraries ##

http://arduiniana.org/libraries/streaming/ - Streaming library to make it easy to do complex output on a class derived from Print.  A simpler form of this is included in the Web\_Demo sketch as the `operator <<` template.

http://www.arduino.cc/cgi-bin/yabb2/YaBB.pl?num=1233499187 - Ethernet 2 library, a rewritten library for talking to the Ethernet shield that works a little better and takes less program space.

http://www.arduino.cc/cgi-bin/yabb2/YaBB.pl?num=1238640832 - replacement for the Client.cpp file in the Arduino 15 Hardware/Library/Ethernet folder.  This is also used with Ethernet2.  You want to use this because it makes your server connections more reliable.  I hope this becomes an official part of the Arduino 16 tools.

## Presentation ##

[Wedbuino Presentation](http://docs.google.com/present/view?id=dd8gqxt8_5c8w9qfg3) on Google Docs.

## Support Thread on Arduino.cc ##

["Introducing the Webduino web server library" discussion thread](http://www.arduino.cc/cgi-bin/yabb2/YaBB.pl?num=1238478357) on the arduino.cc message board

## Compatible Ethernet Shields ##

These have all been tested with the library successfully:

  * [Arduino Ethernet Shield](http://arduino.cc/en/Main/ArduinoEthernetShield), both original and updated microSD version
  * [adafruit Ethernet Shield w/ Wiznet 811MJ module](http://www.adafruit.com/index.php?main_page=product_info&cPath=17_21&products_id=83&zenid=e8b1e70a0bcb187eef3dd5d9554a750c)
  * [NKC Electronics Ethernet Shield DIY Kit](http://www.nkcelectronics.com/nkc-ethernet-shield-for-arduino-mega--duemilanove--diecimila-diy-kit.html)
  * [Freetronics Ethernet Shield](http://www.freetronics.com/products/ethernet-shield-with-poe)

These shields are not compatible with the library at this time

  * adafruit Ethernet Shield w/ XPort or XPort+ module; these talk to the XPort module over serial; to handle incoming connections, you would need to both configure the XPort module to listen to your server port and modify the processConnections code to look for a "carrier" on the serial line.
  * [Ethernet Adapter for MCU Projects](http://www.seeedstudio.com/depot/ethernet-adapter-for-mcu-projects-p-160.html) from Seeed Studio; this shield uses the ENC28J60 chip which requires implementing your own TCP/IP stack in software
# Ansluta-Remote-Controller
DIY remote controller for the Ikea Ansluta lights

##Intro

The Ansluta line of Ikea lamps only let u use one remote controller for each light. 
I wanted to use multiple remote's to control one string of lights.

This project is based around an CC2500 2.4Ghz wireless controller and an Atmega328 (ex Arduino Nano).

The choise for the CC2500 was easy because the original Anluta Remote uses this IC, so it's definately possible to use this for controlling my lamp.

Some code is loosely based on:
https://github.com/Zohan/ArduinoCC2500Demo

##Method
I had a look inside an orginal Ansluta remote, it uses an texas instrument uC (MSP430G2221) and an CC2500.
The CC2500 communicates over an SPI bus to the uC. So I've connected a "Bus Pirate" to the SPI bus of the uC and sniffed the commands and data packets to the wireless chip.

The CC2500 is readily available as module on ebay with integrated antenna so no RF design is needed.




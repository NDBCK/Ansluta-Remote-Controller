# Ansluta-Remote-Controller
DIY remote controller for the Ikea Ansluta lights

##Intro

The Ansluta line of Ikea lamps only let u use one remote controller for each light. 
I wanted to use multiple remote's to control one string of lights.

This project is based around an CC2500 2.4Ghz wireless controller and an Atmega328 (ex Arduino Nano).

The choise for the CC2500 was easy because the original Anluta Remote uses this IC, so it's definately possible to use this for controlling my lamp.

Some code is loosely based on:
https://github.com/Zohan/ArduinoCC2500Demo

##Work in progress

###Sniffing SPI
I had a look inside an orginal Ansluta remote, it uses an texas instrument uC (MSP430G2221) and an CC2500.
The CC2500 communicates over an SPI bus to the uC. So I've connected a "Bus Pirate" to the SPI bus of the uC and sniffed the commands and data packets to the wireless chip.

The settings for the bus pirate were (SPI sniffer):
Clock Edge= 0, Polarity= 1 RawData= 0 

The sniffed data is in the file: "SPI_DATA.txt"

##Decoding SPI

Every time the button is pressed 2 data strobes are sent followed by a burst of data followed by a final strobe.

Together with the datasheet from we can decode the data:

Strobe 1:
5B 5B 5C 5C 36 36 0F 0F 5D 5D 	//0x36 SIDLE Exit RX / TX, turn off frequency synthesizer and exit Wake-On-Radio mode if applicable

Strobe 2:
5B 5B 5C 5C 3B 3B 0F 0F 5D 5D 	//0x3B SFTX Flush the TX FIFO buffer. Only issue SFTX in IDLE or TXFIFO_UNDERFLOW states.

Burst of data:
5B 5B 5C 5C 7F 7F 0F 0F 5C 5C 	//0x7F Burst access to TX FIFO
06 06 0F 0F 5C 5C 
55 55 0F 0F 5C 5C 
01 01 0F 0F 5C 5C 
3E 3E 0F 0F 5C 5C 
94 94 0F 0F 5C 5C 
02 02 0F 0F 5C 5C 
AA AA 0F 0F 5C 5C 
FF FF 0F 0F 5D 5D 

Final strobe:
5B 5B 5C 5C 35 35 0F 0F 5D 5D 	//0x35 STX In IDLE state: Enable TX. 


Clearly some data is missing (configuration of the CC2500).
This is when the battery's are inserted in the remote some initializing data is sent to the CC2500 (without pressing a button).

So the module is awakened from sleep mode, the send buffer is cleared, data is loaded into the buffer and finaly the data is transmitted.



The CC2500 is readily available as module on ebay with integrated antenna so no RF design is needed.




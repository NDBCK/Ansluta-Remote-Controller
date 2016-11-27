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

###Decoding SPI

####Button press
Every time the button is pressed 2 data strobes are sent followed by a burst of data followed by a final strobe.

Together with the datasheet from we can decode the data:

```
//Strobe 1:
5B 5B 5C 5C 36 36 0F 0F 5D 5D 	//0x36 SIDLE Exit RX/TX, turn off frequency synthesizer and exit Wake-On-Radio mode if applicable

//Strobe 2:
5B 5B 5C 5C 3B 3B 0F 0F 5D 5D 	//0x3B SFTX Flush the TX FIFO buffer. Only issue SFTX in IDLE or TXFIFO_UNDERFLOW states.

//Burst of data:
5B 5B 5C 5C 7F 7F 0F 0F 5C 5C 	//0x7F Burst access to TX FIFO
06 06 0F 0F 5C 5C 
55 55 0F 0F 5C 5C 
01 01 0F 0F 5C 5C 
3E 3E 0F 0F 5C 5C
94 94 0F 0F 5C 5C
02 02 0F 0F 5C 5C               //Indicates the light intensity 01=OFF; 02=50%; 03=100%
AA AA 0F 0F 5C 5C 
FF FF 0F 0F 5D 5D 

//Final strobe:
5B 5B 5C 5C 35 35 0F 0F 5D 5D 	//0x35 STX In IDLE state: Enable TX. 
```
So the module is awakened from sleep mode, the send buffer is cleared, data is loaded into the buffer and finaly the data is transmitted.

Clearly some data is missing (configuration of the CC2500).


####Initializing Data
When the battery's are inserted in the remote some initializing data is sent to the CC2500 (without pressing a button).
Indicating that the module is powered on continuously and a power-down method is used for preserving the battery's.

```
SPI Data                                            Register        Value

5B [5C 30 0x30(0F 0x0F)5D ]
5B [5C 00 0x00(0F 0x0F)5C 29 0x29(0F 0x0F)5D ]	//	IOCFG2           0x29
5B [5C 02 0x02(0F 0x0F)5C 06 0x06(0F 0x0F)5D ]	//	IOCFG0           0x06
5B [5C 06 0x06(0F 0x0F)5C FF 0xFF(0F 0x0F)5D ]	//	PKTLEN           0xFF
5B [5C 07 0x07(0F 0x0F)5C 04 0x04(0F 0x0F)5D ]	//	PKTCTRL1         0x04
5B [5C 08 0x08(0F 0x0F)5C 05 0x05(0F 0x0F)5D ]	//	PKTCTRL0         0x05
5B [5C 09 0x09(0F 0x0F)5C 01 0x01(0F 0x0F)5D ]	//	ADDR             0x01
5B [5C 0A 0x0A(0F 0x0F)5C 10 0x10(0F 0x0F)5D ]	//	CHANNR           0x10
5B [5C 0B 0x0B(0F 0x0F)5C 09 0x09(0F 0x0F)5D ]	//	FSCTRL1          0x09
5B [5C 0C 0x0C(0F 0x0F)5C 00 0x00(0F 0x0F)5D ]	//	FSCTRL0          0x00
5B [5C 0D 0x0D(0F 0x0F)5C 5D 0x5D(0F 0x0F)5D ]	//	FREQ2            0x5D
5B [5C 0E 0x0E(0F 0x0F)5C 93 0x93(0F 0x0F)5D ]	//	FREQ1            0x93
5B [5C 0F 0x0F(0F 0x0F)5C B1 0xB1(0F 0x0F)5D ]	//	FREQ0            0xB1
5B [5C 10 0x10(0F 0x0F)5C 2D 0x2D(0F 0x0F)5D ]	//	MDMCFG4          0x2D
5B [5C 11 0x11(0F 0x0F)5C 3B 0x3B(0F 0x0F)5D ]	//	MDMCFG3          0x3B
5B [5C 12 0x12(0F 0x0F)5C 73 0x73(0F 0x0F)5D ]	//	MDMCFG2          0x73  
5B [5C 13 0x13(0F 0x0F)5C A2 0xA2(0F 0x0F)5D ]	//	MDMCFG1          0xA2
5B [5C 14 0x14(0F 0x0F)5C F8 0xF8(0F 0x0F)5D ]	//	MDMCFG0          0xF8
5B [5C 15 0x15(0F 0x0F)5C 01 0x01(0F 0x0F)5D ]	//	DEVIATN          0x01
5B [5C 16 0x16(0F 0x0F)5C 07 0x07(0F 0x0F)5D ]	//	MCSM2            0x07
5B [5C 17 0x17(0F 0x0F)5C 30 0x30(0F 0x0F)5D ]	//	MCSM1            0x30
5B [5C 18 0x18(0F 0x0F)5C 18 0x18(0F 0x0F)5D ]	//	MCSM0            0x18
5B [5C 19 0x19(0F 0x0F)5C 1D 0x1D(0F 0x0F)5D ]	//	FOCCFG           0x1D
5B [5C 1A 0x1A(0F 0x0F)5C 1C 0x1C(0F 0x0F)5D ]	//	BSCFG            0x1C
5B [5C 1B 0x1B(0F 0x0F)5C C7 0xC7(0F 0x0F)5D ]	//	AGCCTRL2         0xC7
5B [5C 1C 0x1C(0F 0x0F)5C 00 0x00(0F 0x0F)5D ]	//	AGCCTRL1         0x00
5B [5C 1D 0x1D(0F 0x0F)5C B2 0xB2(0F 0x0F)5D ]	//	AGCCTRL0         0xB2
5B [5C 1E 0x1E(0F 0x0F)5C 87 0x87(0F 0x0F)5D ]	//	WOREVT1          0x87
5B [5C 1F 0x1F(0F 0x0F)5C 6B 0x6B(0F 0x0F)5D ]	//	WOREVT0          0x6B
5B [5C 20 0x20(0F 0x0F)5C F8 0xF8(0F 0x0F)5D ]	//	WORCTRL          0xF8
5B [5C 21 0x21(0F 0x0F)5C B6 0xB6(0F 0x0F)5D ]	//	FREND1           0xB6
5B [5C 22 0x22(0F 0x0F)5C 10 0x10(0F 0x0F)5D ]	//	FREND0           0x10
5B [5C 23 0x23(0F 0x0F)5C EA 0xEA(0F 0x0F)5D ]	//	FSCAL3           0xEA
5B [5C 24 0x24(0F 0x0F)5C 0A 0x0A(0F 0x0F)5D ]	//	FSCAL2           0x0A
5B [5C 25 0x25(0F 0x0F)5C 00 0x00(0F 0x0F)5D ]	//	FSCAL1           0x00
5B [5C 26 0x26(0F 0x0F)5C 11 0x11(0F 0x0F)5D ]	//	FSCAL0           0x11
5B [5C 27 0x27(0F 0x0F)5C 41 0x41(0F 0x0F)5D ]	//	RCCTRL1          0x41
5B [5C 28 0x28(0F 0x0F)5C 00 0x00(0F 0x0F)5D ]	//	RCCTRL0          0x00
5B [5C 29 0x29(0F 0x0F)5C 59 0x59(0F 0x0F)5D ]	//	FSTEST           0x59
5B [5C 2C 0x2C(0F 0x0F)5C 88 0x88(0F 0x0F)5D ]	//	TEST2            0x88
5B [5C 2D 0x2D(0F 0x0F)5C 31 0x31(0F 0x0F)5D ]	//	TEST1            0x31
5B [5C 2E 0x2E(0F 0x0F)5C 0B 0x0B(0F 0x0F)5D ]	//	TEST0            0x0B
5B [5C 7E 0x7E(0F 0x0F)5C FF 0xFF(0F 0x0F)5D ]	//	?????            0xFF
5B [5C 39 0x39(0F 0x0F)5D ]		

```
The CC2500 is readily available as module on ebay with integrated antenna so no RF design is needed.




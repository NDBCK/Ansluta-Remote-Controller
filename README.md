# Ansluta-Remote-Controller
DIY remote controller for the Ikea Ansluta lights (the 2.4Ghz version)

##Intro

The Ansluta (OMLOPP) line of Ikea lamps only let u use one remote controller for each light. 
I wanted to use multiple remotes to control one string of lights.

Another caveat of the original remotes is that it can only cycle through the different brightness levels. It cycles though: 0% - 50% - 100% - 50% - 0% - ....
I would like to turn it on to 50% brightness and turn it off without cycling to 100%.

![alt text](https://github.com/NDBCK/Ansluta-Remote-Controller/blob/master/anslutaOr.JPG "Original remote")

This project is based around an [CC2500 2.4Ghz wireless controller](http://www.ti.com/lit/ds/swrs040c/swrs040c.pdf) and an Atmega328 (ex Arduino Nano).

The choice for the CC2500 was easy because the original Ansluta Remote uses this IC, so it's definately possible to use this for controlling the lamp.

Some code is loosely based on:
https://github.com/Zohan/ArduinoCC2500Demo


Article Number Remote: 802.883.28  
Article Number Transformer: 803.007.64 / 103.201.81

##Reverse Engineering

###Sniffing SPI
I had a look inside an orginal Ansluta remote, it uses an texas instrument uC ([MSP430G2221](http://www.ti.com/lit/ds/symlink/msp430g2131.pdf)) and an CC2500.
The CC2500 communicates over an SPI bus to the uC. So I've connected a ["Bus Pirate"](http://dangerousprototypes.com/docs/Bus_Pirate_v4_design_overview) to the SPI bus of the uC and sniffed the commands and data packets to the wireless chip.
 
![alt text](https://github.com/NDBCK/Ansluta-Remote-Controller/blob/master/RemoteSnif.jpg "Sniffing SPI data")


The settings for the bus pirate were ([SPI sniffer utility](http://dangerousprototypes.com/docs/Bus_Pirate_binary_SPI_sniffer_utility)):
Clock Edge= 0, Polarity= 1 RawData= 0 

The sniffed data is in the file: ["SPI_DATA.txt"](https://github.com/NDBCK/Ansluta-Remote-Controller/blob/master/SPI_DATA.txt)

###Decoding SPI

####Button press
Every time the button is pressed a sequence is repeated 50 times: 2 data strobes are sent followed by a burst of data followed by a final strobe.

Together with the datasheet we can decode the data:

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
So the module is awakened from sleep mode, the send buffer is cleared, data is loaded into the buffer and finaly the data is transmitted. This is repeated 50 times presumable to ensure that the data is received at least one time.

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
5B [5C 7E 0x7E(0F 0x0F)5C FF 0xFF(0F 0x0F)5D ]	//	?????            0xFF  -> unknown register 
5B [5C 39 0x39(0F 0x0F)5D ]		

```
####Pairing remote and transformer
When the button on the receiver is pressed (transformer) and the button on the transmitter is pushed (and held down) the receiver learns the address of the remote. The sniffed data (complete) 

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
FF FF 0F 0F 5C 5C               //Indicates the pairing sequence
AA AA 0F 0F 5C 5C 
FF FF 0F 0F 5D 5D 

//Final strobe:
5B 5B 5C 5C 35 35 0F 0F 5D 5D 	//0x35 STX In IDLE state: Enable TX. 
```
The sniffed SPI data shows first a standard button press (because the button is pressed) and afterwards the same sequence with only one byte changed. The byte that dictates the status of the light (Off - 50% - 100%) is replaced by 0xFF.
Just like a standard button press the sequence is repeated 50 times.

###Prototype Hardware

The CC2500 is readily available as module on ebay with integrated antenna and capacitors so no RF design is needed. Only an SPI connection and power supply is needed. I chose a cheap ebay module with a real CC2500 IC (no glob top).

![alt text](https://github.com/NDBCK/Ansluta-Remote-Controller/blob/master/cc2500module.jpg "CC2500 module")

To communicate with the wireless module by SPI, I used an arduino nano because I had it laying around.
For connecting the nano to the CC2500 we have to be carefull with the different voltages. The CC2500 is 3V and the nano is 5V.
An easy (dirty) way to connect them is using a resistor (about 1K) in series with the data lines comming for the arduino (MOSI, CLK, CS). The data line comming from the CC2500 (MISO) doesn't need a resistor (a high level from CC2500 is 3V and the nano's input detects it as a high level without modifications).
The lineair voltage regulator (3.3V) on the nano is used as power source for the wireless module.

![alt text](https://github.com/NDBCK/Ansluta-Remote-Controller/blob/master/ProtoSch.jpg "Schematic of the prototype")

###Prototype Code
Now we have the necessary SPI data we and hardware we can write some basic code for the nano.
The resulting code can be found [HERE](https://github.com/NDBCK/Ansluta-Remote-Controller/blob/master/AnslutaProto/AnslutaProto.ino).
The configuration of the module is mostly the same as the original IKEA remote except the output power is changed to the maximal TX power by setting the first byte of the PATABLE with 0xFF.
The IDLE mode isn't used because after we configure the module we immediately send the data.

The SPI settings (found by looking at the graph in the datasheet "Configuration Register Write and Read Operations") are:
  * SPI Mode 0
  * MSB First
  * Max speed: 6Mhz (no need to use extra delays)

The prototype code simply sends the necessary signals to turn the lights on and off in an endless loop (for testing purposes).
Now that the prototype works it's time to make a couple of remotes to control the light.
 
It's needed to remark that the givven code uses the address that my original Ikea remote has. Somewhere within the 8 bytes that are sent in a burst will be an unique identifier. Without another remote to compare it to I don't know what the identifier is. (Probably there won't be a sort of error detection like CRC because only one byte changes when the button is pressed).
 
##Designing a PCB
Comming soon

##Different addresses
Comming soon

#include "cc2500_REG.h"
#include "cc2500_VAL.h"
#include <SPI.h>

#define CC2500_SIDLE    0x36      // Exit RX / TX, turn
#define CC2500_STX      0x35      // Enable TX. If in RX state, only enable TX if CCA passes
#define CC2500_SFTX     0x3B      // Flush the TX FIFO buffer. Only issue SFTX in IDLE or TXFIFO_UNDERFLOW states
#define CC2500_SRES     0x30


char vertragingA = 1;    // No delay is also possible
char vertragingB = 200;  // 100 doesn't work (to fast), 150 works + safety margin (empirical)

void WriteReg(char addr, char value){
  digitalWrite(SS,LOW);
  while (digitalRead(MISO) == HIGH) {
    };
  SPI.transfer(addr);
  delayMicroseconds(vertragingB);
  SPI.transfer(value);
  digitalWrite(SS,HIGH);
}

void SendStrobe(char strobe){
  digitalWrite(SS,LOW);
  
  while (digitalRead(MISO) == HIGH) {
  };
  SPI.transfer(strobe);
  digitalWrite(SS,HIGH);
  delayMicroseconds(vertragingB);
}

void init_CC2500(){
  WriteReg(REG_IOCFG2,VAL_IOCFG2);
  WriteReg(REG_IOCFG0,VAL_IOCFG0);
  WriteReg(REG_PKTLEN,VAL_PKTLEN);
  WriteReg(REG_PKTCTRL1,VAL_PKTCTRL1);
  WriteReg(REG_PKTCTRL0,VAL_PKTCTRL0);
  WriteReg(REG_ADDR,VAL_ADDR);
  WriteReg(REG_CHANNR,VAL_CHANNR);
  WriteReg(REG_FSCTRL1,VAL_FSCTRL1);
  WriteReg(REG_FSCTRL0,VAL_FSCTRL0);
  WriteReg(REG_FREQ2,VAL_FREQ2);
  WriteReg(REG_FREQ1,VAL_FREQ1);
  WriteReg(REG_FREQ0,VAL_FREQ0);
  WriteReg(REG_MDMCFG4,VAL_MDMCFG4);
  WriteReg(REG_MDMCFG3,VAL_MDMCFG3);
  WriteReg(REG_MDMCFG2,VAL_MDMCFG2);
  WriteReg(REG_MDMCFG1,VAL_MDMCFG1);
  WriteReg(REG_MDMCFG0,VAL_MDMCFG0);
  WriteReg(REG_DEVIATN,VAL_DEVIATN);
  WriteReg(REG_MCSM2,VAL_MCSM2);
  WriteReg(REG_MCSM1,VAL_MCSM1);
  WriteReg(REG_MCSM0,VAL_MCSM0);
  WriteReg(REG_FOCCFG,VAL_FOCCFG);
  WriteReg(REG_BSCFG,VAL_BSCFG);
  WriteReg(REG_AGCCTRL2,VAL_AGCCTRL2);
  WriteReg(REG_AGCCTRL1,VAL_AGCCTRL1);
  WriteReg(REG_AGCCTRL0,VAL_AGCCTRL0);
  WriteReg(REG_WOREVT1,VAL_WOREVT1);
  WriteReg(REG_WOREVT0,VAL_WOREVT0);
  WriteReg(REG_WORCTRL,VAL_WORCTRL);
  WriteReg(REG_FREND1,VAL_FREND1);
  WriteReg(REG_FREND0,VAL_FREND0);
  WriteReg(REG_FSCAL3,VAL_FSCAL3);
  WriteReg(REG_FSCAL2,VAL_FSCAL2);
  WriteReg(REG_FSCAL1,VAL_FSCAL1);
  WriteReg(REG_FSCAL0,VAL_FSCAL0);
  WriteReg(REG_RCCTRL1,VAL_RCCTRL1);
  WriteReg(REG_RCCTRL0,VAL_RCCTRL0);
  WriteReg(REG_FSTEST,VAL_FSTEST);
  WriteReg(REG_TEST2,VAL_TEST2);
  WriteReg(REG_TEST1,VAL_TEST1);
  WriteReg(REG_TEST0,VAL_TEST0);
  WriteReg(REG_DAFUQ,VAL_DAFUQ);
}

boolean UNPAIRED = false;     //Used for turning the light on and off

void setup(){
  pinMode(2, INPUT_PULLUP);
  pinMode(SS,OUTPUT);
  SPI.begin();
  SPI.beginTransaction(SPISettings(6000000, MSBFIRST, SPI_MODE0));    //Faster SPI mode, maximal speed for the CC2500 without the need for extra delays
  digitalWrite(SS,HIGH);
  SendStrobe(CC2500_SRES); //0x30 SRES Reset chip.
  init_CC2500();
  WriteReg(0x3E,0xFF);  //Maximum transmit power - write 0xFF to 0x3E (PATABLE)
}

char teller=0;
void loop(){

if(UNPAIRED){
  Serial.println(teller,HEX);
    for(byte i=0;i<50;i++){       //Send 50 times
      SendStrobe(CC2500_SIDLE);   //0x36 SIDLE Exit RX / TX, turn off frequency synthesizer and exit Wake-On-Radio mode if applicable.
      SendStrobe(CC2500_SFTX);    //0x3B SFTX Flush the TX FIFO buffer. Only issue SFTX in IDLE or TXFIFO_UNDERFLOW states.
      digitalWrite(SS,LOW);
      while (digitalRead(MISO) == HIGH) { };
      SPI.transfer(0x7F);
      delayMicroseconds(vertragingA);
      SPI.transfer(0x06);
      delayMicroseconds(vertragingA);
      SPI.transfer(0x55);
      delayMicroseconds(vertragingA);
      SPI.transfer(0x01);
      delayMicroseconds(vertragingA);
      SPI.transfer(0x3E);
      delayMicroseconds(vertragingA);
      SPI.transfer(0x94);
      delayMicroseconds(vertragingA);
      SPI.transfer(0xFF);                     //Command to pair remote and light
      delayMicroseconds(vertragingA);
      SPI.transfer(0xAA);
      delayMicroseconds(vertragingA);
      SPI.transfer(0xFF);
      digitalWrite(SS,HIGH);
      SendStrobe(CC2500_STX);                 //0x35 STX In IDLE state: Enable TX. Perform calibration first if MCSM0.FS_AUTOCAL=1. If in RX state and CCA is enabled: Only go to TX if channel is clear
      delayMicroseconds(vertragingA*10);      //Longer delay for transmitting
    }
    UNPAIRED=false;
  }
  else{
     for(byte i=0;i<50;i++){  //Send 50 times
      SendStrobe(CC2500_SIDLE); //0x36 SIDLE Exit RX / TX, turn off frequency synthesizer and exit Wake-On-Radio mode if applicable.
      SendStrobe(CC2500_SFTX);  //0x3B SFTX Flush the TX FIFO buffer. Only issue SFTX in IDLE or TXFIFO_UNDERFLOW states.
      digitalWrite(SS,LOW);
      while (digitalRead(MISO) == HIGH) { };
      SPI.transfer(0x7F);
      delayMicroseconds(vertragingA);
      SPI.transfer(0x06);
      delayMicroseconds(vertragingA);
      SPI.transfer(0x55);
      delayMicroseconds(vertragingA);
      SPI.transfer(0x01);
      delayMicroseconds(vertragingA);
      SPI.transfer(0x3E);
      delayMicroseconds(vertragingA);
      SPI.transfer(0x94);
      delayMicroseconds(vertragingA);
      SPI.transfer(0x01);               //1 = Turn light off
      delayMicroseconds(vertragingA);
      SPI.transfer(0xAA);
      delayMicroseconds(vertragingA);
      SPI.transfer(0xFF);
      digitalWrite(SS,HIGH);
      SendStrobe(CC2500_STX); //0x35 STX In IDLE state: Enable TX. Perform calibration first if MCSM0.FS_AUTOCAL=1. If in RX state and CCA is enabled: Only go to TX if channel is clear
      delayMicroseconds(vertragingA*10);
    }
    UNPAIRED=true;
    teller+=10;
  }
 
}

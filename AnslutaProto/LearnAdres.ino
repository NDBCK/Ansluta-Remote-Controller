#include "cc2500_REG.h"
#include "cc2500_VAL.h"
#include <SPI.h>

#define CC2500_SIDLE    0x36      // Exit RX / TX, turn
#define CC2500_STX      0x35      // Enable TX. If in RX state, only enable TX if CCA passes
#define CC2500_SFTX     0x3B      // Flush the TX FIFO buffer. Only issue SFTX in IDLE or TXFIFO_UNDERFLOW states
#define CC2500_SRES     0x30
#define CC2500_RXFIFO  0x3F
#define CC2500_RX      0x34      // Enable RX. Perform calibration if enabled
#define CC2500_FRX     0x3A      // Flush the RX FIFO buffer. Only issue SFRX in IDLE or RXFIFO_OVERFLOW states


char vertragingA = 1;    // No delay is also possible
char vertragingB = 200;  // 100 doesn't work (to fast), 150 works + safety margin (empirical)
int KNOP1;

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

boolean LIGHTOFF = false;     //Used for turning the light on and off

void setup(){
  pinMode(2, INPUT_PULLUP);
  pinMode(SS,OUTPUT);
Serial.begin(9600);
  SPI.begin();
  SPI.beginTransaction(SPISettings(6000000, MSBFIRST, SPI_MODE0));    //Faster SPI mode, maximal speed for the CC2500 without the need for extra delays
  digitalWrite(SS,HIGH);
  SendStrobe(CC2500_SRES); //0x30 SRES Reset chip.
  init_CC2500();
//  SendStrobe(CC2500_SPWD); //Enter power down mode    -   Not used in the prototype
  WriteReg(0x3E,0xFF);  //Maximum transmit power - write 0xFF to 0x3E (PATABLE)
}

char ReadReg(char addr){
  addr = addr + 0x80;
  digitalWrite(SS,LOW);
  while (digitalRead(MISO) == HIGH) {
    };
  char x = SPI.transfer(addr);
  delay(10);
  char y = SPI.transfer(0);
  digitalWrite(SS,HIGH);
  return y;  
}

char teller=0;
void loop(){
  
  
  SendStrobe(CC2500_RX);
  // Switch MISO to output if a packet has been received or not
  WriteReg(REG_IOCFG1,0x01);
  delay(20);
  if (digitalRead(MISO)) {      
    char PacketLength = ReadReg(CC2500_RXFIFO);
    char recvPacket[PacketLength];
    if(PacketLength <= 8) {
      Serial.println("Packet Received!");
      Serial.print("Packet Length: ");
      Serial.println(PacketLength, DEC);
      Serial.print("Data: ");
      for(char i = 1; i <= PacketLength; i++){
        recvPacket[i] = ReadReg(CC2500_RXFIFO);
        Serial.print(recvPacket[i], HEX);
        Serial.print(" ");
      }
      
      char start=0;
      while((recvPacket[start]!=0x55) && (start < PacketLength)){
        start++;
        Serial.println("R");
      }
      if(recvPacket[start+1]==0x01 && recvPacket[start+5]==0xAA){
        AdresByteA = recvPacket[start+2];
        AdresByteB = recvPacket[start+3];
      }
      Serial.print("Start: ");
      Serial.println(start,DEC);
      Serial.println(" ");
      // Print quality information
      byte rssi = ReadReg(CC2500_RXFIFO);
      byte lqi = ReadReg(CC2500_RXFIFO);
      Serial.print("RSSI: ");
      Serial.println(rssi);
      Serial.print("LQI: ");
      Serial.println(lqi);
    }
    
    // Make sure that the radio is in IDLE state before flushing the FIFO
    // (Unless RXOFF_MODE has been changed, the radio should be in IDLE state at this point) 
    SendStrobe(CC2500_SIDLE);
    // Flush RX FIFO
    SendStrobe(CC2500_FRX);   
  } 
}


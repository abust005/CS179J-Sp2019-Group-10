/*
* Additional functions created by Cody Simons csimo005@ucr.edu
*/

#include <avr/io.h>

// Modify these variables to customize the ports/pins the RF module will use
#define R_REGISTER(R) (R&0x1F)
#define W_REGISTER(R) ((R&0x1f)|(1<<5))
#define R_RX_PAYLOAD 0x61
#define W_TX_PAYLOAD 0xA0
#define FLUSH_TX 0xE1
#define FLUSH_RX 0xE2
#define REUSE_TX_PL 0xE3
#define R_RX_PL_WID 0x60
#define W_ACK_PAYLOAD(P) ((0xA8)|(P&0x07))
#define W_TX_PAYLOAD_NOACK 0xB0
#define NOP 0xFF

#define RF_DDR  DDRB // Don't change these
#define RF_PORT PORTB // Don't change these

#define CSN_PIN 0
#define CE_PIN 1
#define IRQ_PIN 2
#define MOSI_PIN 5
#define MISO_PIN 6
#define SCK_PIN 7

// Master code
// void SPI_MasterInit(void) {
// 	// Set DDRB to have MOSI, SCK, and SS as output and MISO as input
// 	RF_DDR=(1<<SCK_PIN)|(1<<MOSI_PIN)|(1<<4);
// 	// Set SPCR register to enable SPI, enable master, and use SCK frequency
// 	//   of fosc/16  (pg. 168)
// 	SPCR|=(1<<SPE)|(1<<MSTR)|(1<<SPR0);
// 	// Make sure global interrupts are enabled on SREG register (pg. 9)
// 	SREG|=0x80;
// }

// void SPI_MasterTransmit(unsigned char *txData, unsigned char *rxData, volatile uint8_t *port, unsigned char pin) {
// 	// data in SPDR will be transmitted, e.g. SPDR = cData;
// 	unsigned char bytes = sizeof(txData);
// 	*port&=~(1<<pin);
// 	for(unsigned char i=0;i<bytes;++i) {
// 			SPDR=txData[i];
// 			while(!(SPSR & (1<<SPIF)));
// 			rxData[i]=SPDR;
// 	}
// 	*port|=(1<<pin);
// }

unsigned char writeRegister(unsigned char regAddr, unsigned char regValue[], unsigned char bytes) {

	RF_PORT&=~(1<<CSN_PIN);
	SPDR=W_REGISTER(regAddr);
	while(!(SPSR & (1<<SPIF)));
	unsigned char status = SPDR;
	for(unsigned char i=0;i<bytes;++i) {
		SPDR=regValue[i];
		while(!(SPSR & (1<<SPIF)));
	}
	RF_PORT|=(1<<CSN_PIN);
	return status;
}

unsigned char readRegister(unsigned char regAddr, unsigned char regValue[], unsigned char bytes) {
	RF_PORT&=~(1<<CSN_PIN);
	SPDR=R_REGISTER(regAddr);
	while(!(SPSR & (1<<SPIF)));
	unsigned char status = SPDR;
	for(unsigned char i=0;i<bytes;++i) {
		SPDR=regValue[i];
		while(!(SPSR & (1<<SPIF)));
		regValue[i]=SPDR;
	}
	RF_PORT|=(1<<CSN_PIN);
	return status;
}

unsigned char writePayloadTx(unsigned char msg[], unsigned char bytes) {
	RF_PORT&=~(1<<CSN_PIN);
	SPDR=W_TX_PAYLOAD;
	while(!(SPSR & (1<<SPIF)));
	unsigned char status = SPDR;
	for(unsigned char i=0;i<bytes;++i) {
		SPDR=msg[i];
		while(!(SPSR & (1<<SPIF)));
	}
	RF_PORT|=(1<<CSN_PIN);
	return status;
}

unsigned char writePayloadTxNoack(unsigned char msg[], unsigned char bytes) {
	RF_PORT&=~(1<<CSN_PIN);
	SPDR=W_TX_PAYLOAD_NOACK;
	while(!(SPSR & (1<<SPIF)));
	unsigned char status = SPDR;
	for(unsigned char i=0;i<bytes;++i) {
		SPDR=msg[i];
		while(!(SPSR & (1<<SPIF)));
	}
	RF_PORT|=(1<<CSN_PIN);
	return status;
}

unsigned char readPayloadRx(unsigned char msg[], unsigned char bytes) {
	RF_PORT&=~(1<<CSN_PIN);
	SPDR=R_RX_PAYLOAD;
	while(!(SPSR & (1<<SPIF)));
	unsigned char status = SPDR;
	for(unsigned char i=0;i<bytes;++i) {
		SPDR=msg[i];
		while(!(SPSR & (1<<SPIF)));
		msg[i]=SPDR;
	}
	RF_PORT|=(1<<CSN_PIN);
	return status;
}

unsigned char flushTx() {
	unsigned char status;
	RF_PORT&=~(1<<CSN_PIN);
	SPDR=FLUSH_TX;
	while(!(SPSR & (1<<SPIF)));
	status=SPDR;
	RF_PORT|=(1<<CSN_PIN);
	return status;
}

unsigned char flushRx() {
	unsigned char status;
	RF_PORT&=~(1<<CSN_PIN);
	SPDR=FLUSH_RX;
	while(!(SPSR & (1<<SPIF)));
	status=SPDR;
	RF_PORT|=(1<<CSN_PIN);
	return status;
}

void Radio_TxInit() {
	SPI_MasterInit();
	RF_DDR|=(1<<CSN_PIN)|(1<<CE_PIN);
	RF_PORT|=(1<<IRQ_PIN)|(1<<CSN_PIN);
	delay_ms(5); //settling time specified in the data sheet
	unsigned char oneByte[1]={0x72};
	unsigned char threeByte[3]={0x18, 0x04, 0x96};
	writeRegister(0x00, oneByte, 1); //set config reg to power up into tx mode
	oneByte[0]=0x01;
	writeRegister(0x03, oneByte, 1); //set address length to 3 bytes
	writeRegister(0x10, threeByte, 3); //set address of transmit to random number, in this case my birthday
	oneByte[0]=76;
	writeRegister(0x05, oneByte, 1); //set to channel 76
	oneByte[0]=0x00;
	writeRegister(0x01, oneByte, 1); //turn off Auto Acknowledge, one way communication okay if a few packets get dropped
	writeRegister(0x04, oneByte, 1); //turn off retransmit, not ideal for remote control
	oneByte[0]=0x01;
	writeRegister(0x1D, oneByte, 1); //enable W_TX_PAYLOAD_NOACK
	flushTx();
	RF_PORT|=(1<<CE_PIN);
}

void Radio_TxTransmitt(unsigned char msg[], unsigned char bytes) {
	writePayloadTxNoack(msg, bytes);
}

void Radio_RxInit() {
	SPI_MasterInit();
	RF_DDR|=(1<<CSN_PIN)|(1<<CE_PIN);
	RF_PORT|=(1<<IRQ_PIN)|(1<<CSN_PIN);
	delay_ms(5);
	unsigned char oneByte[1]={0x33};
	unsigned char threeByte[3]={0x18, 0x04, 0x96};
	writeRegister(0x00, oneByte, 1); //set config reg to power up into rx mode
	oneByte[0]=0x01;
	writeRegister(0x03, oneByte, 1); //set address length to 3 bytes
	writeRegister(0x0A, threeByte, 3); //set address of pipe 0 to random number, in this case my birthday
	oneByte[0]=76;
	writeRegister(0x05, oneByte, 1); //set channel to 76
	oneByte[0]=0x00;
	writeRegister(0x01, oneByte, 1); //turn off auto acknowledge
	oneByte[0]=5;
	writeRegister(0x11, oneByte, 1); //set packet length for pipe 0
	flushRx();
	RF_PORT|=(1<<CE_PIN);
}

void Radio_RxReset() {
	RF_PORT&=~0x08;
	delay_ms(10);
	RF_PORT|=0x08;
	Radio_RxInit();
}

unsigned char Radio_RxReady() {
	unsigned char FIFOStatus[1];
	readRegister(0x17, FIFOStatus, 1);
	if((FIFOStatus[0]&0x01)==0) {
		return(1);
	}
	return(0);
}

void Radio_RxRead(unsigned char data[], unsigned char bytes) {
	readPayloadRx(data, bytes);
}

enum radioStates{radioInit, radioRead, radioLost};

unsigned char droppedPackets;
unsigned char recievedPackets;

int radioTick(int state) {
  switch(state) {
    case radioInit: {
      droppedPackets=0;
      recievedPackets=0;
      Radio_RxInit();
      //motorInit();
      // servoInit();
      return(radioRead);
      break;
    }
    case radioRead: {
      if(Radio_RxReady()) {
        droppedPackets=0;
        unsigned char state[5];
        Radio_RxRead(state, 5);
        if(recievedPackets==4) {
          //unsigned short servoPosition=(state[0]<<8)|state[1];
          //servoSetPos(servoPosition);
          //unsigned short motorSpeed=(state[2]<<8)|state[3];
  	     //setMotorSpeed(motorSpeed);
         //setLights(state[4]);
		     }
         else {
           recievedPackets++;
	       }
        return(radioRead);
      }
      else {
        droppedPackets++;
        if(droppedPackets==40) {
	        //setMotorSpeed(512);
          //servoSetPos(512);
          //setLights(0x03);
	        droppedPackets=0;
	        recievedPackets=0;
          return(radioLost);
        }
        return(radioRead);
      }
      break;
    }
    case radioLost: {
      if(Radio_RxReady()) {
        droppedPackets=0;
        unsigned char state[5];
        Radio_RxRead(state, 5);
		      if(recievedPackets==4) {
      			//unsigned short servoPosition=(state[0]<<8)|state[1];
      			//servoSetPos(servoPosition);
      			//PORTA=servoPosition>>2;
      			//unsigned short motorSpeed=(state[2]<<8)|state[3];
      			//setMotorSpeed(motorSpeed);
      			//setLights(state[4]);
            return(radioRead);
	        }
          else {
		         recievedPackets++;
          }
          return(radioLost);
      }
      else {
	      droppedPackets++;
	      if(droppedPackets==40) {
          Radio_RxReset();
          droppedPackets=0;
	      }
	      return(radioLost);
      }
      break;
    }
    default: {
      return(radioInit);
      break;
    }
  }
}

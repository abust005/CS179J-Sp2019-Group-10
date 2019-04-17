#ifndef SPI_H
#define SPI_H

#include <avr/io.h>
#include <avr/interrupt.h>

unsigned char receivedData = 0;

// Master code
void SPI_MasterInit(void) {
	// Set DDRB to have MOSI, SCK, and SS as output and MISO as input
	DDRB |= (1<<DDB4)|(1<<DDB5)|(0<<DDB6)|(1<<DDB7);
	// Set SPCR register to enable SPI, enable master, and use SCK frequency of fosc/16  (pg. 168)
	SPCR |= (1<<SPE)|(1<<MSTR)|(1<<SPR0);
	// Make sure global interrupts are enabled on SREG register (pg. 9)
	SREG |= 0x80;
}

void SPI_MasterTransmit(unsigned char cData) {
	// data in SPDR will be transmitted, e.g. SPDR = cData;
	// set SS low
	DDRB |= (0<<DDB4);
	SPDR = 0x00;
	while(!(SPSR & (1<<SPIF))) { // wait for transmission to complete
		;
	}
	SPDR = cData;
	while(!(SPSR & (1<<SPIF))) { // wait for transmission to complete
		;
	}
	DDRB |= (1<<DDB4);
	// set SS high
}

// Servant code
void SPI_ServantInit(void) {
	// set DDRB to have MISO line as output and MOSI, SCK, and SS as input
	DDRB |= (0<<DDB4)|(0<<DDB5)|(1<<DDB6)|(0<<DDB7);
	// set SPR register to enable SPI and enable SPI interrupt (pg. 168)
	SPCR |= (1<<SPIE)|(1<<SPE);
	// make sure global interrupts are enabled on SREG register (pg. 9)
	SREG |= 0x80;
}

ISR(SPI_STC_vect) { // this is enabled in with the SPCR register’s “SPI Interrupt Enable”
	// SPDR contains the received data, e.g. unsigned char receivedData = SPDR;
	receivedData = SPDR;
}


#endif
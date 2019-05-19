/*
 * MADLADS_receiverDemo.c
 *
 *
 * Created: 5/6/2019 4:20:00 PM
 * Authors : Jonathan "Cuomo" Woolf
 *			 Joshua Riley
 *			 Colton Vosburg
 *			 Adriel Bustamante will never read this sentence
 */ 

#define timerPeriod 1
#define tasksNum 3

#include <avr/io.h>
#include <avr/interrupt.h>
#include "scheduler.h"
#include "nrf24l01.h"

unsigned char temp, counter = 0x00; // SPI variables 

unsigned char payload[1] = {0x00};
unsigned char droneSignal = 0x00; // bits 0-1 are up/down, 2-4 are left/right/forward/reverse, 5 is button2, 6 is claw, 7 is parity bit
unsigned char droneXY = 0; 
unsigned char droneZ = 0;
unsigned char b1 = 0;
unsigned char b2 = 0; 
unsigned char parity = 0;
unsigned char column_val = 0x08; // Sets the pattern displayed on columns
unsigned char column_sel = 0x10; // Grounds column to display pattern

// SPI
enum spi_states {receive, noSignal} spi_state;

unsigned char droppedPackets;

int spi_servant(int spi_state)
{
	switch(spi_state)
	{
// 			counter = 0; // Counts the number of bits set to 1
// 			for(int i = 0; i < 6; i++)
// 			{
// 				temp = (droneSignal >> i) &  0x01; // Right shift by i and clear unwanted bits
// 				if(temp == 0x01){ counter++;} // If the bit we are checking is set to 1, update the counter
// 			}
// 			if((counter % 2) == 0) // Checks whether or not we have an even number of bits
// 			{
// 				droneSignal = droneSignal & 0x7F; // Set parity bit to 0 for even number of 1s
// 			}
// 			else
// 			{
// 				droneSignal = droneSignal | 0x80; // Set parity bit to 1 for odd number of 1s
// 			}
		case receive:
			if(Radio_RxReady())
			{
				droppedPackets = 0;
				Radio_RxRead(payload, 1);
				droneSignal = payload[0];
				PORTA = droneSignal;
				
				droneZ = droneSignal & 0x03;
				droneXY = (droneSignal >> 2) & 0x07;
				b1 = (droneSignal >> 6) & 0x01;
				b2 = (droneSignal >> 5) & 0x01;
				parity = (droneSignal >> 7) & 0x01;
			}
			else
			{
				droppedPackets++;
				if(droppedPackets == 40)
				{
					droneSignal = 0x40; // Drone will hover, but claw will be closed incase it has a package
					droppedPackets = 0;
					spi_state = noSignal;
					break;
				}	
			}
			spi_state = receive;
			break;
		case noSignal:
			if(Radio_RxReady())
			{
				droppedPackets = 0;
				Radio_RxRead(payload, 1);
				droneSignal = payload[0];
				PORTA = droneSignal;
				
				droneZ = droneSignal & 0x03;
				droneXY = (droneSignal >> 2) & 0x07;
				b1 = (droneSignal >> 6) & 0x01;
				b2 = (droneSignal >> 5) & 0x01;
				parity = (droneSignal >> 7) & 0x01;
				spi_state = receive;
				break;
			}
			else
			{
				droppedPackets++;
				if(droppedPackets == 40)
				{
					Radio_RxReset();
					droppedPackets = 0;
				}
			}
			spi_state = noSignal;
			break;
		default:
			droppedPackets = 0;
			Radio_RxInit();
			spi_state = receive;
			break;
	}
	return spi_state;
}

// Joysticks are actually wired sideways so left/right and up/down are switched but the states are labeled correctly for their observed actions
enum movement_states {left_right, up_down} movement_state;
int TickFct_movement(int movement_state)
{
	return -1;
	switch(movement_state)
	{
		case left_right: // Right joystick controls left and right movements
			if(droneXY == 1 || droneXY == 3) // Joystick is being tilted right (it is pointed the wrong way so if wired differently this code must be flipped
			{
				if(column_val == 0x01) { column_val = 0x80;} // Move left a row
				else if (column_val != 0x01) { column_val = (column_val >> 1);} // Obviously a right shift must occur
			}
			if(droneXY == 4 || droneXY == 6) // Joystick is being tilted left
			{
				if(column_val == 0x80) { column_val = 0x01;} // Move right a row
				else if (column_val != 0x80) { column_val = (column_val << 1);} // Obviously a left shift must occur
			}
			movement_state = up_down;
			break;
		case up_down: // Left joystick controls forward and reverse movements
			if(droneXY == 2 || droneXY == 3 || droneXY == 6) // Joystick is being tilted up
			{
				if(column_sel == 0x01) { column_sel = 0x80;} // Move up a column
				else if (column_sel != 0x01) { column_sel = (column_sel >> 1);} // Obviously a right shift must occur
			}
			if(droneXY == 5) // Joystick is being tilted down
			{
				if(column_sel == 0x80) { column_sel = 0x01;} // Move down a column
				else if (column_sel != 0x80) { column_sel = (column_sel << 1);} // Obviously a left shift must occur
			}
			movement_state = left_right; // Return to left right state
			break;
		default:
			movement_state = left_right;
			break;
	}
	return movement_state;
}

// Test harness for LED matrix to make sure all user inputs are read in correctly
enum LED_states {synch} LED_state;
int TickFct_LEDState(int state)
{
	return -1;
	switch(LED_state)
	{
		case synch:
			PORTB = ~column_val;
			PORTA = column_sel;
			LED_state = synch;
			break;
		default:
			LED_state = synch;
			break;
	}
	return LED_state;
}

int main(void)
{
	DDRA = 0xFF; PORTA = 0x00; // Output to column val
	DDRD = 0xFF; PORTD = 0x00; // Output to column sel
	// Input from RF Receiver will be received from RX1 
	
	TimerSet(timerPeriod);
	TimerOn();
	
	unsigned char i = 0;
	tasks[i].state = -1;
	tasks[i].period = 50;
	tasks[i].elapsedTime = 0;
	tasks[i].TickFct = &spi_servant;
	i++;
	tasks[i].state = -1;
	tasks[i].period = 300;
	tasks[i].elapsedTime = 0;
	tasks[i].TickFct = &TickFct_movement;
	i++;
	tasks[i].state = -1;
	tasks[i].period = 50;
	tasks[i].elapsedTime = 0;
	tasks[i].TickFct = &TickFct_LEDState;
	
	while (1)
	{
	}
}
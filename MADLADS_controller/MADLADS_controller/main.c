/*
 * MADLADS_controller.c
 *
 * WILL NOT CURRENTLY COMPILE AS IT IS BEING ADJUSTED TO MEET CURRENT NEEDS
 *
 * Created: 4/16/2019 7:30:36 PM
 * Author : Jonathan "Cuomo" Woolf
 */ 

#define timerPeriod 1
#define tasksNum 4

#include <avr/io.h>
#include <avr/interrupt.h>
#include "scheduler.h"
#include "spi.h"

unsigned char temp, counter = 0x00; // SPI variables 

unsigned char droneSignal = 0x00; // bits 0-1 are up/down, 2-4 are left/right/forward/reverse, 5 is claw, 6 is button2, 7 is parity bit

//unsigned short joystick, joystick2, joystick3 = 0x0000; // Variables to store ADC values of joysticks
unsigned short joystick = 0;
unsigned short joystick2 = 0;
unsigned short joystick3 = 0;

// Code provided by UCR for ADC

// Pins on PORTA are used as input for A2D conversion
// The Default Channel is 0 (PA0)
// The value of pinNum determines the pin on PORTA used for A2D conversion
// Valid values range between 0 and 7, where the value represents the desired pin for A2D conversion
void Set_A2D_Pin(unsigned char pinNum)
{
	ADMUX = (pinNum <= 0x07) ? pinNum : ADMUX;
	// Allow channel to stabilize
	static unsigned char i = 0;
	for(i = 0; i < 15; ++i){asm("nop");}
}

// ADEN: Enables analog-to-digital conversion
// ADSC: Starts analog-to-digital conversion
// ADATE: Enables auto-triggering, allowing for constant
//		  analog to digital conversions.
void A2D_init() { ADCSRA |= (1 << ADEN) | (1 << ADSC) | (1 << ADATE);}

void convert(){
	ADCSRA |= (1 << ADSC); // start ADC conversion
	while ( !(ADCSRA & (1<<ADIF))); // wait till ADC conversion
}

// End of code provided by UCR for ADC

// Joysticks are actually wired sideways so left/right and forward/reverse are switched but the states are labeled correctly for their observed actions
/*
000 - standby
001 - right
010 - forward
011 - forward/right
100 - left
101 - reverse
110 - forward/left
111 - N/A
*/
enum movement_states {left_right, forward_reverse} movement_state;

int TickFct_movement(int movement_state)
{
	return movement_state; // FIXME: For debugging purposes
	switch(movement_state)
	{
		case left_right: // Right joystick controls left and right movements
			Set_A2D_Pin(0x02); // Sets analog signal to the left/right axis of the right joystick
			convert();
			joystick = (unsigned short)ADC; // Read ADC value into joystick variable
			droneSignal = (droneSignal & 0xE3); // L/R/F/R set to 000 for hover
			if(joystick > 373) // Joystick is being tilted left
			{
				droneSignal |= 0x10; // L/R set to 100 for left 
			}
			else if(joystick < 127) // Joystick is being tilted right
			{
				droneSignal |= 0x04; // L/R set to 001 for right
			}
			movement_state = forward_reverse;
			break;
		case forward_reverse: // Left joystick controls forward and reverse movements
			Set_A2D_Pin(0x03); // Sets analog signal to the left/right axis of the right joystick
			convert();
			joystick2 = (unsigned short)ADC; // Read ADC value into joystick2 variable
			if(joystick2 > 496) // Joystick is being tilted up
			{
				droneSignal |= 0x08; // F/R set to 1 for forward
			}
			else if(joystick2 < 169) // Joystick is being tilted down
			{
				droneSignal = (droneSignal & 0xF7) | 0x14; // F/R set to 101 for reverse
			}
			movement_state = left_right; // Return to left right state
			break;
		default:
			movement_state = left_right;
			break;
	}
	return movement_state;
}

// Joysticks are actually wired sideways so left/right and up/down are switched but the states are labeled correctly for their observed actions
/*
00 - standby
01 - down
10 - up
11 - N/A
*/
enum altitude_states {up_down} altitude_state;

int TickFct_altitude(int altitude_state)
{
	switch(altitude_state)
	{
		case up_down: // Right joystick controls up and down movements
			Set_A2D_Pin(0x00); // Sets analog signal to the left/right axis of the right joystick
			convert();
			joystick3 = (unsigned short)ADC; // Read ADC value into joystick variable
			droneSignal = (droneSignal & 0xFC); // Up/Down set to 00 for maintain altitude
			if(joystick3 > 495) // Joystick is being tilted up
			{
				droneSignal |= 0x02; // Up/Down set to 10 for up
			}
			else if(joystick3 < 167 && temp != 0) // Joystick is being tilted left
			{
				droneSignal |= 0x01; // Up/Down set to 01 for down
			}
			break;
		default:
			altitude_state = up_down;
			break;
	}
	return altitude_state;
}

enum button_states {buttons} button_state;

int TickFct_button(int button_state)
{
	// ----------vvvvvvvvvvvvvvv-----------------
	// FIXME: For Debugging to make sure of no infinite loop; remove section when done
	switch (button_state)
	{
		case buttons: // Toggles light off [PD6]
			droneSignal &= 0xBF; 
			button_state = -1;
			break;
		default: // Toggles light on [PD6]
			droneSignal |= 0x40; 
			button_state = buttons;
			break;
	}
	return button_state;
	// FIXME: End of debugging section
	// ----------^^^^^^^^^^^^^^^-----------------
	
	switch(button_state)
	{
		case buttons: // Right joystick controls up and down movements
			droneSignal = (droneSignal & 0x9F); // buttons set to 00 for unused
			button_state = buttons;
			break;
		default:
			button_state = buttons;
			break;
	}
	return button_state;
}

// SPI
enum uart_states {wait, send} uart_state;
	
int spi_master(int uart_state)
{
	return uart_state; // FIXME: For debugging purposes
	switch(uart_state)
	{
		case wait:
			counter = 0; // Counts the number of bits set to 1
			for(int i = 0; i < 7; i++)
			{
				temp = (droneSignal >> i) &  0x01; // Right shift by i and clear unwanted bits
				if(temp == 0x01){ counter++;} // If the bit we are checking is set to 1, update the counter
			}
			if((counter % 2) == 0) // Checks whether or not we have an even number of bits
			{
				droneSignal = droneSignal & 0x7F; // Set parity bit to 0 for even number of 1s
			}
			else
			{
				droneSignal = droneSignal | 0x80; // Set parity bit to 1 for odd number of 1s
			}
			uart_state = send;
			break;
		case send:
			SPI_MasterTransmit(droneSignal); // Transmit droneSignal over RF using SPI protocol
			uart_state = wait;
			break;
		default:
			uart_state = wait;
			break;
	}
	return uart_state;
}


int main(void)
{
	DDRA = 0x00; PORTA = 0xFF; // Input from ADC
	DDRD = 0xFF; PORTD = 0x00; // Output for testing
	//DDRC = 0x00; PORTC = 0xFF; // Input from button 2
	//DDRD = 0x00; PORTD = 0xFF; // Input from button 1 (claw)
	// Output from RF transmitter will be sent from MOSI do not initialize DDRB 

	A2D_init();
	//SPI_MasterInit();
	TimerSet(timerPeriod);
	TimerOn();

	unsigned char i = 0;
	tasks[i].state = -1;
	tasks[i].period = 50;
	tasks[i].elapsedTime = 0;
	tasks[i].TickFct = &TickFct_movement;
	i++;
	tasks[i].state = -1;
	tasks[i].period = 50;
	tasks[i].elapsedTime = 0;
	tasks[i].TickFct = &TickFct_altitude;
	i++;
	tasks[i].state = -1;
	//tasks[i].period = 50;
	tasks[i].period = 500; // FIXME: For Debugging purposes only
	tasks[i].elapsedTime = 0;
	tasks[i].TickFct = &TickFct_button;
	i++;
	tasks[i].state = -1;
	tasks[i].period = 25;
	tasks[i].elapsedTime = 0;
	tasks[i].TickFct = &spi_master;

	while (1)
	{
		PORTD = droneSignal; // Test that we are generating the correct signal DELETE ONCE DONE
	}
}


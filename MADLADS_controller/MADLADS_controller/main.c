/*
 * MADLADS_controller.c
 *
 * WILL NOT CURRENTLY COMPILE AS IT IS BEING ADJUSTED TO MEET CURRENT NEEDS
 *
 * Created: 4/16/2019 7:30:36 PM
 * Author : Jonathan "Cuomo" Woolf
 */ 

#define timerPeriod 1
#define tasksNum 3

#include <avr/io.h>
#include <avr/interrupt.h>
#include "scheduler.h"
#include "usart_ATmega1284.h" // Remove once SPI compatible 
#include "spi.h"

unsigned char temp, s_data, counter = 0x00; // SPI variables 

unsigned char droneXYAxis = 0; // 00 - stopped, 01 - creep, 10 - medium, 11 - fast
unsigned char droneZAxis = 0; // 10 - left, 00 - straight, 01 - right
unsigned char claw = 0; // 0 - open, 1 - shut-them
unsigned char b2 = 0; // button usage still undecided
unsigned char droneSignal = 0x00; // bits 0-1 are up/down, 2-4 are left/right/forward/reverse, 5 is claw, 6 is button2, 7 is parity bit

unsigned short joystick, joystick2, joystick3; // Variables to store ADC values of joysticks

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
enum movement_states {left_right, forward_reverse} movement_state;

int TickFct_movement(int movement_state)
{
	switch(movement_state)
	{
		case left_right: // Right joystick controls left and right movements
			Set_A2D_Pin(0x00); // Sets analog signal to the left/right axis of the right joystick
			convert();
			joystick = ADC; // Read ADC value into joystick variable
			droneSignal = (droneSignal & 0xE3); // L/R/F/R set to 000 for hover
			if(joystick > 700) // Joystick is being tilted left
			{
				droneSignal = ((droneSignal & 0xE7) | 0x08); // L/R set to 10 for left NEEDS UPDATING
			}
			else if(joystick < 350) // Joystick is being tilted right
			{
				droneSignal = ((droneSignal & 0xF3) | 0x04); // L/R set to 01 for right NEEDS UPDATING
			}
			movement_state = forward_reverse;
			break;
		case forward_reverse: // Left joystick controls forward and reverse movements
			Set_A2D_Pin(0x01); // Sets analog signal to the left/right axis of the right joystick
			convert();
			joystick2 = ADC; // Read ADC value into joystick2 variable
			if(joystick2 > 700) // Joystick is being tilted up
			{
				droneSignal = (droneSignal & 0xEF); // F/R set to 0 for forward
			}
			else if(joystick2 < 350) // Joystick is being tilted down
			{
				droneSignal = (droneSignal | 0x10); // F/R set to 1 for reverse
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
enum altitude_states {up_down} altitude_state;

int TickFct_altitude(int altitude_state)
{
	switch(altitude_state)
	{
		case up_down: // Right joystick controls up and down movements
			Set_A2D_Pin(0x03); // Sets analog signal to the left/right axis of the right joystick
			convert();
			joystick3 = ADC; // Read ADC value into joystick variable
			droneSignal = (droneSignal & 0xFC); // Up/Down set to 00 for maintain altitude
			if(joystick > 700) // Joystick is being tilted up
			{
				droneSignal = ((droneSignal & 0xE7) | 0x02); // Up/Down set to 10 for up
			}
			else if(joystick < 350) // Joystick is being tilted left
			{
				droneSignal = ((droneSignal & 0xF3) | 0x04); // Up/Down set to 01 for down
			}
			movement_state = up_down;
			break;
		default:
			movement_state = up_down;
			break;
	}
	return altitude_state;
}
//Will be replaced with SPI
enum uart_state{uart_start, send};
int uart_tick(int state)
{
	switch(state)
	{
		case uart_start:
		counter = 0; // Reset counter to 0 after all three signals have been sent
		if(USART_HasTransmitted(1))
		{
			temp = droneSignal; // Updates s_data if previous value has been transmitted
		}
		state = send;
		break;
		case send:
		state = uart_start;
		if(USART_IsSendReady(1) && counter < 3)  // Send three copies of the same signal for redundancy
		{
			if(counter == 0) {s_data = (temp | 0x80);} // Marker denoting first duplicate signal
			if(counter == 1) {s_data = (temp | 0x40);} // Marker denoting second duplicate signal
			if(counter == 2) {s_data = (temp | 0x20);} // Marker denoting third duplicate signal
			USART_Send(s_data, 1); // Send s_data
			counter++; // Updates counter
			state = send;
		}
		break;
		default:
		state = uart_start;
		break;
	}
	return state;
}


int main(void)
{
	DDRA = 0x00; PORTA = 0xFF; // Input
	DDRB = 0xFF; PORTB = 0x00; // Output to column sel
	// Output from RF transmitter will be sent from TX1 DO NOT INITIALIZE DDRD / PORTD as it will not send

	TimerSet(timerPeriod);
	TimerOn();
	A2D_init();
	initUSART(1);

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
	tasks[i].state = uart_start;
	tasks[i].period = 25;
	tasks[i].elapsedTime = 0;
	tasks[i].TickFct = &uart_tick;

	while (1)
	{
	}
}


/*
 * MADLADS_controller.c
 *
 *
 * Created: 4/16/2019 7:30:36 PM
 * Authors : Jonathan "Cuomo" Woolf
 *			 Joshua Riley
 *			 Colton Vosburg
 *			 Adriel Bustamante 
 */ 

#define timerPeriod 1
#define tasksNum 3 // Number of tasks

#include <avr/io.h>
#include <avr/interrupt.h>
#include "scheduler.h"
//#include "spi.h"
#include "nrf24l01.h"
// === Inputs and Variables ===
#define topButton (~PINC & 0x01)
#define bottomButton (~PINC & 0x02)

unsigned char temp, counter = 0x00; // SPI variables 

unsigned char droneSignal = 0x00; // bits 0-1 are up/down, 2-4 are left/right/forward/reverse, 5 is claw, 6 is button2, 7 is parity bit

unsigned char payload[1] = {0x00};

unsigned short joystick, joystick2, joystick3 = 0x0000; // Variables to store ADC values of joysticks

unsigned char upFlag, downFlag, clawFlag = 0x00; // Flags for bitmasking logic

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
LEFT JOYSTICK BITS - ***X XX**
000 - standby
001 - right
010 - forward
011 - forward/right
100 - left
101 - reverse
110 - forward/left
111 - N/A

RIGHT JOYSTICK BITS - **** **XX
00 - standby
01 - down
10 - up
11 - N/A
*/
enum movement_states {left_right, forward_reverse, up_down} movement_state;

int TickFct_movement(int movement_state)
{
	switch(movement_state)
	{
		case left_right: // Left joystick controls left and right movements
			Set_A2D_Pin(0x02); // Sets analog signal to the left/right axis of the left joystick
			convert();
			joystick = ADC; // Read ADC value into joystick variable
			//droneSignal = (droneSignal & 0xE3); // L/R/F/R set to 000 for hover
			if(joystick > 700 && downFlag == 0) // Joystick is being tilted left
			{
				if(upFlag == 1)
				{
					droneSignal = (droneSignal & 0xFB) | 0x10;
				}
				else
				{
					droneSignal = ((droneSignal & 0xE3) | 0x10); // L/R set to 100 for left 
				} 
			}
			else if(joystick < 350 && downFlag == 0) // Joystick is being tilted right
			{
				if(upFlag == 1)
				{
					droneSignal = (droneSignal & 0xEF) | 0x04;
				}
				else
				{
					droneSignal = ((droneSignal & 0xE3) | 0x04); // L/R set to 001 for right
				}
			}
			else // Joysticks are not being tilted
			{
				if (downFlag == 0)
				{
					droneSignal = (droneSignal & 0xEB); // Bits anded with 010 to clear all but forward
				}
			}
			movement_state = forward_reverse; // Return to the forward reverse state
			break;
		case forward_reverse: // Left joystick controls forward and reverse movements
			Set_A2D_Pin(0x03); // Sets analog signal to the left/right axis of the right joystick
			convert();
			joystick2 = ADC; // Read ADC value into joystick2 variable
			if(joystick2 > 700) // Joystick is being tilted up
			{
				droneSignal = (droneSignal | 0x08); // F/R set to 1 for forward
				upFlag = 1;
				downFlag = 0;
			}
			else if(joystick2 < 350) // Joystick is being tilted down
			{
				droneSignal = (droneSignal & 0xF7) | 0x14; // F/R set to 101 for reverse
				upFlag = 0;
				downFlag = 1;
			}
			else // Joystick is not being tilted
			{
				droneSignal = droneSignal & 0xF7; // Bits anded with 101 to clear forward
				upFlag = 0;
				downFlag = 0;
			}
			movement_state = up_down; // Return to up down state
			break;
		case up_down: // Right joystick controls up and down movements
			Set_A2D_Pin(0x00); // Sets analog signal to the left/right axis of the right joystick
			convert();
			joystick3 = ADC; // Read ADC value into joystick variable
			droneSignal = (droneSignal & 0xFC); // Up/Down set to 00 for maintain altitude
			if(joystick3 > 700) // Joystick is being tilted up
			{
				droneSignal |= 0x01; // Up/Down set to 10 for down
			}
			else if(joystick3 < 350) // Joystick is being tilted left
			{
				droneSignal |= 0x02; // Up/Down set to 01 for up
			}
			movement_state = left_right; // Return to left right state
			break;
		default:
			movement_state = left_right; // Return to left right state
			break;
	}
	return movement_state;
}

/* Button one controls the claw while button two is currently misc. but will likely down the drone
00 - neither b
10 - claw is engaged until the first button is pressed again
X1 - Drop it like its hot
*/
enum button_states {buttons} button_state;

int TickFct_button(int button_state)
{	
	switch(button_state)
	{
		case buttons: // Right joystick controls up and down movements
			if(topButton && !bottomButton)
			{
				if(clawFlag == 0) // Engage the claw 
				{
					clawFlag = 1;
					droneSignal = (droneSignal & 0x9F) | 0x40; // Buttons set to 10
				}
				else // Disengage the claw
				{
					clawFlag = 0;
					droneSignal = (droneSignal & 0x9F); // Buttons set to 00
				}
			}
			else if(bottomButton) // Button must be held down to prevent accidental drone loss
			{
				droneSignal = droneSignal | 0x20;
			}
			else
			{
				droneSignal = (droneSignal & 0xDF); // buttons set to 00 for unused
			}
			button_state = buttons;
			break;
		default:
			button_state = buttons;
			break;
	}
	return button_state;
}

// SPI
enum spi_states {wait, send} spi_state;
	
int spi_master(int spi_state)
{
	switch(spi_state)
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
			spi_state = send;
			break;
		case send:
			payload[0] = droneSignal; // Update payload with the droneSignal
			Radio_TxTransmit(payload, 1); // 1 specifies the number of bytes to transmit. 32 is max
			spi_state = wait;
			break;
		default:
			Radio_TxInit();
			spi_state = wait;
			break;
	}
	return spi_state;
}


int main(void)
{
	DDRA = 0x00; PORTA = 0xFF; // Input from ADC
	DDRC = 0x00; PORTC = 0xFF; // Input from buttons
	// DDRD = 0xFF; PORTD = 0x00; // Output for testing
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
 	tasks[i].TickFct = &TickFct_button;
	i++;
	tasks[i].state = -1;
	tasks[i].period = 25;
	tasks[i].elapsedTime = 0;
	tasks[i].TickFct = &spi_master;

	while (1)
	{
		//PORTD = droneSignal; // Test that we are generating the correct signal DELETE ONCE DONE
	}
}


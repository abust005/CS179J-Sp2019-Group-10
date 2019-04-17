// The following is sample code for how to create a main.c that works with scheduler.h
// This program will cause D0 to blink on and off every 1500ms. D4D5D6 will rotate every 500ms.
// This code will work with the ATMega1284 or ATMega32

#include <avr/io.h>
#include <scheduler.h>

volatile unsigned char PORTD_TEMP = 0x00;

enum BL_States { SM1_On, SM1_Off };
int TickFct_BlinkLED(int state) {
	switch(state) { // Transitions
		case -1: // Initial transition
			PORTD_TEMP = 0x00; // Initialization behavior
			state = SM1_On;
		break;
		case SM1_On:
			state = SM1_Off;
		break;
		case SM1_Off:
			state = SM1_On;
		break;
		default:
			state = -1;
	}

	switch(state) { // State actions
		case SM1_On:
			PORTD_TEMP |= 0x01;
		break;
		case SM1_Off:
			PORTD_TEMP &= 0xFE;
		break;
		default:
		break;
	}
	
	PORTD = PORTD_TEMP;
	return state;
}

enum TL_States { SM2_Cyc1, SM2_Cyc2, SM2_Cyc3 };
int TickFct_ThreeLEDs(int state) {
	switch(state) { // Transitions
		case -1: // Initial transition
			state = SM2_Cyc1;
		break;
		case SM2_Cyc1:
			state = SM2_Cyc2;
		break;
		case SM2_Cyc2:
			state = SM2_Cyc3;
		break;
		case SM2_Cyc3:
			state = SM2_Cyc1;
		break;
		default:
			state = -1;
	}

	switch(state) { // State actions
		case SM2_Cyc1:
			PORTD_TEMP = (PORTD_TEMP & 0x0F) | 0x10;
		break;
		case SM2_Cyc2:
			PORTD_TEMP = (PORTD_TEMP & 0x0F) | 0x20;
		break;
		case SM2_Cyc3:
			PORTD_TEMP = (PORTD_TEMP & 0x0F) | 0x40;
		break;
		default:
		break;
	}
	
	PORTD = PORTD_TEMP;
	return state;
}

int main(void) {
	
	// initialize ports
	DDRD = 0xFF; PORTD = 0x00;
	
	tasksNum = 2; // declare number of tasks
	task tsks[2]; // initialize the task array
	tasks = tsks; // set the task array
	
	// define tasks
	unsigned char i=0; // task counter
	tasks[i].state = -1;
	tasks[i].period = 1500;
	tasks[i].elapsedTime = tasks[i].period;
	tasks[i].TickFct = &TickFct_BlinkLED;
	++i;
	tasks[i].state = -1;
	tasks[i].period = 500;
	tasks[i].elapsedTime = tasks[i].period;
	tasks[i].TickFct = &TickFct_ThreeLEDs;
	
	TimerSet(500); // value set should be GCD of all tasks
	TimerOn();

    while(1) {} // task scheduler will be called by the hardware interrupt
	
}

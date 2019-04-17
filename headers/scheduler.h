
// Permission to copy is granted provided that this header remains intact. 
// This software is provided with no warranties.

////////////////////////////////////////////////////////////////////////////////

#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <avr/interrupt.h>

// Internal variables for mapping AVR's ISR to our cleaner TimerISR model.
unsigned long tasksPeriodGCD = 1; // Start count from here, down to 0. Default 1ms
unsigned long tasksPeriodCntDown = 0; // Current internal count of 1ms ticks

// Variables for the ultrasonic sensor
volatile unsigned short sonar = 0;
volatile char sonar_flag = 0;

//unsigned char tasksNum = 0; // Number of tasks in the scheduler. Default 0 tasks

////////////////////////////////////////////////////////////////////////////////
// Struct for Tasks represent a running process in our simple real-time operating system
typedef struct task {
	signed 	 char state; 		//Task's current state
	unsigned long period; 		//Task period
	unsigned long elapsedTime; 	//Time elapsed since last task tick
	int (*TickFct)(int); 		//Task tick function
} task;

task tasks[tasksNum];

///////////////////////////////////////////////////////////////////////////////
// Heart of the scheduler code
void TimerISR() {
    static unsigned char i;
    for (i = 0; i < tasksNum; i++) { 
        if ( tasks[i].elapsedTime >= tasks[i].period ) { // Ready
            tasks[i].state = tasks[i].TickFct(tasks[i].state);
            tasks[i].elapsedTime = 0;
        }
        tasks[i].elapsedTime += tasksPeriodGCD;
    }
}

///////////////////////////////////////////////////////////////////////////////
// In our approach, the C programmer does not touch this ISR, but rather TimerISR()
ISR(TIMER1_COMPA_vect) {
	// CPU automatically calls when TCNT0 == OCR0 (every 1 ms per TimerOn settings)
	tasksPeriodCntDown--; 			// Count down to 0 rather than up to TOP
	if (tasksPeriodCntDown == 0) { 	// results in a more efficient compare
		TimerISR(); 				// Call the ISR that the user uses
		tasksPeriodCntDown = tasksPeriodGCD;
	}
}

///////////////////////////////////////////////////////////////////////////////
// Interrupt Service Routine for ultrasonic sensor that disables counter to record counter memory 
ISR(INT1_vect)
{
	if (sonar_flag == 1) {
		sonar_flag = 0; // Reset flag
		TCCR3B = 0; // Disable counter
		sonar = TCNT3; // Store counter memory
		TCNT3 = 0; // Reset counter memory
	}
	else {
		sonar_flag = 1; // Reset flag
		TCCR3B |= (1 << CS31); // Enable counter
	}
}

///////////////////////////////////////////////////////////////////////////////
// Set TimerISR() to tick every m ms
void TimerSet(unsigned long m) {
	tasksPeriodGCD = m;
	tasksPeriodCntDown = tasksPeriodGCD;
}

///////////////////////////////////////////////////////////////////////////////
void TimerOn() {
	// AVR timer/counter controller register TCCR1
	TCCR1B 	= (1<<WGM12)|(1<<CS11)|(1<<CS10);
                    // WGM12 (bit3) = 1: CTC mode (clear timer on compare)
					// CS12,CS11,CS10 (bit2bit1bit0) = 011: prescaler /64
					// Thus TCCR1B = 00001011 or 0x0B
					// So, 8 MHz clock or 8,000,000 /64 = 125,000 ticks/s
					// Thus, TCNT1 register will count at 125,000 ticks/s

	// AVR output compare register OCR1A.
	OCR1A 	= 125;	// Timer interrupt will be generated when TCNT1==OCR1A
					// We want a 1 ms tick. 0.001 s * 125,000 ticks/s = 125
					// So when TCNT1 register equals 125,
					// 1 ms has passed. Thus, we compare to 125.
					// AVR timer interrupt mask register

#if defined (__AVR_ATmega1284__)
    TIMSK1 	= (1<<OCIE1A); // OCIE1A (bit1): enables compare match interrupt - ATMega1284
#else
    TIMSK 	= (1<<OCIE1A); // OCIE1A (bit1): enables compare match interrupt - ATMega32
#endif

	// Initialize avr counter
	TCNT1 = 0;

	// TimerISR will be called every tasksPeriodCntDown milliseconds
	tasksPeriodCntDown = tasksPeriodGCD;

	// Enable global interrupts
	SREG |= 0x80;	// 0x80: 1000000
}

#endif //SCHEDULER_H

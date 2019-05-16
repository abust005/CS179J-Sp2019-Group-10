
// Permission to copy is granted provided that this header remains intact.
// This software is provided with no warranties.

////////////////////////////////////////////////////////////////////////////////

#ifndef ARDSCHEDULER_H
#define ARDSCHEDULER_H

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
// Currently: Interrupt is called once a millisecond,
SIGNAL(TIMER0_COMPA_vect)
{
  static unsigned char i;
    for (i = 0; i < tasksNum; i++) {
        if ( tasks[i].elapsedTime >= tasks[i].period ) { // Ready
            tasks[i].state = tasks[i].TickFct(tasks[i].state);
            tasks[i].elapsedTime = 0;
        }
        tasks[i].elapsedTime += 1; // Replace 1 with tasksPeriodGDC later
    }
}
///////////////////////////////////////////////////////////////////////////////
// FIXME: Set TimerISR() to tick every m ms
void TimerSet(unsigned long m) {
	// https://learn.adafruit.com/multi-tasking-the-arduino-part-2/timers
  // Timer0 is already used for millis() - we'll just interrupt somewhere
  // in the middle and call the "Compare A" function below
  OCR0A = 0xAF;
  TIMSK0 |= _BV(OCIE0A);
}

///////////////////////////////////////////////////////////////////////////////

#endif //ARDSCHEDULER_H

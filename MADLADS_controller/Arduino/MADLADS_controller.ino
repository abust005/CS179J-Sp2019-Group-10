/*
 * MADLADS_controller.ino
 * Created: 4/16/2019 7:30:36 PM
 * Authors :  Jonathan "Cuomo" Woolf
 *            Joshua Riley
 *            Colton Vosburg
 *            Adriel Bustamante
 *
 * Library: TMRh20/RF24, https://github.com/tmrh20/RF24/
 */
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#define tasksNum 3 // Number of tasks
#include "ardScheduler.h"

// ----------------------------------------
// === Inputs and Variables ===
// ----------------------------------------
#define topButtonPin    30 // This is the button for claw operation
#define bottomButtonPin 31 // This is the button for emergency stop (kill switch)
#define joystick1Pin    A0 // This is the joystick for left and right movement
#define joystick2Pin    A1 // This is the joystick for forward and reverse movement
#define joystick3Pin    A2 // This is the joystick for up and down movement

unsigned char temp, counter = 0x00; // SPI variables
unsigned char droneSignal = 0x00; // bits 0-1 are up/down, 2-4 are left/right/forward/reverse, 5 is claw, 6 is button2, 7 is parity bit
unsigned char upFlag, downFlag, clawFlag = 0x00; // Flags for bitmasking logic
unsigned char payload; // Variable for holding trasmittable data
unsigned short joystick, joystick2, joystick3 = 0x0000; // Variables to store ADC values of joysticks
const byte address[6] = "00001"; // Communication channel for RF module
RF24 radio(7, 8); // CE, CSN

// ----------------------------------------
// === Functions ===
// ----------------------------------------
/*
 * Source code from:
 * https://howtomechatronics.com/tutorials/arduino/arduino-wireless-communication-nrf24l01-tutorial/
 */
void RadioMasterInit() {
  radio.begin();
  radio.openWritingPipe(address);
  radio.setPALevel(RF24_PA_MIN);
  radio.stopListening();
}

// ----------------------------------------
// === Tick Function State Declarations ===
// ----------------------------------------
enum movement_states {Movement_Left_Right, Movement_Forward_Reverse, Movement_Up_Down} movement_state;
enum button_states {Button_Wait, Button_Top_Pressed, Button_Bottom_Pressed} button_state;
enum spi_states {Spi_Wait, Spi_Send} spi_state;

// ----------------------------------------
// === Tick Functions ===
// ----------------------------------------
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
int TickFct_movement(int movement_state)
{
  switch(movement_state)
  {
    case Movement_Left_Right: // Left joystick controls left and right movements
      joystick = analogRead(joystick1Pin); // Read ADC value into joystick variable
      if (joystick > 500 && downFlag == 0) // Joystick is being tilted left
      {
        if (upFlag == 1)
        {
          droneSignal = (droneSignal & 0xFB) | 0x10;
        }
        else
        {
          droneSignal = ((droneSignal & 0xE3) | 0x10); // L/R set to 100 for left
        }
      }
      else if (joystick < 200 && downFlag == 0) // Joystick is being tilted right
      {
        if (upFlag == 1)
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
          droneSignal &= 0xEB; // Bits anded with 010 to clear all but forward
        }
      }
      movement_state = Movement_Forward_Reverse; // Return to the forward reverse state
      break;
    case Movement_Forward_Reverse: // Left joystick controls forward and reverse movements
      joystick2 = analogRead(joystick2Pin); // Read ADC value into joystick2 variable
      if (joystick2 > 500) // Joystick is being tilted up
      {
        droneSignal |= 0x08; // F/R set to 1 for forward
        upFlag = 1;
        downFlag = 0;
      }
      else if (joystick2 < 200) // Joystick is being tilted down
      {
        droneSignal = (droneSignal & 0xF7) | 0x14; // F/R set to 101 for reverse
        upFlag = 0;
        downFlag = 1;
      }
      else // Joystick is not being tilted
      {
        droneSignal &= 0xF7; // Bits anded with 101 to clear forward
        upFlag = 0;
        downFlag = 0;
      }
      movement_state = Movement_Up_Down; // Return to up down state
      break;
    case Movement_Up_Down: // Right joystick controls up and down movements
      joystick3 = analogRead(joystick3Pin); // Read ADC value into joystick variable
      droneSignal &= 0xFC; // Up/Down set to 00 for maintain altitude
      if (joystick3 > 500) // Joystick is being tilted up
      {
        droneSignal |= 0x01; // Up/Down set to 10 for down
      }
      else if (joystick3 < 200) // Joystick is being tilted left
      {
        droneSignal |= 0x02; // Up/Down set to 01 for up
      }
      movement_state = Movement_Left_Right; // Return to left right state
      break;
    default:
      movement_state = Movement_Left_Right; // Return to left right state
      break;
  }
  return movement_state;
}

/* Button one controls the claw while button two is currently misc. but will likely down the drone
00 - neither b
10 - claw is engaged until the first button is pressed again
X1 - Drop it like its hot
*/

int TickFct_button(int button_state)
{
  static unsigned char buttonPressCnt = 0;
  switch(button_state)
  {
    case Button_Wait:
      if (digitalRead(bottomButtonPin) == HIGH){ // Button must be held down to prevent accidental drone loss
        buttonPressCnt = 0;
        button_state = Button_Bottom_Pressed;
      }
      else if (digitalRead(topButtonPin) == HIGH){
        clawFlag = clawFlag ? 0 : 1;
        button_state = Button_Top_Pressed;
      }
      droneSignal &= 0xDF; // buttons set to 00 for unused
      break;
    case Button_Top_Pressed:
      if (digitalRead(topButtonPin) == LOW){
        button_state = Button_Wait;
      }
      else {
        if (clawFlag)
        {
          droneSignal = (droneSignal & 0x9F) | 0x40; // Buttons set to 10
        }
        else {
          droneSignal &= 0x9F;
        }
      }
      break;
    case Button_Bottom_Pressed:
      if (digitalRead(bottomButtonPin) == LOW){
        button_state = Button_Wait;
      }
      else
      {
        if (buttonPressCnt++ >= 10) // Button must be held down to prevent accidental drone loss
        {
           buttonPressCnt = 0;
           droneSignal |= 0x20;
        }
      }
      break;
    default:
      button_state = Button_Wait;
      break;
  }

  return button_state;
}

// SPI
int TickFct_spi_master(int spi_state)
{
  switch(spi_state)
  {
    case Spi_Wait:
      counter = 0; // Counts the number of bits set to 1
      for (int i = 0; i < 7; i++)
      {
        temp = (droneSignal >> i) &  0x01; // Right shift by i and clear unwanted bits
        if (temp == 0x01)
        {
          counter++; // If the bit we are checking is set to 1, update the counter
        }
      }
      if ((counter % 2) == 0) // Checks whether or not we have an even number of bits
      {
        droneSignal &= 0x7F; // Set parity bit to 0 for even number of 1s
      }
      else
      {
        droneSignal |= 0x80; // Set parity bit to 1 for odd number of 1s
      }
      spi_state = Spi_Send;
      break;
    case Spi_Send:
      payload = droneSignal; // Update payload with the droneSignal
      radio.write(&payload, sizeof(payload));
      spi_state = Spi_Wait;
      break;
    default:
      spi_state = Spi_Wait;
      break;
  }
  return spi_state;
}

void setup() {
  // Setup input and output pins
  pinMode(topButtonPin, INPUT);
  pinMode(bottomButtonPin, INPUT);

  // Setup tasks
  unsigned char i = 0;
  tasks[i].state = -1;
  tasks[i].period = 50;
  tasks[i].elapsedTime = 0;
  tasks[i].TickFct = &TickFct_movement;
  i++;
  tasks[i].state = -1;
  tasks[i].period = 100;
  tasks[i].elapsedTime = 0;
  tasks[i].TickFct = &TickFct_button;
  i++;
  tasks[i].state = -1;
  tasks[i].period = 25;
  tasks[i].elapsedTime = 0;
  tasks[i].TickFct = &TickFct_spi_master;

  RadioMasterInit();
  TimerSet(1); // Cannot be changed
}

void loop() {
  // Leave empty
}

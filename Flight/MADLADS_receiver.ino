/*
 * MADLADS_receiver.ino
 * Authors :  Jonathan Woolf
 *            Joshua Riley
 *            Colton Vosburg
 *            Adriel Bustamante
 *
 * Library: TMRh20/RF24, https://github.com/tmrh20/RF24/
 */
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

// ----------------------------------------
// === Inputs and Variables ===
// ----------------------------------------
const byte address[6] = "00001";
unsigned char isClose = 0;      // This is a boolean to show if the drone is close to the ground
unsigned char isValid = 0;      // This is a boolean to show if the signal received is valid with the parity bit
unsigned char droneSignal = 0;  // This is the received signal from either the controller or navigation system
unsigned short hover_cnt = 0;   // This is a count in milliseconds of the time the drone has been hovering
RF24 radio( 7, 8 ); // CE, CSN

// ----------------------------------------
// === Functions ===
// ----------------------------------------
int GetBit( unsigned char var, unsigned char bit )
{
  return ( var & ( 0x01 << bit ) );
}

void RadioServantInit()
{
  radio.begin();
  radio.openReadingPipe( 0, address );
  radio.setPALevel( RF24_PA_MIN );
  // radio.setPALevel( RF24_PA_MAX );
  radio.startListening();
  return;
}

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

BUTTON BITS -         *XX* ****
00 - claw is disengaged
10 - claw is engaged
X1 - emergency shut off

*/

void MotorActuator( unsigned char value )
{
  // FIXME
  if( GetBit( value, 5) )
  { // Kill motors

  }
  else
  {
    if( GetBit( value, 0 ) && !GetBit( value, 1) )
    { // Drone is moving down along z-axis

    }
    else if( !GetBit( value, 0 ) && GetBit( value, 1))
    { // Drone is moving up along z-axis

    }
    else
    { // Drone not moving along z-axis

    }
  }
  return;
}

int IsCloseToGround()
{
  // FIXME
  // Input: LIDAR
  // Output: Boolean
  // if input < distance, return 1
  return 0;
}

int IsSignalValid( unsigned char signal ){
  unsigned char temp = 0;
  for ( unsigned i = 0; i < 8; i++ )
  {
    temp ^= ( ( signal >> i ) & 0x01 );
  }
  return (temp & 0x01);
}

// ----------------------------------------
// === Tick Function State Declarations ===
// ----------------------------------------
enum flightSM {Flight_Arm, Flight_Read, Flight_Standby, Flight_Process, Flight_Descend, Flight_Land} flight_states;


// ----------------------------------------
// === Tick Functions ===
// ----------------------------------------
int TickFct_flight( int state )
{

  switch( state )
  { // Transtions
    case Flight_Arm:
      state = Flight_Read;
      break;
    case Flight_Read:
      state = isValid ? Flight_Process : Flight_Standby;
      break;
    case Flight_Standby:
      if( hover_cnt > 300000 )
      {
        state = Flight_Descend;
      }
      else
      {
        state = Flight_Read;
      }
      break;
    case Flight_Process:
      state = Flight_Read;
      break;
    case Flight_Descend:
      state = isClose ? Flight_Land : Flight_Descend;
      break;
    case Flight_Land:
      // Leave empty
      break;
    default:
      break;
  }

  switch( state )
  { // Actions
    case Flight_Arm:
      hover_cnt = 0;
      RadioServantInit();
      break;
    case Flight_Read:
      if( radio.available() )
      {
        // Read signal from controller
        radio.read( &droneSignal, sizeof(droneSignal) );
      }
      else {
        // FIXME: Read signal from navigation subsystem
        droneSignal = 0x00;
      }
      isValid = IsSignalValid(droneSignal);
      break;
    case Flight_Standby:
      hover_cnt++;
      break;
    case Flight_Process:
      hover_cnt = 0;
      MotorActuator( droneSignal );
      break;
    case Flight_Descend:
      // Measure distance to ground with lidar
      isClose = IsCloseToGround();
      break;
    case Flight_Land:
      // Kill motors and power off drone
      MotorActuator (0x40);
      break;
  }

  return state;
}

// ----------------------------------------
// === Main Code ===
// ----------------------------------------
void setup()
{
  // Setup input and output pins

  // Setup tasks

  // Set timer
}

void loop()
{
  // Leave empty
}

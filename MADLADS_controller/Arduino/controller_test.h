/*
 * MADLADS Controller Test
 * Created: 4/16/2019 7:30:36 PM
 * Authors : Jonathan "Cuomo" Woolf
 *       Joshua Riley
 *       Colton Vosburg
 *       Adriel Bustamante
 * Library: TMRh20/RF24, https://github.com/tmrh20/RF24/
 */
#ifndef CONTROLLER_TEST_H
#define CONTROLLER_TEST_H

//extern enum movement_state;
//extern enum button_state;
//extern enum spi_state;
//extern int TickFct_movement(int);
//extern int TickFct_button(int);
//extern int spi_master(int);
//extern unsigned short joystick, joystick2, joystick3;
//extern unsigned char droneSignal, clawFlag;
//extern unsigned char payload[32];

int SampleTickFct(int state)
// Requires Serial.begin(9600)
{
  switch (state)
  {
    case 0:
      Serial.println("Tick...");
      state = -1;
      break;
    default:
      Serial.println("Tock...");
      state = 0;
      break;
  }
  return state;
}

void movementPrintTest(){
  // DEBUG: To be used without timer interrupt
  // Test: Verify joystick input behavior
  for (unsigned char i = 0; i < 3; i++){
    movement_state = TickFct_movement(movement_state);
    delay(50);
  }
  Serial.print("Joystick 1: ");
  Serial.print(joystick);
  Serial.print(" Joystick 2: ");
  Serial.print(joystick2);
  Serial.print(" Joystick 3: ");
  Serial.println(joystick3);
  Serial.print("Drone Signal: ");
  Serial.println(droneSignal, BIN);
  delay(500);
}

void buttonPrintTest(){
  // DEBUG: To be used without timer interrupt
  // Test: Verify button input behavior
  button_state = TickFct_button(button_state);
  Serial.print("Claw Flag: ");
  Serial.println(clawFlag, BIN);
  Serial.print("Drone Signal: ");
  Serial.println(droneSignal, BIN);
  delay(250);
}

void spiMasterPrintTest(){
  // DEBUG: To be used without timer interrupt
  // Test: Verify RF module wired correctly
  //  Pass: Prints repeatedly if RF module wired correctly
  spi_state = spi_master(spi_state);
  Serial.print("droneSignal: ");
  Serial.println(droneSignal, HEX);
  Serial.print("payload: ");
  Serial.println(payload, HEX);
  delay(250);
}

#endif // CONTROLLER_TEST_H

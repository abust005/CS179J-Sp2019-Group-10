/*
 * MADLADS Controller Test
 * Created: 4/16/2019 7:30:36 PM
 * Authors :  Jonathan "Cuomo" Woolf
 *            Joshua Riley
 *            Colton Vosburg
 *            Adriel Bustamante
 * Library: TMRh20/RF24, https://github.com/tmrh20/RF24/
 */
#ifndef CONTROLLER_TEST_H
#define CONTROLLER_TEST_H

extern int TickFct_movement(int);
extern int TickFct_button(int);
extern int spi_master(int);
extern unsigned short joystick, joystick2, joystick3;
extern unsigned char droneSignal, clawFlag;
extern unsigned char payload;

/* === Timer Interrupt Test ===
 * Test: Verifies timer interrupt works
 * Note: Requires Serial.begin(9600) in void setup()
 */
int SampleTickFct(int state)
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

/* === Movement Test ===
 * DEBUG: To be used WITHOUT timer interrupt
 * Test: Verify joystick input behavior for the following:
 * - Left and right movement joystick
 * - Forward and backward movement joystick
 * - Upward and downward movement joystick
 */
void movementPrintTest(int movement_state){
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

/* === Button Test ===
 * DEBUG: To be used without timer interrupt
 * Test: Verify button input behavior
 */
int buttonPrintTest(int button_state){
  for (unsigned i = 0; i < 5; i++)
  {
    button_state = TickFct_button(button_state);
    delay(50);
  }
  Serial.print("Claw Flag: ");
  Serial.print(clawFlag, HEX);
  Serial.print(" Drone Signal: ");
  Serial.println(droneSignal, HEX);
  return button_state;
}

/* === Spi Master Transmit Test ===
 * DEBUG: To be used WITHOUT timer interrupt
 * Test: Verify RF module wired correctly
 * Note: Can also test if signal is being transmitted
 */
void spiMasterPrintTest(int spi_state){
  droneSignal = 0xAA;
  Serial.print("droneSignal: ");
  Serial.println(droneSignal, HEX);
  spi_state = spi_master(spi_state);
  Serial.print("payload: ");
  Serial.println(payload, HEX);
  delay(500);
}

#endif // CONTROLLER_TEST_H

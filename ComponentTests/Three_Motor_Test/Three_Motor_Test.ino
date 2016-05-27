#include <A4988.h>
#include <BasicStepperDriver.h>
#include <DRV8825.h>
#include <DRV8834.h>

/* 
 *  Ryan Niedzinski
 *  2/11/16
 *  
 *  This sketch requires the library BasicStepperDriver.h which can be downloaded from:
 *  https://github.com/laurb9/StepperDriver
 *  
 *  Purpose: Simulate full powder bed functionality with 3 connected motors
 *    1) move 78 steps (2mm) down for print motor
 *    2) move 104 steps (2.66mm) up for reseroir motor
 *    3) move 1 revolution forward/backward for rake motor
 *    
 *  Assuming 4 pin connections for each motor
 */

// For the 23HS41-1804S motor w/ 1.8 degree step, 
// 200 steps = 360 degrees, -200 = -360 degrees
#define MOTOR_STEPS_RES 200
// Need to check the number of steps for rake motor
#define MOTOR_STEPS_RAKE 200

// Since microstepping is set externally, make sure this matches the selected mode
// 1=full step, 2=half step etc.
#define MICROSTEPS 1

// Direction Pin numbers for a all connected motors:
//    PRINT = printing reservoir
int DIR_PRINT = 2;    //blue wire
int STEP_PRINT = 3;   //red wire
int reset_print = 5;  //green wire
int slp_print = 4;    //white wire

//    RESERVOIR = excess powder reservoir
int DIR_RESERVOIR = 6;    //blue wire
int STEP_RESERVOIR = 7;   //red wire
int reset_reservoir = 9;  //green wire
int slp_reservoir = 8;    //white wire

//    RAKE = rake
int DIR_RAKE = 10;    //blue wire
int STEP_RAKE = 11;   //red wire
int reset_rake = 13;  //green wire
int slp_rake = 12;    //white wire


// 2-wire basic config, microstepping is hardwired on the driver
BasicStepperDriver printStepper(MOTOR_STEPS_RES, DIR_PRINT, STEP_PRINT);
BasicStepperDriver reservoirStepper(MOTOR_STEPS_RES, DIR_RESERVOIR, STEP_RESERVOIR);
BasicStepperDriver rakeStepper(MOTOR_STEPS_RAKE, DIR_RAKE, STEP_RAKE);


int print_dist = 0;
int reservoir_dist = 0;
int rake_dist = 0;

/* // SoH sensor pins
int CO_Senser_Pin = 25;
int Thermo_Senser_Pin = 26;
int led_PIN = 27;
int buz_PIN = 28;
*/

void setup() {
  // reset and sleep inputs should be set to HIGH for driver operation
  digitalWrite(slp_print, LOW);
  digitalWrite(reset_print, HIGH);
  
  digitalWrite(slp_reservoir, LOW);
  digitalWrite(reset_reservoir, HIGH);
  
  digitalWrite(slp_rake, LOW);
  digitalWrite(reset_rake, HIGH);

  /*
   * Set target motor RPM.
   * These motors can do up to about 200rpm.
   * Too high will result in a high pitched whine and the motor does not move.
  */
  printStepper.setRPM(30);
  reservoirStepper.setRPM(30);
  rakeStepper.setRPM(30);
  delay(2000);
}

void loop() {
  /*
   * Tell the driver the microstep level we selected.
   * If mismatched, the motor will move at a different RPM than chosen.
   */
  printStepper.setMicrostep(MICROSTEPS);
  reservoirStepper.setMicrostep(MICROSTEPS);
  rakeStepper.setMicrostep(MICROSTEPS);

  /*
   * 1) Move print reservoir down 5 steps (9 degrees)
   *    Check if it has reached its limit
   *    Check SoH pins (TBR)
   */
  //move print bed down 75-78 steps ~ 2mm
  digitalWrite(slp_print, HIGH);
  printStepper.move(75*MICROSTEPS);
  digitalWrite(slp_print, LOW);
  delay(1000);

  /*
   * 2) Move excess powder reservoir up 5 steps (9 degrees)
   *    Check if it has reached its limit
   *    Check SoH pins (TBR)
   */
  //move reservoir bed up 104 steps ~ 133% two mm (150% was overdoing it)
  digitalWrite(slp_reservoir, HIGH);
  reservoirStepper.move(-104*MICROSTEPS);
  digitalWrite(slp_reservoir, LOW);
  delay(1000);

  /*
   * 3) Move rake one full revolution forwards and then backwards (length of powder bed ~980 steps)
   *    Check if it has reached its limit then reverse direction and move backwards
   *    Check SoH pins (TBR)
   */ 
  //move rake forward then backward
  digitalWrite(slp_rake, HIGH);
  rakeStepper.move(980*MICROSTEPS);
  digitalWrite(slp_rake, LOW);
  delay(1000);
  digitalWrite(slp_rake, HIGH);
  rakeStepper.move(-980*MICROSTEPS);
  digitalWrite(slp_rake, LOW);

  // 5 minute delay for each laser run, can increase this if going to just re-upload after each laser pass
  delay(300000);

  // can include a SoH check here if necessary
  if (print_dist == 180) {
    // Simulating reaching the bottom of the print reservoir (current input at 1/2 revolution)
    printStepper.rotate(180);
    print_dist = 0;
  }
  
  delay(3000);
}


/*
void safetyChecker() {
  // check voltage levels or low/high on SoH sensor pins
  if (CO_Senser_Pin == HIGH) {
    // Carbon Monoxide Senser has been triggered, alert user
    // Flash and pulse buzzer for 30 seconds
    for (int i = 0; i < 60; i++){
      digitalWrite(led_PIN, HIGH);
      digitalWrite(buz_PIN, HIGH);
      delay(250);
      digitalWrite(led_PIN, LOW);
      digitalWrite(buz_PIN, LOW);
      delay(250);
    }
    // LED and Buzzer remain on for 30 more seconds
    digitalWrite(led_PIN, HIGH);
    digitalWrite(buz_PIN, HIGH);
    delay(30000);
    // Turn off buzzer and led, but prevent further movement of PB
    digitalWrite(led_PIN, LOW);
    digitalWrite(buz_PIN, LOW);
    delay(300000);
  }

  // check thermometer voltage
  if (Thermo_Senser_Pin == HIGH) {
    // Thermo Senser has been triggered, alert user
    // Flash and pulse buzzer for 30 seconds
    for (int i = 0; i < 60; i++){
      digitalWrite(led_PIN, HIGH);
      digitalWrite(buz_PIN, HIGH);
      delay(250);
      digitalWrite(led_PIN, LOW);
      digitalWrite(buz_PIN, LOW);
      delay(250);
    }
    // LED and Buzzer remain on for 30 more seconds
    digitalWrite(led_PIN, HIGH);
    digitalWrite(buz_PIN, HIGH);
    delay(30000);
    // Turn off buzzer and led, but prevent further movement of PB
    digitalWrite(led_PIN, LOW);
    digitalWrite(buz_PIN, LOW);
    delay(300000);
  }
}
*/

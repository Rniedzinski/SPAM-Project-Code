#include <A4988.h>
#include <BasicStepperDriver.h>
#include <DRV8825.h>
#include <DRV8834.h>

/*
 * Ryan Niedzinski
 * 2/3/16
 * 
 * Purpose: Individually test the forward and backward control
 * of the different stepper motors used for the powder bed
 * 
 * Check this wiring diagram for a bipolar circuit
 * https://www.arduino.cc/en/Tutorial/MotorKnob
 * 
 * Must include library from: https://github.com/laurb9/StepperDriver
 */

// For the 23HS41-1804S motor w/ 1.8 degree step, 
// 200 steps = 360 degrees, -200 = -360 degrees
int MOTOR_STEPS = 200;

// Since microstepping is set externally, make sure this matches the selected mode
// 1=full step, 2=half step etc.
int MICROSTEPS = 1;

// Pin numbers for a single motor, direction = DIR, step = STEP
int DIR = 8;    //blue wire
int STEP = 9;   //red wire

// 2-wire basic config, microstepping is hardwired on the driver
BasicStepperDriver stepper(MOTOR_STEPS, DIR, STEP);

// Pin number variables (CORRECT THESE FOR THE RIGHT PINS)
int reset = 10;    //green wire
int slp = 11;      //white wire

int distance = 0;



void setup() {
  // reset and sleep inputs should be set to HIGH for driver operation
  digitalWrite(slp, HIGH);
  digitalWrite(reset, HIGH);

  /*
   * Set target motor RPM.
   * These motors can do up to about 200rpm.
   * Too high will result in a high pitched whine and the motor does not move.
   */
  stepper.setRPM(20);
}

void loop() {
  /*
   * Use this first loop for moving the motor +/- 1 revolution:
   * 
   * Tell the driver the microstep level we selected.
   * If mismatched, the motor will move at a different RPM than chosen.
   */
  stepper.setMicrostep(MICROSTEPS);

  //Moving motor one full revolution in 90 degree increments (50 steps) using the step notation
  stepper.move(50*MICROSTEPS);
  delay(1000);
  stepper.move(50*MICROSTEPS);
  delay(1000);  
  stepper.move(50*MICROSTEPS);
  delay(1000);
  stepper.move(50*MICROSTEPS);
  delay(1000);
  
  //Move motor backwards one full revolution in 90 degree increments
 // stepper.move(-200*MICROSTEPS);   //200 = one rev, microsteps = 1
  stepper.move(-50*MICROSTEPS);
  delay(1000);
  stepper.move(-50*MICROSTEPS);
  delay(1000);  
  stepper.move(-50*MICROSTEPS);
  delay(1000);
  stepper.move(-50*MICROSTEPS);
  delay(1000);

/* 
 *  Second loop tests more likely operating scenario
 *
 // --------------
  stepper.setMicrostep(MICROSTEPS); 
  
  // Need to know distance to degree conversion for accurate stepping distances
  stepper.move(5);  //9 degrees = 5 steps (9/1.8 = 5)

  distance = distance + 5;
  
  // Simulating reaching the end of the powder bed or top/bot of the reservoirs (in this case 1/4 of a rev)
  if (distance >= 50) {
    delay(1000);

    stepper.move(-50*MICROSTEPS);
    distance = 0;
  }
  delay(1000);
*/
  
}

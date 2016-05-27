#include "A4988.h"
#include "BasicStepperDriver.h"
#include "DRV8825.h"
#include "DRV8834.h"

/*
 * Ryan Niedzinski
 * 2/3/16
 * 
 * Purpose: Test the stepper motor controls with the use of the SLP pin in order to 
 * decrease heat build up in the motor drivers when not in use.
 * 
 */

// For the 23HS41-1804S motor w/ 1.8 degree step, 
// 200 steps = 360 degrees, -200 = -360 degrees
int MOTOR_STEPS = 200;

// Since microstepping is set externally, make sure this matches the selected mode
// 1=full step, 2=half step etc.
int MICROSTEPS = 1;

// Pin numbers for a single motor, direction = DIR, step = STEP
int DIR = 10;    //blue wire
int STEP = 11;   //red wire
int slp = 12;      //white wire
int reset = 13;    //green wire

// 2-wire basic config, microstepping is hardwired on the driver
BasicStepperDriver stepper(MOTOR_STEPS, DIR, STEP);

int distance = 0;


void setup() {
  // reset and sleep inputs should be set to HIGH for driver operation
  digitalWrite(slp, LOW);
  digitalWrite(reset, HIGH);

  stepper.setRPM(40);
  delay(3000);
}

void loop() {
  
  stepper.setMicrostep(MICROSTEPS);
  // 1mm = 70 degrees = 38.888 steps
  // 1014 at 78 per time
  //Moving motor one full revolution in 90 degree increments using the step notation


  digitalWrite(slp, HIGH);
  stepper.move(450*MICROSTEPS);
  digitalWrite(slp, LOW);
  delay(10000);
  
  digitalWrite(slp, HIGH);
  stepper.move(-450*MICROSTEPS);
  digitalWrite(slp, LOW);
  delay(10000);
  
  digitalWrite(slp, HIGH);
  stepper.move(450*MICROSTEPS);
  digitalWrite(slp, LOW);
  delay(10000);
  
}

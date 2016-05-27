#include <A4988.h>
#include <BasicStepperDriver.h>
#include <DRV8825.h>
#include <DRV8834.h>

#include <Wire.h> // I2C library, required for MLX90614
#include <SparkFunMLX90614.h> // SparkFunMLX90614 Arduino library

/*
   Ryan Niedzinski
   4/7/16

   Purpose: Full powder bed functionality with 3 connected motors
     1) Print Piston Down ~2mm
     2) Reservoir Piston Up ~2.7mm
     3) Rake Forward into snap switch then Rake Backward into snap switch
     4) Remain in SoH sensor checking mode with Emergency Subclass while printing (set time)

*/

// For the 23HS41-1804S motor w/ 1.8 degree step,
// 200 steps = 360 degrees, -200 = -360 degrees
// 38 steps ~= 1mm vertical movement for pistons
#define MOTOR_STEPS_RES 200
#define MOTOR_STEPS_RAKE 200

// Set Microstepping
// 1=full step, 2=half step
#define MICROSTEPS 1


/*
   ------------------------------------------------------------------------------------
   USER INPUT NEEDED FOR EACH PRINT:
      1)  Estimated time to complete the print job (1 min = delay(60000))
      2)  Number of Layers (1 layer ~= 2mm height)
*/

#define RESER_POTEN_LIMIT 0.85  //check potentiometer readings for bed zero height from ZEROING script
#define LAYER_DEPTH 2           //each individual layer depth in mm

/*
   ------------------------------------------------------------------------------------
*/

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

// SoH sensor pins
IRTherm therm; // Create an IRTherm object to interact with throughout
int led_pin = 45;
int buz_pin = 46;

// Switch Pins
int rake_forward_switch = A0;   //blue
int rake_backward_switch = A2;  //yellow
int print_bot_switch = A1;      //white
int reservoir_bot_switch = A3;  //black

// Misc variables
float print_voltage = 0;
float reservoir_voltage = 0;
int layers = 0;
float print_depth = 38 * LAYER_DEPTH;
float reservoir_depth = 53 * LAYER_DEPTH;
int print_move = (int) print_depth;
int reser_move = (int) reservoir_depth;




void setup() {
  // reset and sleep inputs should be set to HIGH for driver operation
  // Print Piston Pins
  digitalWrite(slp_print, LOW);
  digitalWrite(reset_print, HIGH);

  // Reservoir Piston Pins
  digitalWrite(slp_reservoir, LOW);
  digitalWrite(reset_reservoir, HIGH);

  // Rake Piston Pins
  digitalWrite(slp_rake, LOW);
  digitalWrite(reset_rake, HIGH);
  
  // Potentiometer Power In Pins
  digitalWrite(A4, HIGH);    //blue reservoir
  digitalWrite(A5, HIGH);    //black print

  // Read thermostate readings on 9600 serial baud rate
  Serial.begin(9600);        // Initialize Serial to log output
  therm.begin();             // Initialize thermal IR sensor
  therm.setUnit(TEMP_C);     // Set the library's units (C, F, K)

  // Emergency warning LED and Buzzer pins (LOW = off)
  //pinMode(led_PIN, OUTPUT);
  //pinMode(buz_PIN, OUTPUT);
  digitalWrite(led_pin, LOW);
  digitalWrite(buz_pin, LOW);

  //Set motor RPM
  printStepper.setRPM(30);
  reservoirStepper.setRPM(30);
  rakeStepper.setRPM(30);

  // Print print has begun
  Serial.println("SRM Print Job Has Started");

  // 5 second delay on first run to provide time to power on the motors
  delay(5000);
}






void loop() {
  // Set the driver microsteps (1 = no microstepping)
  printStepper.setMicrostep(MICROSTEPS);
  reservoirStepper.setMicrostep(MICROSTEPS);
  rakeStepper.setMicrostep(MICROSTEPS);
  
  Serial.println("Reading Surface Temperature");
  float run_bed = analogRead(print_bot_switch);      //white
  
  // when button is not pressed, read thermosensor on a loop
  // when button is pressed, leave this while loop and run powder bed loop 
  while (run_bed <= 10) {
    if (therm.read()) {
      float ambientT = therm.ambient();
      float objectT = therm.object();
      Serial.println("Surface Temperature " + String(objectT) + "C");
        
      if (objectT >= 180.0) {
        Serial.println();
        Serial.println("TEMPERATURE OVER 180C STOP PRINTING JOB");
        Serial.println();
        for (int i = 0; i < 60; i++) {
          digitalWrite(led_pin, HIGH);
          digitalWrite(buz_pin, HIGH);
          delay(250);
          digitalWrite(led_pin, LOW);
          digitalWrite(buz_pin, LOW);
          delay(250);
        }
        // LED and Buzzer remain on for 30 more seconds
        digitalWrite(led_pin, HIGH);
        digitalWrite(buz_pin, HIGH);
        delay(30000);
        // Turn off buzzer and led, but prevent further movement of PB
        digitalWrite(led_pin, LOW);
        digitalWrite(buz_pin, LOW);
        delay(300000);
      }
    }
    else {
      Serial.println("ERROR: Thermo Sensor Malfunction");
    }
    delay(100);
    run_bed = analogRead(print_bot_switch);      //white
  }
   
   
    
  /*
   *  1) Move print reservoir down
   *     Check potentiometer to make sure it hasnt reached its lower distance limit
   */
  Serial.println("Starting Layer " + String(layers));
  
  // A7 pin for print potentiometer
  int print_value = analogRead(A7);   //black tape = print bed
  print_voltage = print_value * (5.0 / 1023.0);

  // arbitrary half way point on the potentiometer to stop reservior from going too deep
  if (print_value <= 0.20) {
    Serial.println("Bottom Safety Limit of Print Bed Reached");
    for (int i = 0; i < 15; i++) {
      digitalWrite(buz_pin, HIGH);
      delay(1000);
      digitalWrite(buz_pin, LOW);
      delay(1000);
    }
    delay(100000);
  }
    
  // if not at the bottom of the printing area continue moving PB
  digitalWrite(slp_print, HIGH);
  printStepper.move(print_move * MICROSTEPS);
  digitalWrite(slp_print, LOW);
  delay(1000);



  /*
   *  2) move reservoir bed up 140% of LAYER_DEPTH
   *     Check potentiometer to make sure it hasnt reached the upper distance limit
   */
  
  // A6 pin for reservoir potentiometer, check if it reads 0 at the top or bottom
  int reservoir_value = analogRead(A6); //blue tape = reservoir
  reservoir_voltage = reservoir_value * (5.0 / 1023.0);

  // voltage above the limit variable will warn user and stop movement
  if (reservoir_voltage >= RESER_POTEN_LIMIT) {
    Serial.println("Top of the Reservoir Bed Reached");
    for (int i = 0; i < 15; i++) {
      digitalWrite(buz_pin, HIGH);
      delay(1000);
      digitalWrite(buz_pin, LOW);
      delay(1000);
    }
    delay(100000);
  }
  
  // if not at the top of the printing area continue moving PB
  digitalWrite(slp_reservoir, HIGH);
  reservoirStepper.move(-reser_move * MICROSTEPS);
  digitalWrite(slp_reservoir, LOW);
  delay(1000);



  /*
   *  3) Move rake until it hits the stop switches forward and then backward
   *     a)  use switches to determine if the rake has reaached the end or returned
   *         10 >= switch is hit, 10 <= not hit
   */
   
  // Move rake forward into stop switch
  float stop_switch1 = analogRead(rake_forward_switch); //blue
  digitalWrite(slp_rake, HIGH);
  while (stop_switch1 <= 10) {
    rakeStepper.move(5 * MICROSTEPS);
    stop_switch1 = analogRead(rake_forward_switch);
  }
  digitalWrite(slp_rake, LOW);
  Serial.println("Reversing direction");
  delay(1000);

  // Move rake backward into stop switch
  float stop_switch2 = analogRead(rake_backward_switch);
  digitalWrite(slp_rake, HIGH);
  while (stop_switch2 <= 10) {
    rakeStepper.move(-5 * MICROSTEPS);
    stop_switch2 = analogRead(rake_backward_switch);
  }
  digitalWrite(slp_rake, LOW);
  Serial.println("Rake Movement Done");
  delay(1000);

  layers = layers+1;
  Serial.println("Layer " + String(layers) + " Complete");
  
  /*for (int count_down = 5; count_down > 0; count_down--){
    Serial.println(String(count_down));
    delay(1000);
  } */
  Serial.println();
  run_bed = analogRead(print_bot_switch);  //white
}









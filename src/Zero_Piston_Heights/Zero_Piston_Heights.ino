#include <A4988.h>
#include <BasicStepperDriver.h>
#include <DRV8825.h>
#include <DRV8834.h>

#include <Wire.h> // I2C library, required for MLX90614
#include <SparkFunMLX90614.h> // SparkFunMLX90614 Arduino library

/*
   Ryan Niedzinski
   3/31/16

 **Must include both the stepper driver library and the sparkfun thermo library**
   https://github.com/laurb9/StepperDriver
   https://github.com/sparkfun/SparkFun_MLX90614_Arduino_Library
   
   Purpose: Full powder bed functionality with 3 connected motors
            **Input desired motor height (maximum will be determined by half of potentiometer length)
     1) Move Print bed to the print surface (screws level with middle dividing acrylic)
     2) Move Reservoir bed down 135% the height of the desired SRM height
     3) Flash LED when complete and do not repeat (if LED is connected)

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
      1)  Desired SRM height (in mm)    MAXIMUM HEIGHT IS 50mm
*/
#define PRINT_HEIGHT 10            //default is 10 mm, MUST BE CHECKED FOR EACH NEW DESIGN
#define PRINT_POTEN_LIMIT 0.87     //check the potentiometer reading for print at bed hieght
#define RESER_POTEN_LIMIT 0.85     //check the potentiometer reading for reservoir at bed height

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
int led_pin = 45;
int buz_pin = 46;

float print_voltage = 0;
float reservoir_voltage = 0;

float reservior_distance = (PRINT_HEIGHT * 1.40) + 1.0;
int mm_depth = (int)reservior_distance;




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
  digitalWrite(A4, HIGH);    //blue print bed?
  digitalWrite(A5, HIGH);    //black reservoir?

  // Read thermostate readings on 9600 serial baud rate
  Serial.begin(9600); // Initialize Serial to log output

  // Emergency warning LED and Buzzer pins (LOW = off)
  //pinMode(led_PIN, OUTPUT);
  //pinMode(buz_PIN, OUTPUT);
  digitalWrite(led_pin, LOW);
  digitalWrite(buz_pin, LOW);

  //Set motor RPM
  printStepper.setRPM(30);
  reservoirStepper.setRPM(30);
  rakeStepper.setRPM(30);

  // Print 'print job' has begun
  Serial.println("Moving Print Bed To Top For A SRM Length: " + String(PRINT_HEIGHT) + "mm");
  Serial.println("Moving Reservoir Down: " + String(mm_depth) + "mm");

  // 5 second delay on first run to provide time to power on the motors
  delay(5000);
}






void loop() {
  // Set the driver microsteps (1 = no microstepping)
  printStepper.setMicrostep(MICROSTEPS);
  reservoirStepper.setMicrostep(MICROSTEPS);
  rakeStepper.setMicrostep(MICROSTEPS);
  
  
  // if this part is not commented out you still need to verify potentiometer values!
  // use the code directly below this block to read potentiometer voltages on the serial port
  Serial.println("YOU STILL NEED TO CHECK THE POTENTIOMETER UPPER LIMIT VALUES BEFORE CONTINUING");
  delay(1000000);


/*
  // check what the top of each potentiometer reads for voltage and/or analogRead value
  int sensor_Value = 0;
  float voltage = 0;
  for(int test_len = 0; test_len <= 1000; test_len++)
  {
    sensor_Value = analogRead(A7);  //print
    voltage = sensor_Value * (5.0/1023.0);
    Serial.println("A7 pin reads: " + String(voltage) + " (should be print bed)");
    delay(100);

    digitalWrite(slp_print, HIGH);
    printStepper.move(-39 * MICROSTEPS);
    digitalWrite(slp_print, LOW);
    //delay(500);

    delay(1000);

    //digitalWrite(slp_reservoir, HIGH);
    //reservoirStepper.move(-39 * MICROSTEPS);
    //digitalWrite(slp_reservoir, LOW);
    
    sensor_Value = analogRead(A6);
    voltage = sensor_Value * (5.0/1023.0);
    Serial.println("A6 pin reads: " + String(voltage));
    delay(1000);
  }
  delay(10000);
*/
  
  
  
  if(PRINT_HEIGHT >= 75) {
    Serial.println("ERROR: CANNOT PRINT THIS DEEP, MAX = 50mm");
    delay(300000);
  }
  
// moves print bed down 5mm before zeroing to potentiometer tolerance
  for(int temp_move = 0; temp_move < 5; temp_move++) {
    digitalWrite(slp_print, HIGH);
    printStepper.move(39 * MICROSTEPS);
    digitalWrite(slp_print, LOW);
    delay(500);
  }

  /*
   *  1) Move print bed up 1 mm increments (~35-39 steps)
   *     Check potentiometer until it reaches its upper distance limit (voltage reading <= ??)
  */
  // A6 pin for print potentiometer, check if it reads 0 at the top or bottom
  int print_value = analogRead(A7);  //black
  print_voltage = print_value * (5.0 / 1023.0);

  // check height of screws compared to potentiometer voltage for consistent zeroing
  digitalWrite(slp_print, HIGH);
  while (print_voltage <= PRINT_POTEN_LIMIT) {     // <----- REPLACE THIS VOLTAGE VALUE FOR CURRENT POTENTIOMETER VALUES
    print_value = analogRead(A7);  //black
    print_voltage = print_value * (5.0 / 1023.0);
    printStepper.move(-39 * MICROSTEPS);
  }
  digitalWrite(slp_print, LOW);
  delay(2000);

  Serial.println("Print bed is at the top of the powder bed");
  delay(2000);

  /*
   *  2) move reservoir bed down 1.40*PRINT_HEIGHT after reaching the surface limit
   *     Check potentiometer to make sure it hasnt reached the lower distance limit
  */
  // A1 pin for reservoir potentiometer, check if it reads 0 at the top or bottom
  int reservoir_value = analogRead(A6);  //blue
  reservoir_voltage = reservoir_value * (5.0 / 1023.0);

  // move reservoir up to surface of powder bed (check against potentiometer readings)
  digitalWrite(slp_reservoir, HIGH);
  while (reservoir_voltage <= RESER_POTEN_LIMIT) {   // <----- REPLACE THIS VOLTAGE VALUE FOR CURRENT POTENTIOMETER VALUES
    reservoir_value = analogRead(A6);  //blue
    reservoir_voltage = reservoir_value * (5.0 / 1023.0);
    reservoirStepper.move(-39 * MICROSTEPS);
  }
  digitalWrite(slp_reservoir, LOW);
  Serial.println("Reservoir piston is at the top of the powder bed");
  delay(2000);

  // check to make sure potentiometer isnt below half way point (can change or remove lower limit)
  digitalWrite(slp_reservoir, HIGH);
  for (int rDown = 0; rDown <= mm_depth; rDown++) {
    reservoir_value = analogRead(A6);  //blue
    reservoir_voltage = reservoir_value * (5.0 / 1023.0);
    if (reservoir_voltage <= 0.20) {
      Serial.println("WARNING: Reached Safety Limit for Bottom of the Reservoir Piston");
      for (int i = 0; i < 10; i++) {
        digitalWrite(buz_pin, HIGH);
        delay(1000);
        digitalWrite(buz_pin, LOW);
        delay(1000);
      }
      delay(100000);
    }
    reservoirStepper.move(39 * MICROSTEPS);
  }
  digitalWrite(slp_reservoir, LOW);
  delay(2000);

  Serial.println("Reservoir Piston has been lowered " + String(mm_depth) + "mm");
  delay(20000);
  
  /*
   *  3) Zeroing complete, flash LED to signal complete
  */
  Serial.println("Zeroing Complete");
  for (int i = 0; i < 5; i++) {
    digitalWrite(led_pin, HIGH);
    delay(1000);
    digitalWrite(led_pin, LOW);
    delay(300000);
  }
  Serial.println();
}

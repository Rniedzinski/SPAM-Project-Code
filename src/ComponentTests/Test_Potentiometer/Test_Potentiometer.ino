/*
 * Slightly modified code from: https://www.arduino.cc/en/Tutorial/ReadAnalogVoltage
 * Last modified 2/12/2016
 * 
 * Purpose: Read voltage values determined by a linear potentiometer and display them
 * on the Serial Monitor for further analysis.
 */


void setup() {
/* 
  After uploading to the arduino, open serial communication with
  the magnify glass symbol in the top right of the sketch
*/
  // Initialize serial communication at 9600 bits per second:
  Serial.begin(9600);
}

void loop() {
  // sensorValue is the number for the resistance value from the analoge pin A0 
  // Output between 0 (off 0V) and 1023 (full on 5V)
  int sensorValue = analogRead(A0);    //int means exact number, can have better accuracy with float/double
  
  // resistance value is not voltage so do a quick computation (5V divided by 1023)
  float voltage = sensorValue * (5.0/1023.0);
  
  // print resistance value or voltage value
  //Serial.println(sensorValue);
  Serial.println(voltage);
  
  // delay in between reads for stability
  delay(1);
}

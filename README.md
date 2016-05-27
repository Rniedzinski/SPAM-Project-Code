# SPAM-Project-Code
Collection of Arduino sketches that test individual components as well as run the full SPAM powder bed.

SPAM Powder Bed Firmware

The powder bed in the SPAM project is controlled by an Arduino Mega. In order to utilize these sketches, please include the
two additional Arduino libraries BasicStepperDriver and SparkFun_MLX90614. These are available for download from Github at:
https://github.com/laurb9/StepperDriver
https://github.com/sparkfun/SparkFun_MLX90614_Arduino_Library

The TestComponents folder contains multiple sketches designed to test the functionality of the various components in the 
powder bed design. The src folder contains a Zeroing function and a Full System sketch for running the entire powder bed.

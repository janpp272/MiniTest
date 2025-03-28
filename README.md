# MiniTest
## Setup
- Install the required libraries from the arduino library manager
- Delete new_handler.cpp from libraries/ArduinoSTL/src

## Usage
- Insert the DUT (device under test)
- The pattern will be executed once the Arduino is connected to power

## LED
- short green blink: Pattern was successfully executed
- short red blink: Pattern has failed
- 3x short red blink: Assertion error, pattern could not be executed

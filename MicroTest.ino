#define PINCOUNT 12 // the number of defined pins
#define VECTORCOUNT 4 // the number of vectors (lines) in the pattern

void setup() {
  // setup serial monitor and status leds
  Serial.begin(9600);
  pinMode(11, OUTPUT);
  pinMode(12, OUTPUT);

  // define the pins in the order that they will be used in the pattern
  int pins[PINCOUNT] = { 9, A1, A2, A3, A4, A5, 7, 6, 5, 4, 3, 2 };

  // define the pinmodes for all pins in the order that they have been given in pins, 1 for write, 0 for read 
  int pinModes[PINCOUNT] = { 1, 1, 0, 1, 1, 0, 1, 1, 0, 1, 1, 0 };

  // set up the pattern with all read and write information
  int pattern[VECTORCOUNT][PINCOUNT] = {
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0 },
    { 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0 },
    { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 }
  };

  // set up a variable to store the test result
  bool fail = false;

  // set pin modes for all pins
  for (int pin = 0; pin < PINCOUNT; pin++) {
    if (pinModes[pin] == 1) pinMode(pins[pin], OUTPUT);
    else pinMode(pins[pin], INPUT);
  }

  // go over each vector (line) in the pattern
  for (int vector = 0; vector < VECTORCOUNT; vector++) {
    // go over each pin
    for (int stc = 0; stc < PINCOUNT; stc++) {
      // if the pin is a write pin, apply the signal from the pattern that corresponds to the current vector and pin
      if (pinModes[stc] == 1) digitalWrite(pins[stc], pattern[vector][stc]);
    }
    // short delay to give the chip time to compute the inputs and apply it's outputs
    delay(10);
    // go over each pin again
    for (int stc = 0; stc < PINCOUNT; stc++) {
      // if the pin is a read pin, check it's value and compare it to the expected value from the pattern that corresponds to the current vector and pin
      // if the return is not as expected, set the fail flag
      if (pinModes[stc] == 0 && digitalRead(pins[stc]) != pattern[vector][stc]) fail = true;
    }
  }

  // apply result leds to communicate test result
  digitalWrite(LED_BUILTIN, HIGH);
  if (fail) digitalWrite(11, HIGH);
  else digitalWrite(12, HIGH);

  delay(300);
  digitalWrite(LED_BUILTIN, LOW);
  digitalWrite(11, LOW);
  digitalWrite(12, LOW);

}

// loop remains empty, no repeated execution of code needed
void loop() {}

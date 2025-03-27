/**
 * MiniTest.ino
 * @brief A small testing framework for executing digital tests for logic ICs on Arduino.
 * @author janpp272
 */
#include <ArduinoSTL.h>

#include <iostream>
#include <vector>
#include <algorithm>

/** 
 * @brief Enum class for colors.
 */
enum class Color {
  RED,
  GREEN,
  BOTH
};

/**
 * @brief Enum class for pin roles.
 * Drive pins will be initialized as OUTPUT pins to send signals to the IC.
 * Receive pins will be initialized as INPUT pins to recieve signals from the IC.
 */
enum class PinRole {
  DRIVE,
  RECEIVE
};

/**
 * @brief Enum class for GPIO pins.
 * The GPIO pins are defined as D1 to D10 and A1 to A6.
 * Pins D11 and D12 are used for the RED and GREEN LEDs respectively and are not used for testing.
 */
enum class GPIO {
  D1,
  D2,
  D3,
  D4,
  D5,
  D6,
  D7,
  D8,
  D9,
  D10,
  A1,
  A2,
  A3,
  A4,
  A5,
  A6
};

/**
 * @brief Converts GPIO enum to pin number.
 * @param gpio The GPIO enum value.
 * @return The corresponding pin number.
 */
int gpioToPin(GPIO gpio) {
  switch(gpio) {
    case GPIO::D1: return 1;
    case GPIO::D2: return 2;
    case GPIO::D3: return 3;
    case GPIO::D4: return 4;
    case GPIO::D5: return 5;
    case GPIO::D6: return 6;
    case GPIO::D7: return 7;
    case GPIO::D8: return 8;
    case GPIO::D9: return 9;
    case GPIO::D10: return 10;
    case GPIO::A1: return A1;
    case GPIO::A2: return A2;
    case GPIO::A3: return A3;
    case GPIO::A4: return A4;
    case GPIO::A5: return A5;
    case GPIO::A6: return A6;
  }
}

/**
 * @brief Asserts a condition. If the condition is not met, the RED LED blinks and the program is terminated.
 * @param condition The condition to check.
 * @param exception The exception message to display.
 */
bool assertThat(bool condition, String exception) {
  if (!condition) {
    for (int i = 0; i < 3; i++) {
      blink(Color::RED, 0.2);
      delay(100);
    }
    Serial.begin(9600);
    while (!Serial);
    Serial.println(exception);
    abort();
  }
}

/**
 * @brief Blinks the RED or GREEN LED for a specified duration.
 * @param color The color of the LED to blink.
 * @param duration The duration to blink the LED in seconds.
 */
void blink(Color color, double duration) {
  assertThat(duration > 0, "Unexpected blink duration!");
  switch(color) {
    case Color::RED:
      digitalWrite(11, HIGH);
      break;
    case Color::GREEN:
      digitalWrite(12, HIGH);
      break;
    case Color::BOTH:
      digitalWrite(11, HIGH);
      digitalWrite(12, HIGH);
  }
  delay(duration * 1000);
  digitalWrite(11, LOW);
  digitalWrite(12, LOW);
}

/**
 * @brief Pin class representing a GPIO pin and its role (drive or receive).
 */
class Pin {
  public:
    /**
     * @brief Default constructor for Pin class.
     */
    Pin() {    }

    /**
     * @brief Constructor for Pin class.
     * @param pinName The name of the GPIO pin.
     * @param pinRole The role of the pin (drive or receive).
     */
    Pin(GPIO pinName, PinRole pinRole) {
      switch (pinRole) {
        case PinRole::DRIVE:
          pinMode(gpioToPin(pinName), OUTPUT);
          break;
        case PinRole::RECEIVE:
          pinMode(gpioToPin(pinName), INPUT);
          break;
      }
      myPinName = pinName;
      myPinRole = pinRole;
    };

    /**
     * @brief Gets the pin name.
     * @return The name of the pin.
     */
    GPIO getName() {
      return myPinName;
    }

    /**
     * @brief Gets the pin role (drive or receive).
     * @return The role of the pin.
     */
    PinRole getRole() {
      return myPinRole;
    }

    /**
     * @brief Checks if two pins are equal.
     * @param comparison The pin to compare with.
     * @return True if the pins are equal, false otherwise.
     */
    bool operator==(const Pin& comparison) const {
      return this->myPinName == comparison.myPinName && myPinRole == comparison.myPinRole;
    }

    /**
     * @brief Checks if two pins are not equal.
     * @param comparison The pin to compare with.
     * @return True if the pins are not equal, false otherwise.
     */
    bool operator!=(const Pin& comparison) const {
      return this->myPinName != comparison.myPinName || myPinRole != comparison.myPinRole;
    }

  private:
    GPIO myPinName;
    PinRole myPinRole;
};

/**
 * @brief PinConfiguration class representing a list of pins used in a test program.
 */
class PinConfiguration {
  public:
    /**
     * @brief Constructor for PinConfiguration.
     * @param dataPins A vector of pins to be configured.
     */
    PinConfiguration(std::vector<Pin> dataPins) {
      for (size_t i = 0; i < dataPins.size(); ++i) {
        for (size_t j = i + 1; j < dataPins.size(); ++j) {
          assertThat(dataPins[i].getName() != dataPins[j].getName(), "Duplicate pin found!");
        }
      }
      myPins = dataPins;
    }

    /**
     * * @brief Gets the number of configured pins.
     */
    int getPinCount() {
      return myPins.size();
    }

    /**
     * @brief Gets all pis of a role (drive or receive).
     * @param role The role of the pins to get.
     * @return A vector of pins with the specified role.
     */
    std::vector<Pin> getPinGroupByRole(PinRole role) {
      std::vector<Pin> pinGroup;
      for (Pin pin : myPins) {
        if(pin.getRole() == role) {
          pinGroup.push_back(pin);
        }
      }
      return pinGroup;
    }

    /**
     * @brief Checks if a certain pin is configured.
     * @param pin The pin to check.
     * @return True if the pin is configured, false otherwise.
     */
    bool hasPin(Pin pin) {
      return std::find(myPins.begin(), myPins.end(), pin) != myPins.end();
    }

    /**
     * @brief Gets the index of a pin in the configuration.
     * @param pin The pin to find.
     * @return The index of the pin in the configuration.
     */
    int getPinIndex(Pin pin) {
      for (size_t i = 0; i < myPins.size(); ++i) {
        if (myPins[i] == pin) {
          return i;
        }
      }
      assertThat(false, "Pin was not found!");
    }

  private:
    std::vector<Pin> myPins;
};

/**
 * @brief PatternData class representing the pattern data for a test program.
 * It contains a vector of vectors, where each inner vector represents a vector of state characters.
 */
class PatternData {
  public:
    /**
     * @brief Constructor for PatternData.
     * @param data A vector of vectors representing the pattern data.
     */
    PatternData(std::vector<std::vector<char>> data) {
      assertThat(data.size() > 0, "Pattern is empty!");

      std::vector<char> allowedCharacters;
      allowedCharacters.push_back('0');
      allowedCharacters.push_back('1');
      allowedCharacters.push_back('L');
      allowedCharacters.push_back('H');
      allowedCharacters.push_back('X');

      for (std::vector<char> vector : data) {
        for (char stateCharacter : vector) {
          assertThat(std::find(allowedCharacters.begin(), allowedCharacters.end(), stateCharacter) != allowedCharacters.end(), "Invalid state character");
        }
      }

      myData = data;
    }

    /**
     * @brief Gets the vector of state characters for a specific vector number.
     * @param vectorNumber The index of the vector to get.
     * @return A vector of state characters for the specified vector number.
     */
    std::vector<char> getVector(int vectorNumber) {
      assertThat(vectorNumber < myData.size() && vectorNumber >= 0, "Vector position out of range");
      return myData[vectorNumber];
    }

    /**
     * @brief Gets the state character for a specific vector number and pin index.
     * @param vectorNumber The index of the vector.
     * @param pinIndex The index of the pin in the vector.
     * @return The state character for the specified vector number and pin index.
     */
    char getStateCharacter(int vectorNumber, int pinIndex) {
      assertThat(vectorNumber < myData.size() && vectorNumber >= 0, "Vector position out of range");
      return myData[vectorNumber][pinIndex];
    }

    /**
     * @brief Gets the number of vectors in the pattern data.
     * @return The number of vectors in the pattern data.
     */
    int getVectorCount() {
      return myData.size();
    }

  private:
    std::vector<std::vector<char>> myData;
    int vectorLength;
};

/**
 * @brief Pattern class representing a test pattern consisting out of a pin configuration and pattern data.
 */
class Pattern {
  public:
    /**
     * @brief Constructor for Pattern.
     * @param pinConfiguration The pin configuration for the pattern.
     * @param patternData The pattern data for the pattern.
     */
    Pattern(PinConfiguration pinConfiguration, PatternData patternData) : myPinConfiguration(pinConfiguration), myPatternData(patternData) {
    }

    /**
     * @brief Gets the pattern data.
     * @return The pattern data.
     */
    std::vector<char> getVector(int vectorNumber) {
      assertThat(vectorNumber < myPatternData.getVectorCount() && vectorNumber >= 0, "Vector number out of range");
      assertThat(myPatternData.getVector(vectorNumber).size() == myPinConfiguration.getPinCount(), "Wrong number of elements in vector!");
      return myPatternData.getVector(vectorNumber);
    }

    /**
     * @brief Gets the pin configuration of the pattern.
     * @return The pin configuration of the pattern.
     */
    PinConfiguration getPinConfiguration() {
      return myPinConfiguration;
    }

    /**
     * @brief Gets a state character from the pattern.
     * @param vector The index of the vector.
     * @param pin The pin to get the state character for.
     * @return The state character for the specified vector and pin.
     */
    char getStateCharacter(int vector, Pin pin) {
      assertThat(vector >= 0 && vector < myPatternData.getVectorCount(), "Vector index out of range");
      assertThat(myPinConfiguration.hasPin(pin), "Pin is not defined in pin configuration");
      char stateChar = myPatternData.getStateCharacter(vector, myPinConfiguration.getPinIndex(pin));
      if (pin.getRole() == PinRole::DRIVE) {
        assertThat(stateChar == '0' || stateChar == '1', "Invalid state character!");
      } else {
        assertThat(stateChar == 'L' || stateChar == 'H' || stateChar == 'X', "Invalid state character!");
      }
      return stateChar;
    }

    /**
     * @brief Gets the number of vectors in the pattern.
     * @return The number of vectors in the pattern.
     */
    int getVectorCount() {
      return myPatternData.getVectorCount();
    }

    private:
      PinConfiguration myPinConfiguration;
      PatternData myPatternData;
};

/**
 * @brief Result class representing the result of a test program execution.
 * It contains information about whether the test passed or failed and the failing vectors.
 */
class Result {
  public:
    /**
     * @brief Default constructor for Result class.
     */
    Result() {
      passed = true;
      failed = false;
    }

    /**
     * @brief Sets the result to failed and adds the failing vector to the list of failing vectors.
     * @param failingVector The vector number that failed.
     */
    void fail(int failingVector) {
      passed = false;
      failed = true;
      failingVectors.push_back(failingVector);
    }

    /**
     * @brief Checks wether the result is pass.
     * @return True if the test passed, false otherwise.
     */
    bool hasPassed() {
      return passed;
    }

    /**
     * @brief Checks wether the result is failed.
     * @return True if the test failed, false otherwise.
     */
    bool hasFailed() {
      return failed;
    }

    /**
     * @brief Gets the vector numbers that failed.
     * @return A vector of failing vector numbers.
     */
    std::vector<int> getFailingVectors() {
      return failingVectors;
    }

  private:
    bool passed;
    bool failed;
    std::vector<int> failingVectors = std::vector<int>();
};

/**
 * @brief TestProgram class representing a test program that executes a pattern on the configured pins.
 */
class TestProgram {
  public:
    /**
     * @brief Constructor for TestProgram.
     * @param pattern The pattern to execute.
     * @param result The result object to store the test result.
     */
    TestProgram(Pattern pattern, Result& result) : myPattern(pattern), myResult(result) {    }

    /**
     * @brief Executes the test program.
     * Sets the drive pins to the specified state and checks the receive pins for expected values for every vector in the pattern.
     */
    void execute() {
      PinConfiguration pinConfiguration = myPattern.getPinConfiguration();
      std::vector<Pin> drivePins = pinConfiguration.getPinGroupByRole(PinRole::DRIVE);
      std::vector<Pin> receivePins = pinConfiguration.getPinGroupByRole(PinRole::RECEIVE);
      int vectorCount = myPattern.getVectorCount();
      for (int i = 0; i < vectorCount; i++) {
        for (Pin pin : drivePins) {
          char driveValue = myPattern.getStateCharacter(i, pin);
          switch (driveValue) {
            case '0':
              digitalWrite(gpioToPin(pin.getName()), LOW);
              break;
            case '1':
              digitalWrite(gpioToPin(pin.getName()), HIGH);
              break;
          }
        }
        delay(50); // Wait for the IC to react to the new driving values
        for (Pin pin : receivePins) {
          char expectedResponse = myPattern.getStateCharacter(i, pin);
          int pinResponse = digitalRead(gpioToPin(pin.getName()));
          if (expectedResponse != 'X') {
            if (pinResponse == 1 && expectedResponse != 'H') {
              myResult.fail(vectorCount);
            } else if (pinResponse == 0 && expectedResponse != 'L') {
              myResult.fail(vectorCount);
            }
          }
        }
      }
    }

  private:
    Pattern myPattern;
    Result& myResult;
};

/**
 * @brief Setup function for the Arduino sketch.
 * Initializes the pins and executes the test program.
 */
void setup() {
  // Set pin modes for LEDs
  pinMode(11, OUTPUT); //RED
  pinMode(12, OUTPUT); //GREEN

  // Example pin configuration for testing an AND-gate with 2 inputs and 1 output
  // Pin D9 and A1 are the inputs (drive pins) and A2 is the output (receive pin).
  std::vector<Pin> myPins = {
    Pin(GPIO::D9, PinRole::DRIVE),
    Pin(GPIO::A1, PinRole::DRIVE),
    Pin(GPIO::A2, PinRole::RECEIVE)
  };
  
  // Example pattern data for the AND-gate test program
  // The first two vectors represent the input combinations (00, 01, 10, 11) and the expected output (L or H).
  std::vector<std::vector<char>> myPatternDataVectors = {
    {'0', '0', 'L'},
    {'0', '1', 'L'},
    {'1', '0', 'L'},
    {'1', '1', 'H'}
  };
  
  // Initialize all necessary objects
  PinConfiguration myPinConfiguration(myPins);
  PatternData myPatternData(myPatternDataVectors);
  Pattern myPattern(myPinConfiguration, myPatternData);
  
  Result myResult;

  TestProgram myTestProgram(myPattern, myResult);

  // Execute the test program on the IC
  myTestProgram.execute();

  // Evaluate and display the results
  if (myResult.hasPassed()) {
    blink(Color::GREEN, 0.3);

  } else {
    blink(Color::RED, 0.3);
  }
}

/**
 * @brief Loop function for the Arduino sketch.
 * This function is empty and does not perform any actions.
 */
void loop() {  abort();  }

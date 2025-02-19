#include <ArduinoSTL.h>

#include<math.h>

#include <iostream>
#include <vector>
#include <algorithm>

const static int PIN_COUNT = 12;

enum class Color {
  RED,
  GREEN,
  BOTH
};

enum class PinState {
  DRIVE,
  RECEIVE
};

bool assertThat(bool condition, String exception) {
  if (!condition) {
    Serial.println("Assertion error: '" + exception);
    abort();
  }
}

void blink(Color color, int duration) {
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

class Pin {
  public:
    Pin() {    }

    Pin(int pinName, String pinRole) {
      assertThat(pinRole.equals("DRIVE") || pinRole.equals("RECEIVE"), "Invalid Pin Mode");

      if (pinRole.equals("DRIVE") || pinRole.equals("POWER")) {
        pinMode(pinName, OUTPUT);
      } else if (pinRole.equals("RECEIVE") || pinRole.equals("GND")) {
        pinMode(pinName, INPUT);
      }
      myPinName = pinName;
      myPinRole = pinRole;
    };

    int getName() {
      return myPinName;
    }

    String getRole() {
      return myPinRole;
    }

    bool operator==(const Pin& comparison) const {
      // return this->myPinName == comparison.myPinName && myPinRole == comparison.myPinRole;
      return true;
    }

    bool operator!=(const Pin& comparison) const {
        // return this->myPinName != comparison.myPinName || myPinRole != comparison.myPinRole;
        return true;
    }

  private:
    int myPinName;
    String myPinRole;
};

class PinConfiguration {
  public:
    PinConfiguration(std::vector<Pin> pins) {
      assertThat(pins.size() == PIN_COUNT, "Pin configuration has wrong number of pins!");

      for (Pin pin : pins) {
        int pinName = pin.getName();
        assertThat(std::find(knownPinNames.begin(), knownPinNames.end(), pinName) != knownPinNames.end(), "Duplicate pin defined!");
        knownPinNames.push_back(pinName);
      }
      myPins = pins;
    }

    int getPinCount() {
      return myPins.size();
    }

    std::vector<Pin> getPinGroupByRole(String role) {
      assertThat(role.equals("DRIVE") || role.equals("RECEIVE"), "Unknown pin role");
      std::vector<Pin> pinGroup;
      for (Pin pin : myPins) {
        if(pin.getRole().equals(role)) {
          pinGroup.push_back(pin);
        }
      }
      return pinGroup;
    }

    bool hasPin(Pin pin) {
      return std::find(myPins.begin(), myPins.end(), pin) != myPins.end();
    }

    int getPinIndex(Pin pin) {
      for (size_t i = 0; i < myPins.size(); ++i) {
        if (myPins[i] == pin) {
          return i;
        }
      }
      assertThat(false, "Pin was not found!");
    }

    void powerUp() {
      for (Pin pin : myPins) {
        if (pin.getRole().equals("POWER")) {
          digitalWrite(pin.getName(), HIGH);
        }
      }
    }

    void powerDown() {
      for (Pin pin : myPins) {
        if (pin.getRole().equals("POWER")) {
          digitalWrite(pin.getName(), LOW);
        }
      }
    }

  private:
    std::vector<Pin> myPins;
    std::vector<int> knownPinNames;
};

class PatternData {
  public:
    PatternData(std::vector<std::vector<char>> data) {
      assertThat(data.size() > 0, "Pattern is empty!");

      std::vector<char> allowedCharacters;
      allowedCharacters.push_back('0');
      allowedCharacters.push_back('1');
      allowedCharacters.push_back('L');
      allowedCharacters.push_back('H');
      allowedCharacters.push_back('X');

      for (std::vector<char> vector : data) {
        assertThat(vector.size() == PIN_COUNT, "Vector has wrong number of elements");
        for (char stateCharacter : vector) {
          assertThat(std::find(allowedCharacters.begin(), allowedCharacters.end(), stateCharacter) != allowedCharacters.end(), "Invalid state character");
        }
      }

      myData = data;
    }

    std::vector<char> getVector(int vectorNumber) {
      assertThat(vectorNumber < myData.size() && vectorNumber >= 0, "Vector position out of range");
      return myData[vectorNumber];
    }

    char getStateCharacter(int vectorNumber, int pinIndex) {
      assertThat(vectorNumber < myData.size() && vectorNumber >= 0, "Vector position out of range");
      return myData[vectorNumber][pinIndex];
    }

    int getVectorCount() {
      return myData.size();
    }

  private:
    std::vector<std::vector<char>> myData;
    int vectorLength;
};

class Pattern {
  public:
    Pattern(PinConfiguration pinConfiguration, PatternData patternData) : myPinConfiguration(pinConfiguration), myPatternData(patternData) {    }

    std::vector<char> getVector(int vectorNumber) {
      assertThat(vectorNumber < myPatternData.getVectorCount() && vectorNumber >= 0, "Vector number out of range");
      return myPatternData.getVector(vectorNumber);
    }

    PinConfiguration getPinConfiguration() {
      return myPinConfiguration;
    }

    char getStateCharacter(int vector, Pin pin) {
      assertThat(vector >= 0 && vector < myPatternData.getVectorCount(), "Vector index out of range");
      return myPatternData.getStateCharacter(vector, myPinConfiguration.getPinIndex(pin));
    }

    int getVectorCount() {
      return myPatternData.getVectorCount();
    }

    int powerUp() {
      myPinConfiguration.powerUp();
    }

    private:
      PinConfiguration myPinConfiguration;
      PatternData myPatternData;
};

class Result {
  public:
    Result() {
      passed = true;
      failed = false;
    }

    void fail(int failingVector) {
      passed = false;
      failed = true;
      failingVectors.push_back(failingVector);
    }

    bool hasPassed() {
      return passed;
    }

    bool hasFailed() {
      return failed;
    }

    std::vector<int> getFailingVectors() {
      return failingVectors;
    }

  private:
    bool passed;
    bool failed;
    std::vector<int> failingVectors = std::vector<int>();
};

class TestProgram {
  public:
    TestProgram(Pattern pattern, Result result) : myPattern(pattern), myResult(result) {    }

    void execute() {
      PinConfiguration pinConfiguration = myPattern.getPinConfiguration();
      int vectorCount = myPattern.getVectorCount();
      int pinCount = pinConfiguration.getPinCount();
      for (int i = 0; i < vectorCount; i++) {
        for (Pin pin : pinConfiguration.getPinGroupByRole("DRIVE")) {
          char driveValue = myPattern.getStateCharacter(i, pin);
          switch (driveValue) {
            case '0':
              digitalWrite(pin.getName(), LOW);
              break;
            case '1':
              digitalWrite(pin.getName(), HIGH);
              break;
          }
        }
        delay(10);
        for (Pin pin : pinConfiguration.getPinGroupByRole("RECEIVE")) {
          char expectedResponse = myPattern.getStateCharacter(i, pin);
          int pinResponse = digitalRead(pin.getName());
          if (pinResponse == 1 && expectedResponse == 'L') {
            myResult.fail(vectorCount);
            blink(Color::RED, 10);
            abort();
          } else if (pinResponse == 0 && expectedResponse == 'H') {
            myResult.fail(vectorCount);
            blink(Color::RED, 10);
            abort();
          }
        }
        blink(Color::GREEN, 10);
      }
    }

  private:
    Pattern myPattern;
    Result myResult;
};

void setup() {
  pinMode(11, OUTPUT); //RED
  pinMode(12, OUTPUT); //GREEN
  Serial.begin(9600);

  std::vector<Pin> myPins = {
    Pin(1, "DRIVE"),
    Pin(2, "DRIVE"),
    Pin(3, "RECEIVE"),
  };
  
  PinConfiguration myPinConfiguration(myPins);

  int maxNumberOfInputCombinations = pow(2, myPinConfiguration.getPinGroupByRole("DRIVE").size());

  std::vector<std::vector<char>> myPatternDataData = {
    {'0', '0', '0', '0', '0', '0', '0', '0', 'L', 'L', 'L', 'L'},
    {'0', '0', '0', '0', '0', '0', '0', '1', 'L', 'L', 'L', 'L'},
    {'0', '0', '0', '0', '0', '0', '1', '0', 'L', 'L', 'L', 'L'},
    {'0', '0', '0', '0', '0', '0', '1', '1', 'L', 'L', 'L', 'H'},
    {'0', '0', '0', '0', '0', '1', '0', '0', 'L', 'L', 'L', 'L'},
    {'0', '0', '0', '0', '0', '1', '0', '1', 'L', 'L', 'L', 'L'},
    {'0', '0', '0', '0', '0', '1', '1', '0', 'L', 'L', 'L', 'L'},
    {'0', '0', '0', '0', '0', '1', '1', '1', 'L', 'L', 'L', 'H'},
    {'0', '0', '0', '0', '1', '0', '0', '0', 'L', 'L', 'L', 'L'},
    {'0', '0', '0', '0', '1', '0', '0', '1', 'L', 'L', 'L', 'L'},
    {'0', '0', '0', '0', '1', '0', '1', '0', 'L', 'L', 'L', 'L'},
    {'0', '0', '0', '0', '1', '0', '1', '1', 'L', 'L', 'L', 'H'},
    {'0', '0', '0', '0', '1', '1', '0', '0', 'L', 'L', 'H', 'L'},
    {'0', '0', '0', '0', '1', '1', '0', '1', 'L', 'L', 'H', 'L'},
    {'0', '0', '0', '0', '1', '1', '1', '0', 'L', 'L', 'H', 'L'},
    {'0', '0', '0', '0', '1', '1', '1', '1', 'L', 'L', 'H', 'H'},
    {'0', '0', '0', '1', '0', '0', '0', '0', 'L', 'L', 'L', 'L'},
    {'0', '0', '0', '1', '0', '0', '0', '1', 'L', 'L', 'L', 'L'},
    {'0', '0', '0', '1', '0', '0', '1', '0', 'L', 'L', 'L', 'L'},
    {'0', '0', '0', '1', '0', '0', '1', '1', 'L', 'L', 'L', 'H'},
    {'0', '0', '0', '1', '0', '1', '0', '0', 'L', 'L', 'L', 'L'},
    {'0', '0', '0', '1', '0', '1', '0', '1', 'L', 'L', 'L', 'L'},
    {'0', '0', '0', '1', '0', '1', '1', '0', 'L', 'L', 'L', 'L'},
    {'0', '0', '0', '1', '0', '1', '1', '1', 'L', 'L', 'L', 'H'},
    {'0', '0', '0', '1', '1', '0', '0', '0', 'L', 'L', 'L', 'L'},
    {'0', '0', '0', '1', '1', '0', '0', '1', 'L', 'L', 'L', 'L'},
    {'0', '0', '0', '1', '1', '0', '1', '0', 'L', 'L', 'L', 'L'},
    {'0', '0', '0', '1', '1', '0', '1', '1', 'L', 'L', 'L', 'H'},
    {'0', '0', '0', '1', '1', '1', '0', '0', 'L', 'L', 'H', 'L'},
    {'0', '0', '0', '1', '1', '1', '0', '1', 'L', 'L', 'H', 'L'},
    {'0', '0', '0', '1', '1', '1', '1', '0', 'L', 'L', 'H', 'L'},
    {'0', '0', '0', '1', '1', '1', '1', '1', 'L', 'L', 'H', 'H'},
    {'0', '0', '1', '0', '0', '0', '0', '0', 'L', 'L', 'L', 'L'},
    {'0', '0', '1', '0', '0', '0', '0', '1', 'L', 'L', 'L', 'L'},
    {'0', '0', '1', '0', '0', '0', '1', '0', 'L', 'L', 'L', 'L'},
    {'0', '0', '1', '0', '0', '0', '1', '1', 'L', 'L', 'L', 'H'},
    {'0', '0', '1', '0', '0', '1', '0', '0', 'L', 'L', 'L', 'L'},
    {'0', '0', '1', '0', '0', '1', '0', '1', 'L', 'L', 'L', 'L'},
    {'0', '0', '1', '0', '0', '1', '1', '0', 'L', 'L', 'L', 'L'},
    {'0', '0', '1', '0', '0', '1', '1', '1', 'L', 'L', 'L', 'H'},
    {'0', '0', '1', '0', '1', '0', '0', '0', 'L', 'L', 'L', 'L'},
    {'0', '0', '1', '0', '1', '0', '0', '1', 'L', 'L', 'L', 'L'},
    {'0', '0', '1', '0', '1', '0', '1', '0', 'L', 'L', 'L', 'L'},
    {'0', '0', '1', '0', '1', '0', '1', '1', 'L', 'L', 'L', 'H'},
    {'0', '0', '1', '0', '1', '1', '0', '0', 'L', 'L', 'H', 'L'},
    {'0', '0', '1', '0', '1', '1', '0', '1', 'L', 'L', 'H', 'L'},
    {'0', '0', '1', '0', '1', '1', '1', '0', 'L', 'L', 'H', 'L'},
    {'0', '0', '1', '0', '1', '1', '1', '1', 'L', 'L', 'H', 'H'},
    {'0', '0', '1', '1', '0', '0', '0', '0', 'L', 'H', 'L', 'L'},
    {'0', '0', '1', '1', '0', '0', '0', '1', 'L', 'H', 'L', 'L'},
    {'0', '0', '1', '1', '0', '0', '1', '0', 'L', 'H', 'L', 'L'},
    {'0', '0', '1', '1', '0', '0', '1', '1', 'L', 'H', 'L', 'H'},
    {'0', '0', '1', '1', '0', '1', '0', '0', 'L', 'H', 'L', 'L'},
    {'0', '0', '1', '1', '0', '1', '0', '1', 'L', 'H', 'L', 'L'},
    {'0', '0', '1', '1', '0', '1', '1', '0', 'L', 'H', 'L', 'L'},
    {'0', '0', '1', '1', '0', '1', '1', '1', 'L', 'H', 'L', 'H'},
    {'0', '0', '1', '1', '1', '0', '0', '0', 'L', 'H', 'L', 'L'},
    {'0', '0', '1', '1', '1', '0', '0', '1', 'L', 'H', 'L', 'L'},
    {'0', '0', '1', '1', '1', '0', '1', '0', 'L', 'H', 'L', 'L'},
    {'0', '0', '1', '1', '1', '0', '1', '1', 'L', 'H', 'L', 'H'},
    {'0', '0', '1', '1', '1', '1', '0', '0', 'L', 'H', 'H', 'L'},
    {'0', '0', '1', '1', '1', '1', '0', '1', 'L', 'H', 'H', 'L'},
    {'0', '0', '1', '1', '1', '1', '1', '0', 'L', 'H', 'H', 'L'},
    {'0', '0', '1', '1', '1', '1', '1', '1', 'L', 'H', 'H', 'H'},
  };

  PatternData myPatternData(myPatternDataData);

  Pattern myPattern(myPinConfiguration, myPatternData);

  Result myResult;

  TestProgram myTestProgram(myPattern, myResult);

  myTestProgram.execute();
}

void loop() {  }

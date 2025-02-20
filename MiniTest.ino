#include <ArduinoSTL.h>

#include<math.h>

#include <iostream>
#include <vector>
#include <algorithm>

enum class Color {
  RED,
  GREEN,
  BOTH
};

enum class PinRole {
  DRIVE,
  RECEIVE
};

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

    GPIO getName() {
      return myPinName;
    }

    PinRole getRole() {
      return myPinRole;
    }

    bool operator==(const Pin& comparison) const {
      return this->myPinName == comparison.myPinName && myPinRole == comparison.myPinRole;
      // return true;
    }

    bool operator!=(const Pin& comparison) const {
      return this->myPinName != comparison.myPinName || myPinRole != comparison.myPinRole;
      // return true;
    }

  private:
    GPIO myPinName;
    PinRole myPinRole;
};

class PinConfiguration {
  public:
    PinConfiguration(std::vector<Pin> dataPins) {
      // assertThat(std::find(dataPins.begin(), dataPins.end(), pinName) != dataPins.end(), "Duplicate pin defined!");
      myPins = dataPins;
    }

    PinConfiguration(std::vector<Pin> dataPins, GPIO supplyPin, GPIO groundPin) {
      pinMode(gpioToPin(supplyPin), OUTPUT);
      digitalWrite(gpioToPin(supplyPin), HIGH);
      pinMode(gpioToPin(groundPin), INPUT);
      myPins = dataPins;
    }

    int getPinCount() {
      return myPins.size();
    }

    std::vector<Pin> getPinGroupByRole(PinRole role) {
      std::vector<Pin> pinGroup;
      for (Pin pin : myPins) {
        if(pin.getRole() == role) {
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

  private:
    std::vector<Pin> myPins;
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
    Pattern(PinConfiguration pinConfiguration, PatternData patternData) : myPinConfiguration(pinConfiguration), myPatternData(patternData) {
      // Assert that every vector in the pattern has the right length
    }

    std::vector<char> getVector(int vectorNumber) {
      assertThat(vectorNumber < myPatternData.getVectorCount() && vectorNumber >= 0, "Vector number out of range");
      assertThat(myPatternData.getVector(vectorNumber).size() == myPinConfiguration.getPinCount(), "Wrong number of elements in vector!");
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
      for (int i = 0; i < vectorCount; i++) {
        for (Pin pin : pinConfiguration.getPinGroupByRole(PinRole::DRIVE)) {
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
        delay(10);
        for (Pin pin : pinConfiguration.getPinGroupByRole(PinRole::RECEIVE)) {
          char expectedResponse = myPattern.getStateCharacter(i, pin);
          int pinResponse = digitalRead(gpioToPin(pin.getName()));
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
  Serial.println("Starting setup");

  std::vector<Pin> myPins = {
    Pin(GPIO::D1, PinRole::DRIVE),
    Pin(GPIO::D2, PinRole::DRIVE),
    Pin(GPIO::D3, PinRole::RECEIVE),
  };
  
  PinConfiguration myPinConfiguration(myPins);

  int maxNumberOfInputCombinations = pow(2, myPinConfiguration.getPinGroupByRole(PinRole::DRIVE).size());

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

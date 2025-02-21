#include <ArduinoSTL.h>

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
    for (int i = 0; i < 3; i++) {
      blink(Color::RED, 2);
      delay(100);
    }
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
  delay(duration * 100);
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
    }

    bool operator!=(const Pin& comparison) const {
      return this->myPinName != comparison.myPinName || myPinRole != comparison.myPinRole;
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
      // for (int i = 0; i < patternData.getVectorCount(); i++) {
      //   assertThat(patternData.getVector(i).size() == pinConfiguration.getPinCount(), "Invalid number of state characters in vector!");
      // }
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
      // assertThat(myPinConfiguration.hasPin(pin), "Pin is not defined in pin configuration");
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
    TestProgram(Pattern pattern, Result& result) : myPattern(pattern), myResult(result) {    }

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
          } else if (pinResponse == 0 && expectedResponse == 'H') {
            myResult.fail(vectorCount);
          }
        }
      }
    }

  private:
    Pattern myPattern;
    Result& myResult;
};

void setup() {
  pinMode(11, OUTPUT); //RED
  pinMode(12, OUTPUT); //GREEN

  std::vector<Pin> myPinsMinimal = {
    Pin(GPIO::D9, PinRole::DRIVE),
    Pin(GPIO::A1, PinRole::DRIVE),
    Pin(GPIO::A2, PinRole::RECEIVE),
  };
  
  std::vector<std::vector<char>> myPatternDataVectorsMinimal = {
    {'0', '0', 'L'},
    {'0', '1', 'L'},
    {'1', '0', 'L'},
    {'1', '1', 'H'}
  };
  
  PinConfiguration myPinConfiguration(myPinsMinimal);
  PatternData myPatternData(myPatternDataVectorsMinimal);
  Pattern myPattern(myPinConfiguration, myPatternData);
  
  Result myResult;

  TestProgram myTestProgram(myPattern, myResult);

  myTestProgram.execute();

  if (myResult.hasPassed()) {
    blink(Color::GREEN, 1);
  } else {
    blink(Color::RED, 1);
  }
}

void loop() {  abort();  }

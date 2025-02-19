#include <ArrayList.h>

const static int PIN_COUNT = 12;

bool assertThat(bool condition, String exception) {
  if (!condition) {
    Serial.println("Assertion error: '" + exception);
    abort();
  }
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
      return myPinName == comparison.myPinName && myPinRole == comparison.myPinRole;
    }

  private:
    int myPinName;
    String myPinRole;
};

class PinConfiguration {
  public:
    PinConfiguration(ArrayList<Pin> pins) {
      assertThat(pins.size() == PIN_COUNT, "Pin configuration has wrong number of pins!");

      for (Pin pin : pins) {
        int pinName = pin.getName();
        assertThat(!knownPinNames.contains(pinName), "Duplicate pin defined!");
        knownPinNames.add(pinName);
      }
      myPins = pins;
    }

    int getPinCount() {
      return myPins.size();
    }

    ArrayList<Pin> getPinGroupByRole(String role) {
      assertThat(role.equals("DRIVE") || role.equals("RECEIVE"), "Unknown pin role");
      ArrayList<Pin> pinGroup;
      for (Pin pin : myPins) {
        if(pin.getRole().equals(role)) {
          pinGroup.add(pin);
        }
      }
      return pinGroup;
    }

    bool hasPin(Pin pin) {
      return myPins.contains(pin);
    }

    int getPinIndex(Pin pin) {
      return myPins.indexOf(pin);
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
    ArrayList<Pin> myPins;
    ArrayList<int> knownPinNames;
};

class PatternData {
  public:
    PatternData(ArrayList<ArrayList<char>> data) {
      assertThat(data.size() > 0, "Pattern is empty!");

      ArrayList<char> allowedCharacters;
      allowedCharacters.add('0');
      allowedCharacters.add('1');
      allowedCharacters.add('L');
      allowedCharacters.add('H');
      allowedCharacters.add('X');

      for (ArrayList<char> vector : data) {
        assertThat(vector.size() == PIN_COUNT, "Vector has wrong number of elements");
        for (char stateCharacter : vector) {
          assertThat(allowedCharacters.contains(stateCharacter), "Invalid state character");
        }
      }

      myData = data;
    }

    ArrayList<char> getVector(int vectorNumber) {
      assertThat(vectorNumber < myData.size() && vectorNumber >= 0, "Vector position out of range");
      return myData.get(vectorNumber);
    }

    char getStateCharacter(int vectorNumber, int pinIndex) {
      assertThat(vectorNumber < myData.size() && vectorNumber >= 0, "Vector position out of range");
      return myData.get(vectorNumber).get(pinIndex);
    }

    int getVectorCount() {
      return myData.size();
    }

  private:
    ArrayList<ArrayList<char>> myData;
    int vectorLength;
};

class Pattern {
  public:
    Pattern(PinConfiguration pinConfiguration, PatternData patternData) : myPinConfiguration(pinConfiguration), myPatternData(patternData) {    }

    ArrayList<char> getVector(int vectorNumber) {
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
      failingVectors.add(failingVector);
    }

    bool hasPassed() {
      return passed;
    }

    bool hasFailed() {
      return failed;
    }

    ArrayList<int> getFailingVectors() {
      return failingVectors;
    }

  private:
    bool passed;
    bool failed;
    ArrayList<int> failingVectors = ArrayList<int>();
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
            case '1':
              digitalWrite(pin.getName(), HIGH);
          }
        }
        delay(10);
        for (Pin pin : pinConfiguration.getPinGroupByRole("RECEIVE")) {
          char expectedResponse = myPattern.getStateCharacter(i, pin);
          int pinResponse = digitalRead(pin.getName());
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
    Result myResult;
};

void setup() {
  Pin configuredPins[PIN_COUNT] = {
    Pin(1, "DRIVE"),
    Pin(2, "DRIVE"),
    Pin(3, "RECEIVE"),
    Pin(4, "DRIVE"),
    Pin(5, "DRIVE"),
    Pin(6, "RECEIVE"),
    Pin(7, "DRIVE"),
    Pin(8, "DRIVE"),
    Pin(9, "RECEIVE"),
    Pin(10, "DRIVE"),
    Pin(11, "DRIVE"),
    Pin(12, "RECEIVE")
  };

  ArrayList<Pin> thisConfiguration = ArrayList<Pin>();
  for (int i = 0; i < PIN_COUNT; i++) {
    thisConfiguration.add(configuredPins[i]);
  }
  
  PinConfiguration myPinConfiguration(thisConfiguration);

  int drivePins;
  for (Pin pin : thisConfiguration) {
    if (pin.getRole().equals("DRIVE")) {
      drivePins++;
    }
  }
  int maxNumberOfInputCombinations = 2^drivePins;

  char arrayData[maxNumberOfInputCombinations][PIN_COUNT] = {
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

  ArrayList<ArrayList<char>> thisData;
  for (auto &row : arrayData) {
    ArrayList<char> vector = ArrayList<char>();
    for (char stateCharacter : row) {
      vector.add(stateCharacter);
    }
    thisData.add(vector);
  }

  PatternData myPatternData(thisData);

  Pattern myPattern(myPinConfiguration, myPatternData);

  Result myResult;

  TestProgram myTestProgram(myPattern, myResult);

  myTestProgram.execute();
}

void setup() {
  Serial.begin(9600);
  Pin myPin(1, "ASDF");
  ArrayList<Pin> pinList = ArrayList<Pin>();
  pinList.add(myPin);
  PinConfiguration myPinConfig(pinList);
}

void loop() {  }

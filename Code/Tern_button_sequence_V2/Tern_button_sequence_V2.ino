// #include <LiquidCrystal_I2C.h>

// LiquidCrystal_I2C lcd(0x3F,16,2);  // set the LCD address to 0x3F for a 16 chars and 2 line display

// void setup() {
//   lcd.init();
//   lcd.clear();         
//   lcd.backlight();      // Make sure backlight is on
  
//   // Print a message on both lines of the LCD.
//   lcd.setCursor(2,0);   //Set cursor to character 2 on line 0
//   lcd.print("Hello world!");
  
//   lcd.setCursor(2,1);   //Move cursor to character 2 on line 1
//   lcd.print("LCD Tutorial");
// }

// void loop() {
// }

#include <LiquidCrystal_I2C.h>

// Define pin numbers for buttons and LEDs
const int startButtonPin = 2;
const int stopButtonPin = 3;
const int nextButtonPin = 4;

const int startLedPin = 5;
const int stopLedPin = 6;
const int nextLedPin = 7;

const int solenoidPins[] = {8, 9, 10, 11};

// Define program sequences
const int programSequences[5][4] = {
  {1, 2, 3, 4},
  {2, 3, 3, 4},
  {2, 4, 3, 2},
  {1, 2, 2, 2},
  {2, 3, 1, 4}
};

// LCD setup
LiquidCrystal_I2C lcd(0x3F, 16, 2);  // Set the LCD address to 0x3F for a 16 chars and 2-line display

void setup() {
  pinMode(startButtonPin, INPUT_PULLUP);
  pinMode(stopButtonPin, INPUT_PULLUP);
  pinMode(nextButtonPin, INPUT_PULLUP);

  pinMode(startLedPin, OUTPUT);
  pinMode(stopLedPin, OUTPUT);
  pinMode(nextLedPin, OUTPUT);

  for (int i = 0; i < 4; i++) {
    pinMode(solenoidPins[i], OUTPUT);
  }

  lcd.init();
  lcd.clear();
  lcd.backlight();  // Make sure the backlight is on
}

void turnOnSolenoid(int solenoidNumber) {
  digitalWrite(solenoidPins[solenoidNumber - 1], HIGH);
}

void turnOffSolenoids() {
  for (int i = 0; i < 4; i++) {
    digitalWrite(solenoidPins[i], LOW);
  }
}

void turnOnButtonLed(int ledPin) {
  digitalWrite(ledPin, HIGH);
}

void turnOffButtonLeds() {
  digitalWrite(startLedPin, LOW);
  digitalWrite(stopLedPin, LOW);
  digitalWrite(nextLedPin, LOW);
}

void displayMessage(const char *line1, const char *line2) {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(line1);
  lcd.setCursor(0, 1);
  lcd.print(line2);
}

void loop() {
  bool resetToStart = false;

  while (true) {
    displayMessage("Press Start", "");

    Serial.println("Waiting for Start button press");
    while (digitalRead(startButtonPin) == HIGH);

    for (int sequenceIndex = 0; sequenceIndex < 5; sequenceIndex++) {
      if (resetToStart) {
        resetToStart = false;
        break;
      }

      displayMessage("Running test", "");

      Serial.println("Starting new sequence");
      turnOnButtonLed(startLedPin);

      int previousSolenoid = -1;

      for (int solenoidIndex = 0; solenoidIndex < 4; solenoidIndex++) {
        int currentSolenoid = programSequences[sequenceIndex][solenoidIndex];

        Serial.print("Running solenoid ");
        Serial.println(currentSolenoid);

        displayMessage("Running test", "");

        // Check if the current solenoid is the same as the previous one
        if (currentSolenoid == previousSolenoid) {
          // Toggle LED for the repeated solenoid
          digitalWrite(solenoidPins[currentSolenoid - 1], LOW);
          delay(500);
          digitalWrite(solenoidPins[currentSolenoid - 1], HIGH);
        } else {
          // Turn off the LED for the previous solenoid
          if (previousSolenoid != -1) {
            digitalWrite(solenoidPins[previousSolenoid - 1], LOW);
          }
          // Turn on the LED for the current solenoid
          digitalWrite(solenoidPins[currentSolenoid - 1], HIGH);
        }

        turnOnSolenoid(currentSolenoid);
        delay(1000);
        turnOffSolenoids();

        if (digitalRead(stopButtonPin) == LOW) {
          Serial.println("Stop button pressed");
          turnOffButtonLeds();
          displayMessage("Press Next", "to Reset");
          resetToStart = true;
          break;
        }

        if (solenoidIndex == 3) {
          delay(500);
        }

        // Update the previous solenoid
        previousSolenoid = currentSolenoid;
      }

      if (!resetToStart) {
        displayMessage("Press Next", "");
      }

      Serial.println("Turning off LEDs after sequence");
      // Turn off the LED for the last solenoid in the sequence
      digitalWrite(solenoidPins[programSequences[sequenceIndex][3] - 1], LOW);

      Serial.println("Turning on Next LED");
      turnOnButtonLed(nextLedPin);

      bool nextButtonPressed = false;
      while (!nextButtonPressed) {
        if (digitalRead(nextButtonPin) == LOW) {
          Serial.println("Next button pressed");
          nextButtonPressed = true;
          delay(200);  // Debounce
        }
      }

      Serial.println("Turning off LEDs after sequence");
      // Turn off the LED for the last solenoid in the sequence
      digitalWrite(solenoidPins[programSequences[sequenceIndex][3] - 1], LOW);

      // Turn off Start LED after the sequence is complete
      turnOffButtonLeds();
    }

    Serial.println("Turning off LEDs after all sequences");
    // Turn off Next LED after all sequences are complete
    turnOffButtonLeds();
  }
}

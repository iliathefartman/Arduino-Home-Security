#include <Keypad.h>
#include <LiquidCrystal_I2C.h>
#include <Wire.h>

// Keypad connections
const byte ROWS = 4;
const byte COLS = 4;
char keys[ROWS][COLS] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};
byte rowPins[ROWS] = {22, 23, 24, 25};
byte colPins[COLS] = {26, 27, 28, 29};

const char customKeys[] = {'0', 'A', 'B', 'C', 'D', '#', '*'};
Keypad keypad = Keypad( customKeys, rowPins, colPins, ROWS, COLS );

// LCD connections
LiquidCrystal_I2C lcd(0x27, 16, 2);

// PIR and buzzer pins
const int pirPin = 30;
const int buzzerPin = 31;

// System variables
String passcode = "CCCC"; // Set your passcode here
String enteredPasscode = "";
bool systemArmed = false;
unsigned long alarmStartTime = 0;
const unsigned long alarmDuration = 10000; // 10 seconds
unsigned long lastActivityTime = 0;
const unsigned long idleTimeout = 10000; // 10 seconds
bool isIdle = false;

void setup() {
  Serial.begin(9600);

  lcd.init();
  lcd.backlight();
  updateLCDDisplay();

  pinMode(pirPin, INPUT);
  pinMode(buzzerPin, OUTPUT);
}

void loop() {
  char key = keypad.getKey();

  if (key) {
    handleKeyPress(key);
    Serial.print("Key Pressed: ");
    Serial.println(key);
    lastActivityTime = millis();
    if (isIdle) {
      isIdle = false;
      lcd.backlight();
      updateLCDDisplay();
    }
  }

  if (!isIdle && (millis() - lastActivityTime >= idleTimeout)) {
    isIdle = true;
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Idle");
    lcd.noBacklight();
  }

  if (systemArmed && digitalRead(pirPin) == HIGH) {
    if (alarmStartTime == 0) {
      alarmStartTime = millis();
    }
    tone(buzzerPin, 1000);
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("INTRUDER ALERT!");
  } else {
    noTone(buzzerPin);
    if (alarmStartTime > 0 && (millis() - alarmStartTime >= alarmDuration)) {
      alarmStartTime = 0;
      updateLCDDisplay(); 
    }
  }
}

void handleKeyPress(char key) {
  beep();
  if (key == '#') {
    handleArmDisarm();
  } 
}

void handleArmDisarm() {
  if (systemArmed) {
    lcd.setCursor(0, 1);
    lcd.print("Enter Code:    ");
    enteredPasscode = "";

    while (enteredPasscode.length() < passcode.length()) {
      char disarmingKey = keypad.getKey();
      if (disarmingKey) {
        beep();
        enteredPasscode += disarmingKey;
        lcd.print("*");
        Serial.print(disarmingKey); // (Optional debugging)
      }
    }

    if (enteredPasscode == passcode) {
      systemArmed = false;
      enteredPasscode = "";
      updateLCDDisplay();
      Serial.println("System Disarmed"); // (Optional)
    } else {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Incorrect Code");
      delay(1000);
      enteredPasscode = "";
      updateLCDDisplay(); 
      Serial.println("Incorrect Code Entered"); // (Optional)
    }
  } else {
    systemArmed = true;
    updateLCDDisplay();
    Serial.println("System Armed"); // (Optional)
  }
}

void updateLCDDisplay() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Room Security");
  lcd.setCursor(0, 1);
  lcd.print(systemArmed ? "Armed" : "Disarmed");
}

void beep() {
  tone(buzzerPin, 2000);
  delay(100);
  noTone(buzzerPin);
}
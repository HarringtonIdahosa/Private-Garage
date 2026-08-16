#include <Servo.h>
#include <LiquidCrystal.h>
#include <Keypad.h>

// LCD
LiquidCrystal lcd(7, 8, 9, 10, 11, 12);

// KEYPAD
const byte ROWS = 4;
const byte COLS = 4;

char keys[ROWS][COLS] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};

byte rowPins[ROWS] = {30, 31, 32, 33};
byte colPins[COLS] = {34, 35, 36, 37};

Keypad keypad = Keypad(
  makeKeymap(keys),
  rowPins,
  colPins,
  ROWS,
  COLS
);

// GARAGE COMPONENTS

const int trigPin = 22;
const int echoPin = 23;

const int servoPin = 24;

const int greenLED = 25;
const int redLED = 26;

const int buzzer = 27;

Servo gateServo;

// GARAGE SETTINGS

const int closedPosition = 0;
const int openPosition = 90;

const int detectionDistance = 15;

// PASSWORD

const String correctPIN = "2026";

String enteredPIN = "";

// SETUP

void setup() {

  Serial.begin(9600);

  // LCD
  lcd.begin(16, 2);

  // Ultrasonic
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  // LEDs
  pinMode(greenLED, OUTPUT);
  pinMode(redLED, OUTPUT);

  // Buzzer
  pinMode(buzzer, OUTPUT);

  // Servo
  gateServo.attach(servoPin);
  gateServo.write(closedPosition);

  // Turn everything off
  digitalWrite(greenLED, LOW);
  digitalWrite(redLED, LOW);
  digitalWrite(buzzer, LOW);

  // Startup
  lcd.setCursor(0, 0);
  lcd.print("AUTO GARAGE");

  lcd.setCursor(0, 1);
  lcd.print("Starting...");

  delay(2000);

  lcd.clear();

  lcd.setCursor(0, 0);
  lcd.print("Garage Ready");

  lcd.setCursor(0, 1);
  lcd.print("Waiting...");
}


void loop() {

  long distance = getDistance();

  Serial.print("Distance: ");
  Serial.print(distance);
  Serial.println(" cm");

 // CAR DETECTED
 
  if (distance > 0 && distance <= detectionDistance) {

    enteredPIN = "";

    lcd.clear();

    lcd.setCursor(0, 0);
    lcd.print("CAR DETECTED");

    delay(1000);

    enterPassword();
  }

  delay(100);
}

// ENTER PASSWORD

void enterPassword() {

  enteredPIN = "";

  lcd.clear();

  lcd.setCursor(0, 0);
  lcd.print("ENTER PIN:");

  lcd.setCursor(0, 1);

  while (true) {

    char key = keypad.getKey();

    if (key) {

      Serial.print("Key: ");
      Serial.println(key);

      // NUMBER

      if (key >= '0' && key <= '9') {

        // Don't allow more than 4 digits
        if (enteredPIN.length() < 4) {

          enteredPIN += key;

          lcd.setCursor(0, 1);

          // Display asterisk for every digit
          for (int i = 0; i < enteredPIN.length(); i++) {
            lcd.print("*");
          }
        }
      }

      // CLEAR

      else if (key == '*') {

        enteredPIN = "";

        lcd.setCursor(0, 1);
        lcd.print("                ");

        lcd.setCursor(0, 1);
      }

      // SUBMIT

      else if (key == '#') {

        // Check password
        if (enteredPIN == correctPIN) {

          accessGranted();

          return;

        } else {

          accessDenied();

          enteredPIN = "";

          lcd.clear();

          lcd.setCursor(0, 0);
          lcd.print("ENTER PIN:");

          lcd.setCursor(0, 1);
        }
      }
    }

    delay(20);
  }
}

// ACCESS GRANTED

void accessGranted() {

  Serial.println("ACCESS GRANTED");

  lcd.clear();

  lcd.setCursor(0, 0);
  lcd.print("ACCESS GRANTED");

  lcd.setCursor(0, 1);
  lcd.print("WELCOME!");

  // Green LED
  digitalWrite(greenLED, HIGH);
  digitalWrite(redLED, LOW);

  // Two success beeps
  tone(buzzer, 2000);
  delay(150);
  noTone(buzzer);

  delay(100);

  tone(buzzer, 2500);
  delay(200);
  noTone(buzzer);

  delay(1000);

  
  // OPEN GATE
  
  lcd.clear();

  lcd.setCursor(0, 0);
  lcd.print("ACCESS GRANTED");

  lcd.setCursor(0, 1);
  lcd.print("OPENING GATE");

  gateServo.write(openPosition);

  delay(1500);

  
  // GATE OPEN
  lcd.clear();

  lcd.setCursor(0, 0);
  lcd.print("GATE OPEN");

  lcd.setCursor(0, 1);
  lcd.print("DRIVE THROUGH");

  // Wait for car
  waitForCarToLeave();

  // CLOSE GATE
  lcd.clear();

  lcd.setCursor(0, 0);
  lcd.print("CLOSING GATE");

  gateServo.write(closedPosition);

  delay(1000);

  digitalWrite(greenLED, LOW);

  
  // READY
  

  lcd.clear();

  lcd.setCursor(0, 0);
  lcd.print("Garage Ready");

  lcd.setCursor(0, 1);
  lcd.print("Waiting...");
}


// ACCESS DENIED

void accessDenied() {

  Serial.println("ACCESS DENIED");

  lcd.clear();

  lcd.setCursor(0, 0);
  lcd.print("ACCESS DENIED");

  lcd.setCursor(0, 1);
  lcd.print("WRONG PIN");

  // Red LED
  digitalWrite(redLED, HIGH);
  digitalWrite(greenLED, LOW);

  // Warning beep
  tone(buzzer, 500);
  delay(700);
  noTone(buzzer);

  delay(1500);

  digitalWrite(redLED, LOW);
}

// ULTRASONIC DISTANCE

long getDistance() {

  digitalWrite(trigPin, LOW);

  delayMicroseconds(2);

  digitalWrite(trigPin, HIGH);

  delayMicroseconds(10);

  digitalWrite(trigPin, LOW);

  long duration = pulseIn(echoPin, HIGH, 30000);

  if (duration == 0) {
    return -1;
  }

  long distance = duration * 0.0343 / 2;

  return distance;
}

// WAIT FOR CAR TO LEAVE

void waitForCarToLeave() {

  Serial.println("....");

  unsigned long startTime = millis();

  while (true) {

    long distance = getDistance();

    // Car moved away
    if (distance > detectionDistance || distance == -1) {

      delay(1000);

      distance = getDistance();

      if (distance > detectionDistance || distance == -1) {
        break;
      }
    }

    // Safety timeout
    if (millis() - startTime > 15000) {

      break;
    }

    delay(200);
  }
}

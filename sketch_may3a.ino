#include <Wire.h>
#include <Adafruit_PN532.h>
#include <Keypad.h>
#include "rgb_lcd.h"
#include <Servo.h>
#include "pitches.h"
#include <DHT.h>

#define SERVO_PIN 25
#define BUZZER_PIN  15
#define DHT11_PIN  19
#define PN532_SDA  21 // Define SDA pin
#define PN532_SCL  22 // Define SCL pin

Servo servoMotor;
DHT dht11(DHT11_PIN, DHT11);

#define ROW_NUM 4     // four rows
#define COLUMN_NUM 3  // three columns

int melody[] = {
  NOTE_C4
};

int noteDurations[] = {
  4
};

char keys[ROW_NUM][COLUMN_NUM] = {
  { '1', '2', '3' },
  { '4', '5', '6' },
  { '7', '8', '9' },
  { '*', '0', '#' }
};

rgb_lcd lcd;

const int colorR = 255;
const int colorG = 0;
const int colorB = 0;

byte pin_rows[ROW_NUM] = { 18, 5, 17, 16 };  // GPIO18, GPIO5, GPIO17, GPIO16 connect to the row pins
byte pin_column[COLUMN_NUM] = { 4, 0, 2 };   // GPIO4, GPIO0, GPIO2 connect to the column pins

Keypad keypad = Keypad(makeKeymap(keys), pin_rows, pin_column, ROW_NUM, COLUMN_NUM);

const String password = "7890";  // change your password here
String input_password;

Adafruit_PN532 nfc(PN532_SDA, PN532_SCL); // Instantiate PN532 with SDA and SCL pins

void setup() {
  Serial.begin(9600);
  input_password.reserve(32);  // maximum input characters is 33, change if needed

  // Set up the LCD's number of columns and rows:
  lcd.begin(16, 2);
  lcd.setRGB(colorR, colorG, colorB);

  // Print a message to the LCD.
  lcd.print("Scan RFID card");

  servoMotor.attach(SERVO_PIN);
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW); // Turn off the buzzer initially
  dht11.begin();

  // Initialize the PN532 module
  nfc.begin();
  nfc.SAMConfig();
}

void loop() {
  // Read temperature in Celsius
  float tempC = dht11.readTemperature();
  // RFID card detection logic
  uint8_t success;
  uint8_t uid[] = { 0, 0, 0, 0, 0, 0, 0 };
  uint8_t uidLength;

  success = nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength);

  if (success) {
    Serial.println("Found an RFID card!");
    // Display temperature on LCD
    lcd.clear();
    lcd.print("Temp: ");
    if (isnan(tempC)) {
      lcd.print("Error"); // Display error message if temperature reading is not valid
    } else {
      lcd.print(tempC);
      lcd.print(" C");
    }

    lcd.setCursor(0, 1); // Move cursor to the beginning of the second line
    lcd.print("Enter PIN");

    // Wait for the user to enter the PIN
    input_password = "";  // clear input password
    while (input_password.length() < password.length()) {
      char key = keypad.getKey();
      if (key) {
        input_password += key;  // append new character to input password string
        lcd.print("*");  // Display "*" for each entered digit
        delay(200);  // Delay to prevent multiple inputs for a single key press
      }
    }

    // Check if the entered PIN matches the stored password
    if (input_password == password) {
      Serial.println("The password is correct, ACCESS GRANTED!");
      lcd.clear();
      lcd.print("Access granted");
      delay(2000);
      lcd.clear();
      lcd.print("Door opening");
      for (int pos = 0; pos <= 180; pos += 1) {
        // In steps of 1 degree
        servoMotor.write(pos);
        delay(15); // Waits 15ms to reach the position
      }
      delay(4000);
      lcd.clear();
      lcd.print("Door closing");
      for (int pos = 180; pos >= 0; pos -= 1) {
        servoMotor.write(pos);
        delay(15); // Waits 15ms to reach the position
      }
      ESP.restart();
    } else {
      Serial.println("The password is incorrect, ACCESS DENIED!");
      lcd.clear();
      lcd.print("Access denied");
      digitalWrite(BUZZER_PIN, HIGH); // Turn on the buzzer for error sound
      // Play error sound
      for (int thisNote = 0; thisNote < 8; thisNote++) {
        int noteDuration = 1000 / noteDurations[thisNote];
        tone(BUZZER_PIN, melody[thisNote], noteDuration);
        delay(noteDuration * 1.30);
        noTone(BUZZER_PIN);
        delay(50); // Adding a short pause between notes for better sound quality
      }
      digitalWrite(BUZZER_PIN, LOW); // Turn off the buzzer after playing error sound
      lcd.clear();
      lcd.print("Please scan RFID");
      delay(2000);
      lcd.clear();
      lcd.print("Scan RFID card");
    }
  }
}


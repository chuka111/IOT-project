#include <Keypad.h>
#include <Wire.h>
#include "rgb_lcd.h"
#include <Servo.h>
#include "pitches.h"

#define SERVO_PIN 26
#define BUZZZER_PIN  15

Servo servoMotor;

#define ROW_NUM 4     // four rows
#define COLUMN_NUM 3  // three columns

int melody[] = {
  NOTE_C4, NOTE_G3, NOTE_G3, NOTE_A3, NOTE_G3, 0, NOTE_B3, NOTE_C4
};

int noteDurations[] = {
  4, 8, 8, 4, 4, 4, 4, 4
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

void setup() {
  Serial.begin(9600);
  input_password.reserve(32);  // maximum input characters is 33, change if needed

  // set up the LCD's number of columns and rows:
    lcd.begin(16, 2);

    lcd.setRGB(colorR, colorG, colorB);

    // Print a message to the LCD.
   lcd.println("Enter pin");

    //delay(1000);
  servoMotor.attach(SERVO_PIN);
  pinMode(BUZZZER_PIN, OUTPUT);
  digitalWrite(BUZZZER_PIN,LOW);
}

void loop() {
  char key = keypad.getKey();

  if (key) {
    Serial.print(key);
    lcd.setCursor(0, 1);
    lcd.print(key);

    if (key == '*') {
      input_password = "";  // clear input password
      //lcd.clear();
    } else if (key == '#') {
      if (password == input_password) {
        Serial.println("The password is correct, ACCESS GRANTED!");
        lcd.println("Correct!!");
        delay(2000);
        lcd.clear();
        lcd.print("door opening");
        for (int pos = 0; pos <= 180; pos += 1) {
        // in steps of 1 degree
          servoMotor.write(pos);
          delay(15); // waits 15ms to reach the position
        }
        delay(4000);

        lcd.clear();
        lcd.print("door closing");

        for (int pos = 180; pos >= 0; pos -= 1) {
          servoMotor.write(pos);
          delay(15); // waits 15ms to reach the position
        } 
        ESP.restart();
        // DO YOUR WORK HERE

      } else {
        Serial.println("The password is incorrect, ACCESS DENIED!");
        lcd.println("Incorrect!!");
          

          for (int thisNote = 0; thisNote < 8; thisNote++) {
            digitalWrite(BUZZZER_PIN,HIGH);
            int noteDuration = 1000 / noteDurations[thisNote];
            tone(BUZZZER_PIN, melody[thisNote], noteDuration);

            int pauseBetweenNotes = noteDuration * 1.30;
            delay(pauseBetweenNotes);
            noTone(BUZZZER_PIN);
           
          }
           delay(2000);
          digitalWrite(BUZZZER_PIN,LOW);
          ESP.restart();
      }

      input_password = "";  // clear input password
    } else {
      input_password += key;  // append new character to input password string
    }
  }

}
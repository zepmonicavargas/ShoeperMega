#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <SPI.h>
#include <MFRC522.h>

#define SS_PIN 53
#define RST_PIN 5
MFRC522 rfid(SS_PIN, RST_PIN);

LiquidCrystal_I2C lcd(0x27, 16, 2); 

#define coinSlot 47
#define uvc 35
#define blower 31
#define fog 37
#define pump 3
#define led 2


int IN3 = 41;
int IN4 = 39;
int ENB = 10;
int IN1 = 45;
int IN2 = 43;
int ENA = 26;
int coinSlotStatus;
int pulse = 0;
void disinfect();

void setup() {
  Serial.begin(9600);
  SPI.begin();
  rfid.PCD_Init();
  Serial.println("Place your RFID card near the scanner...");
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(ENA, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);
  pinMode(ENB, OUTPUT);
  // Start with the motor stopped
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  analogWrite(ENA, 0); // Speed 0 (stopped)
  pinMode(coinSlot, INPUT_PULLUP);
  pinMode(uvc, OUTPUT);
  pinMode(fog, OUTPUT);
  pinMode(pump, OUTPUT);
  pinMode(blower, OUTPUT);
  pinMode(led, OUTPUT);
  lcd.init();             // Initialize the LCD
  lcd.backlight();        // Turn on the backlight
  lcd.setCursor(0, 0);    // Set the cursor to column 0, line 0
  //lcd.print("Hello, World!"); // Print text
  //lcd.setCursor(0, 1);    // Set the cursor to column 0, line 1
  //lcd.print("I2C LCD Test");
  //delay(2000);
  //lcd.clear();
  pinMode(ENA, OUTPUT);
  analogWrite(ENA, 0);  // Ensure it starts with 0 (off)
  
  //digitalWrite(uvc, HIGH);
  digitalWrite(uvc, HIGH);  // Turn off the LED
  digitalWrite(fog, LOW); // Turn on the LED
  digitalWrite(blower,LOW);
  digitalWrite(pump, HIGH);
}
unsigned long previousMillis = 0;
const long interval = 200; // Check RFID every 200 ms
bool cashMode = false;  // Flag to keep track of cash input

void loop() {
  digitalWrite(led, HIGH);
  // Check for serial input
  if (Serial.available() > 0) {
    String input = Serial.readString(); // Read the input string
    input.trim(); // Remove any leading or trailing whitespace

    // Activate cash mode when input is "cash"
    if (input == "cash") {
      cashMode = true;
      digitalWrite(led, LOW);
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Balance:");
    }

    // Activate RFID mode when input is "rfid"
    else if (input == "rfid") {
      cashMode = false;  // Stop cash mode
      digitalWrite(led, LOW);
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Scan RFID Tag");

      // Stay in this loop until RFID tag is detected
      while (true) {
        if (rfid.PICC_IsNewCardPresent() && rfid.PICC_ReadCardSerial()) {
          // Print the RFID serial number directly
          for (byte i = 0; i < rfid.uid.size; i++) {
            Serial.print(rfid.uid.uidByte[i], HEX);
            if (i < rfid.uid.size - 1) {
              Serial.print(" ");  // Print a space between bytes
            }
          }
          Serial.println();  // New line after the complete UID

          // End communication with card and call your function
          rfid.PICC_HaltA(); // End communication with card
          break;             // Exit loop after tag is detected
        }
      }
    }
    if (input == "start"){
        disinfect();
      }
  }

  // Cash mode handling
  if (cashMode) {
    digitalWrite(led, LOW);
    coinSlotStatus = digitalRead(coinSlot);
    // Serial.println(coinSlotStatus);  // Print continuously to monitor

    if (coinSlotStatus == 0) {
      pulse += 1;
      lcd.clear();
      lcd.setCursor(0, 1);
      Serial.println("1");  // Print "1" for each pulse received
      delay(30);  // Debounce delay

      if (pulse == 20) {
        delay(5000);
        pulse = 0;
        return;
      }
    }
  }
}

void disinfect(){

    
    digitalWrite(uvc, LOW); // Turn on the LED
    digitalWrite(fog, HIGH); // Turn on the LED
    digitalWrite(pump, HIGH); // Turn on the LED
    digitalWrite(IN1, HIGH);
    digitalWrite(IN2, LOW);
    analogWrite(ENA, 255);
    delay(5000);             // Keep the LED on for 5 seconds
    digitalWrite(uvc, HIGH);  // Turn off the LED

    digitalWrite(fog, LOW); // Turn on the LED
    digitalWrite(pump, LOW); // Turn on the LED
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, LOW);
    analogWrite(ENA, 0);
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Disinfecting");
    digitalWrite(led, HIGH); // Turn on the LED
    digitalWrite(blower, HIGH); // Turn on the LED
    digitalWrite(IN3, HIGH);
    digitalWrite(IN4, LOW);
    analogWrite(ENB, 255);
    delay(5000);             // Keep the LED on for 5 seconds

    digitalWrite(led, LOW);  // Turn off the LED
    digitalWrite(blower, LOW); // Turn on the LED
    digitalWrite(IN3, LOW);
    digitalWrite(IN4, LOW);
    analogWrite(ENB, 0);
    Serial.println("finish");
    lcd.clear();
}
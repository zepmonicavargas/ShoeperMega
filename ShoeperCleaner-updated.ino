#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <SPI.h>
#include <MFRC522.h>
#include <SoftwareSerial.h>


#define SS_PIN 53
#define RST_PIN 49
MFRC522 rfid(SS_PIN, RST_PIN);

LiquidCrystal_I2C lcd(0x27, 16, 2); 

#define coinSlot 47
#define uvc 35
#define blower 31
#define fog 37
#define ozone 33
#define pump 3
#define led 2
#define Motor1 23
#define Motor2 25
#define Motor3 27
#define air 29
#define fogpower 4
#define floatswitch 6

int IN3 = 41; // exhaust
int IN4 = 39; // exhaust

int coinSlotStatus;
int pulse = 0;
bool wasHigh = true;  

SoftwareSerial sim800l(17, 18);

void disinfect();

void setup() {
  Serial.begin(115200);
  sim800l.begin(9600);
  SPI.begin();
  rfid.PCD_Init();
  delay(100);
  Serial.println("Place your RFID card near the scanner...");
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);
  pinMode(coinSlot, INPUT);
  pinMode(uvc, OUTPUT);
  pinMode(fog, OUTPUT);
  pinMode(pump, OUTPUT);
  pinMode(ozone, OUTPUT);
  pinMode(blower, OUTPUT);
  pinMode(led, OUTPUT);
  pinMode(Motor1, OUTPUT);
  pinMode(Motor2, OUTPUT);
  pinMode(Motor3, OUTPUT);
  pinMode(air, OUTPUT);
  pinMode(fogpower, OUTPUT);
  pinMode(floatswitch,INPUT_PULLUP);

  digitalWrite(uvc, HIGH);  // Turn off the LED
  digitalWrite(fog, LOW); // Turn on the LED
  digitalWrite(blower, HIGH);
  digitalWrite(Motor1, HIGH);
  digitalWrite(Motor2, HIGH);
  digitalWrite(Motor3, HIGH);
  digitalWrite(fogpower, LOW); //
  digitalWrite(pump, HIGH); 

  delay(1000);
  sim800l.println("AT");
  delay(1000);
  sim800l.println("AT+CMGF=1"); // Set SMS to text mode
  delay(1000);
  
}
void sendSMS(String message) {
  sim800l.println("AT+CMGF=1");                      // Ensure SMS text mode
  delay(500);
  sim800l.println("AT+CMGS=\"+639153704339\"");       // Your Globe number in international format
  delay(500);
  sim800l.print(message);                             // Message content
  delay(500);
  sim800l.write(26);                                  // ASCII code for CTRL+Z to send SMS
  delay(5000);                                        // Wait for SMS to send
}

unsigned long lastCoinPulseTime = 0;
const unsigned long coinDebounceDelay = 30; 
unsigned long previousMillis = 0;
const long interval = 200; // Check RFID every 200 ms
bool cashMode = false;  // Flag to keep track of cash input

int lastStableState = HIGH;  // Assume float is HIGH at start
int lastReading = HIGH;
unsigned long lastDebounceTime = 0;
const unsigned long debounceDelay = 50;

void loop() {
  digitalWrite(led, HIGH);
  
  // Check for serial input
  if (Serial.available() > 0) {
    String input = Serial.readString(); // Read the input string
    input.trim(); // Remove any leading or trailing whitespace

    // Activate cash mode when input is "cash"
    if (input == "cash") {
      cashMode = true;
    }

    // Activate RFID mode when input is "rfid"
    else if (input == "rfid") {
      cashMode = false;
      digitalWrite(led, LOW);
      // Serial.println("yes");

      rfid.PCD_Init();        // Initialize
      rfid.PCD_Reset();       // Reset the MFRC522
      rfid.PCD_Init();        // Initialize again after reset
      rfid.PCD_AntennaOn();   // Turn on the antenna
      
      // Stay in this loop until an RFID tag is detected or "back" is received
      while (true) {
        delay(10);
        
        // Check if RFID tag is present
        if (rfid.PICC_IsNewCardPresent() && rfid.PICC_ReadCardSerial()) {
          Serial.print("<"); // Start marker
          for (byte i = 0; i < rfid.uid.size; i++) {
              // Optional: add a leading zero for single digit hex numbers
              if (rfid.uid.uidByte[i] < 0x10) Serial.print("0");
              Serial.print(rfid.uid.uidByte[i], HEX);
              if (i < rfid.uid.size - 1) {
                  Serial.print(" ");
              }
          }
          Serial.println(">"); // End marker

          rfid.PICC_HaltA(); // End communication with card
          break;          // Exit the RFID waiting loop after tag is detected
        }
        
        // Check for serial input to break out of RFID waiting mode
        if (Serial.available() > 0) {
          String backInput = Serial.readString();
          backInput.trim();
          if (backInput == "back") {
            // Optionally, provide feedback like Serial.println("Returning to main loop.");
            break;  // Exit the RFID waiting loop
          }
        }
        
        delay(100); // Slight delay to reduce CPU usage
      }
    }
    
    if (input == "start"){
        disinfect();
        //Serial.println("finish");
        //return;
      }

   else if (input == "level") {
     delay(1000);
      while (true) {
        int currentReading = digitalRead(floatswitch);

        if (currentReading != lastReading) {
          lastDebounceTime = millis();  // reset debounce timer
          lastReading = currentReading;
        }

        // If the reading has stayed stable for debounceDelay
        if ((millis() - lastDebounceTime) > debounceDelay) {
          if (currentReading != lastStableState) {
            lastStableState = currentReading;

            if (lastStableState == HIGH) {
              Serial.println("chemlow");
              sendSMS("Chemical Low detected!");
            } else {
              Serial.println("OK");
            }

            delay(500); // Short delay to ensure clean transmission
          }
        }

        // Check for exit command from Serial
        if (Serial.available()) {
          String command = Serial.readStringUntil('\n');
          command.trim();
          if (command == "back") {
            break;
          }
        }

        delay(10); // Small delay to avoid busy loop
      }
    }





  // Cash mode handling
  while (cashMode) {
    digitalWrite(led, LOW);
    coinSlotStatus = digitalRead(coinSlot);

    if (coinSlotStatus == LOW) { // Assuming the pulse pulls the line LOW (0)
        if (millis() - lastCoinPulseTime > coinDebounceDelay) {
          pulse += 1;
          Serial.println("1");  // Print "1" for each pulse received
          lastCoinPulseTime = millis();
        }
      


      if (pulse == 40) {
        //delay(5000);
        pulse = 0;
        cashMode = false;
        return;
      }
    }

    // Listen for Serial input to exit cash mode
    if (Serial.available() > 0) {
      String backInput = Serial.readString();
      backInput.trim();
      if (backInput == "back") {
        //Serial.println("Exiting cash mode...");
        cashMode = false;
        return;  // Exit cashMode loop
      }
    }

    //delay(50); // Optional delay to reduce CPU usage
  }
}
}

void disinfect(){
  //pump on start and uvc motors on 
    digitalWrite(uvc, LOW);
    digitalWrite(pump, LOW); 
    digitalWrite(Motor1, LOW);
    digitalWrite(Motor2, LOW);
    digitalWrite(Motor3, LOW);
    
    delay(5000);

  // pump off
    digitalWrite(pump, HIGH);
    delay(25000); 

  //motors off ozone on
    digitalWrite(Motor1, HIGH);
    digitalWrite(Motor2, HIGH);
    digitalWrite(Motor3, HIGH);
    digitalWrite(ozone, HIGH);
    digitalWrite(air, HIGH);  
    delay(300000);
 
  //ozone off drying on and exhaust

    digitalWrite(ozone, LOW);
    digitalWrite(fogpower, HIGH);
    digitalWrite(led, HIGH); 
    digitalWrite(blower, LOW); 
    digitalWrite(IN3, HIGH); // exhaust
    digitalWrite(IN4, LOW); // exhaust
    delay(180000);            

  // fog on 30 seconds; off drying and exhaust

    digitalWrite(air, LOW);
    digitalWrite(fog, HIGH);
    digitalWrite(blower, HIGH); 
    digitalWrite(IN3, LOW); // exhaust
    digitalWrite(IN4, LOW); // exhaust
    delay(60000);
    
  // exhaust on and blower on final drying; off fog and power

    digitalWrite(fog, LOW);
    digitalWrite(fogpower, LOW);
    digitalWrite(blower, LOW); 
    digitalWrite(IN3, HIGH); // exhaust
    digitalWrite(IN4, LOW); // exhaust
    delay(30000);

  // all done off all components
    digitalWrite(uvc,HIGH);
    digitalWrite(blower, HIGH); 
    digitalWrite(IN3, LOW); // exhaust
    digitalWrite(IN4, LOW); // exhaust
    Serial.println("finish");
}

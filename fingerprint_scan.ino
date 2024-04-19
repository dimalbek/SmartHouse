#include <Adafruit_Fingerprint.h>
#include <SoftwareSerial.h>
#include <Servo.h>
#include <LiquidCrystal_I2C.h>
#include <Wire.h> 
#include <OneWire.h>
#include <DallasTemperature.h>
#include <RtcDS1302.h>
#include <SPI.h>
#include <SD.h>

// SD card adapter setup
File myFile; // declaration of file for logging to SD card
const int chipSelect = 53; // CS pin for SD card

// RTC setup
ThreeWire myWire(6,7,5); // IO, SCLK, CE
RtcDS1302<ThreeWire> Rtc(myWire);

// FingerPrint Sensor setup
SoftwareSerial mySerial(10, 11);  // RX, TX
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);
uint8_t id = 2; // Starting ID for enrolling fingerprints
Servo myservo; // servo declaration
// Button pin declarations
const int addButtonPin = 2;  // The pin number connected to the add button
const int readButtonPin = 3;  // The pin number connected to the read button

// LCD I2C address setup for a 16 chars and 2 line display
LiquidCrystal_I2C lcd(0x27, 16, 2);

// temperature sensor connection
#define ONE_WIRE_BUS 13

// LED pins connection 
#define LED_ABOVE_28 8 // LED that turns on when temp is above 28°C
#define LED_BELOW_26 9 // LED that turns on when temp is below 26°C
// Setup a oneWire instance to communicate with any OneWire devices
OneWire oneWire(ONE_WIRE_BUS);

// Pass our oneWire reference to Dallas Temperature sensor 
DallasTemperature sensors(&oneWire);

void setup() {
  Serial.begin(9600);
  
  while (!Serial); // Wait for serial port to connect
  Serial.println("\n\nFingerprint sensor setup");

  // SD card initialization
  pinMode(SS, OUTPUT);

  if (!SD.begin(chipSelect)) {
    Serial.println("Initialization failed!");
    return;
  }

  // Open the file
  myFile = SD.open("logss.txt", FILE_WRITE);
  if (myFile) {
    Serial.println("File opened successfully in setup.");
//    myFile.close();
  } else {
    Serial.println("Failed to open file in setup.");
  }

  // Set the button pins as input
  pinMode(addButtonPin, INPUT);  
  pinMode(readButtonPin, INPUT);

  // Start temperature sensor and LCD initialization
  sensors.begin();
  lcd.init();
  lcd.backlight();

  

  // Initialize LED pins as outputs and turn them off
  pinMode(LED_ABOVE_28, OUTPUT);
  digitalWrite(LED_ABOVE_28, LOW);
  pinMode(LED_BELOW_26, OUTPUT);
  digitalWrite(LED_BELOW_26, LOW);

  delay(500);  // Short delay might help stabilize input readings

  // Initialize fingerprint sensor
  finger.begin(57600);
  if (finger.verifyPassword()) {
    Serial.println("Found fingerprint sensor!");
    lcd.print("Sensor detected");
  } else {
    Serial.println("Did not find fingerprint sensor :(");
    lcd.print("Sensor error");
    while (1) { delay(1); }
  }
  
  // servo attach and initial position setup
  myservo.attach(4);
  myservo.write(0);


  Serial.println("Temperature Sensor Ready");

  // RTC initialization
  Rtc.Begin();

  RtcDateTime compiled = RtcDateTime(__DATE__, __TIME__);
  printDateTime(compiled);
  Serial.println();
    
  if (!Rtc.IsDateTimeValid()) {
        Serial.println("RTC lost confidence in the DateTime!");
        Rtc.SetDateTime(compiled);
    }

    if (Rtc.GetIsWriteProtected()) {
        Serial.println("RTC was write protected, enabling writing now");
        Rtc.SetIsWriteProtected(false);
    }

    if (!Rtc.GetIsRunning()) {
        Serial.println("RTC was not actively running, starting now");
        Rtc.SetIsRunning(true);
    }

    RtcDateTime now = Rtc.GetDateTime();
    if (now < compiled) {
        Serial.println("RTC is older than compile time!  (Updating DateTime)");
        Rtc.SetDateTime(compiled);
    } else if (now > compiled) {
        Serial.println("RTC is newer than compile time. (this is expected)");
    } else if (now == compiled) {
        Serial.println("RTC is the same as compile time! (not expected but all is fine)");
    }
}
uint8_t readnumber(void) {
  uint8_t num = 0;
  
  while (num == 0) {
    while (! Serial.available());
    num = Serial.parseInt();
  }
  return num;
}
const int debounceDelay = 50;  // Delay in milliseconds to debounce the button
unsigned long lastDebounceTimeAdd = 0;  // Last time the output pin was toggled
unsigned long lastDebounceTimeRead = 0;  // Last time the output pin was toggled
bool lastButtonStateAdd = HIGH;  // the previous reading from the input pin
bool lastButtonStateRead = HIGH;  // the previous reading from the input pin

void loop() {
// Temperature measurement and display logic
  sensors.requestTemperatures(); 
  float tempC = sensors.getTempCByIndex(0);
  
  lcd.clear();
  lcd.setCursor(0, 0);

  if(tempC != DEVICE_DISCONNECTED_C) 
  {
    lcd.print("Temp: ");
    lcd.print(tempC);
    lcd.print(" C");
    Serial.print("Temperature: ");
    Serial.print(tempC);
    Serial.println(" C");

    // Temperature-based LED control
    if(tempC > 28) {
      digitalWrite(LED_ABOVE_28, HIGH); // Turn on the LED for above 28°C
      digitalWrite(LED_BELOW_26, LOW);  // Ensure the other LED is off
    } 
    else if(tempC < 26) {
      digitalWrite(LED_ABOVE_28, LOW);  // Ensure the above 28°C LED is off
      digitalWrite(LED_BELOW_26, HIGH); // Turn on the LED for below 26°C
    } 
    else {
      // In the 26°C to 28°C range, turn both LEDs off
      digitalWrite(LED_ABOVE_28, LOW);
      digitalWrite(LED_BELOW_26, LOW);
    }
  }
  else {
    lcd.print("Error: No Sensor");
    Serial.println("Error: DS18B20 sensor disconnected");
    // Turn off both LEDs if there's a sensor error
    digitalWrite(LED_ABOVE_28, LOW);
    digitalWrite(LED_BELOW_26, LOW);
  }
  
  



//RTC date and time update
    RtcDateTime now = Rtc.GetDateTime();

    printDateTime(now);
    Serial.println();

    if (!now.IsValid()) {
        Serial.println("RTC lost confidence in the DateTime!");
    }

    delay(1000); // Wait before the next reading



// Fingerprint Sensor logic
  
  
  int currentButtonStateAdd = digitalRead(addButtonPin);
  int currentButtonStateRead = digitalRead(readButtonPin);

  // Only toggle the LED if the new button state is HIGH and different from the last state
  if ((millis() - lastDebounceTimeAdd) > debounceDelay) {
    if (currentButtonStateAdd != lastButtonStateAdd) {
      lastDebounceTimeAdd = millis();  // reset the debouncing timer
      if (currentButtonStateAdd == HIGH) {

        lcd.clear();
        lcd.setCursor(0, 0);
//        lcd.print();
        
        lcd.print("Enrolling...");
        Serial.println("Ready to enroll a fingerprint!");
        Serial.print("Enrolling ID #");
        Serial.println(id);
        enrollFingerprint();
        id++;
        delay(1000);
      }
    }
  }
  
  if ((millis() - lastDebounceTimeRead) > debounceDelay) {
    if (currentButtonStateRead != lastButtonStateRead) {
      lastDebounceTimeRead = millis();  // reset the debouncing timer
      if (currentButtonStateRead == HIGH) {
        lcd.clear();
        lcd.setCursor(0, 0);
//        lcd.print();


        
        lcd.print("Reading...");
        Serial.println("Place finger\n");

        readFingerprint();
        
      }
    }
  }

  lastButtonStateAdd = currentButtonStateAdd;
  lastButtonStateRead = currentButtonStateRead;
}

// print DateTime function
void printDateTime(const RtcDateTime& dt) {
    char datestring[20];

    snprintf_P(datestring, 
            countof(datestring),
            PSTR("%02u/%02u/%04u %02u:%02u"),
            dt.Month(),
            dt.Day(),
            dt.Year(),
            dt.Hour(),
            dt.Minute());

    Serial.println("Date:"); // Print a label for the date
    lcd.setCursor(0, 1); // Move to the start of the second line
    lcd.print(datestring);
    Serial.println(datestring); // Print the formatted date and time
}

// fingerprint enrollment start
void enrollFingerprint() {
  Serial.println("Enrolling new fingerprint...");
  getFingerprintEnroll();
}

// fingerprint read start
void readFingerprint() {
  // Place your code for reading a fingerprint here
  Serial.println("Reading fingerprint...");
  // Assuming `getFingerprintIDez` is a function you've defined for reading
  getFingerprintIDez();
}


// Check if fingerprint already exists
uint8_t getFingerprintID() {
  uint8_t p = finger.getImage();
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image taken");
      break;
    case FINGERPRINT_NOFINGER:
      Serial.println("No finger detected");
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      return p;
    case FINGERPRINT_IMAGEFAIL:
      Serial.println("Imaging error");
      return p;
    default:
      Serial.println("Unknown error");
      return p;
  }

  // OK success!

  p = finger.image2Tz();
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image converted");
      break;
    case FINGERPRINT_IMAGEMESS:
      Serial.println("Image too messy");
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      return p;
    case FINGERPRINT_FEATUREFAIL:
      Serial.println("Could not find fingerprint features");
      return p;
    case FINGERPRINT_INVALIDIMAGE:
      Serial.println("Could not find fingerprint features");
      return p;
    default:
      Serial.println("Unknown error");
      return p;
  }
  
  // OK converted!
  p = finger.fingerFastSearch(); // Check if fingerprint already in system
  if (p == FINGERPRINT_OK) {

    Serial.println("Found a print match!");
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    Serial.println("Communication error");
    return p;
  } else if (p == FINGERPRINT_NOTFOUND) {
    Serial.println("Did not find a match");
    return p;
  } else {
    Serial.println("Unknown error");
    return p;
  }   
  
  // found a match
  Serial.print("Found ID #"); Serial.print(finger.fingerID); 
  
  Serial.print(" with confidence of "); Serial.println(finger.confidence); 
  
//  lcd.print(finger.fingerID);
  return finger.fingerID;
}

// returns -1 if failed, otherwise returns ID #
int getFingerprintIDez() {
 
  int p = FINGERPRINT_NOFINGER;
  while (p == FINGERPRINT_NOFINGER) {
    p = finger.getImage();
  }
  if (p != FINGERPRINT_OK)  return -1;
  p = finger.image2Tz();
  if (p != FINGERPRINT_OK)  return -1;

  p = finger.fingerFastSearch();
  if (p != FINGERPRINT_OK){
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Failed.Try again");
    return -1;
  }  
  
  // found a match
  Serial.print("Found ID #"); Serial.print(finger.fingerID); 
  Serial.print(" with confidence of "); Serial.println(finger.confidence);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Opened");
  logEvent("Door opened", finger.fingerID); // Log door opening to SD

  myservo.write(90); // open servo lock
  delay(4000); // wait for 4 seconds
  myservo.write(0); // close servo lock
//  delay(1000);
  return finger.fingerID; 
}



// enroll new fingerprint into system
uint8_t getFingerprintEnroll() {

  int p = -1;
  Serial.print("Waiting for valid finger to enroll as #"); Serial.println(id);
  while (p != FINGERPRINT_OK) {
    p = finger.getImage(); // get finger image
    switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image taken");
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Image taken"); // Notify using LCD that image was taken
      break;
    case FINGERPRINT_NOFINGER:
      Serial.println(".");
      break;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      break;
    case FINGERPRINT_IMAGEFAIL:
      Serial.println("Imaging error");
      break;
    default:
      Serial.println("Unknown error");
      break;
    }
  }

  // OK success!

  p = finger.image2Tz(1);
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image converted");
      break;
    case FINGERPRINT_IMAGEMESS:
      Serial.println("Image too messy");
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      return p;
    case FINGERPRINT_FEATUREFAIL:
      Serial.println("Could not find fingerprint features");
      return p;
    case FINGERPRINT_INVALIDIMAGE:
      Serial.println("Could not find fingerprint features");
      return p;
    default:
      Serial.println("Unknown error");
      return p;
  }
  
  Serial.println("Remove finger");
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Remove finger");
  delay(2000);
  p = 0;
  while (p != FINGERPRINT_NOFINGER) {
    p = finger.getImage();
  }
  Serial.print("ID "); Serial.println(id);
  p = -1;
  Serial.println("Place same finger again");
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Place again");
  while (p != FINGERPRINT_OK) {
    p = finger.getImage();
    switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image taken");
      break;
    case FINGERPRINT_NOFINGER:
      Serial.print(".");
      break;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      break;
    case FINGERPRINT_IMAGEFAIL:
      Serial.println("Imaging error");
      break;
    default:
      Serial.println("Unknown error");
      break;
    }
  }

  // OK success!

  p = finger.image2Tz(2);
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image converted");
      break;
    case FINGERPRINT_IMAGEMESS:
      Serial.println("Image too messy");
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      return p;
    case FINGERPRINT_FEATUREFAIL:
      Serial.println("Could not find fingerprint features");
      return p;
    case FINGERPRINT_INVALIDIMAGE:
      Serial.println("Could not find fingerprint features");
      return p;
    default:
      Serial.println("Unknown error");
      return p;
  }
  
  // OK converted!
  Serial.print("Creating model for #");  Serial.println(id);
  
  p = finger.createModel();
  if (p == FINGERPRINT_OK) {
    Serial.println("Prints matched!");
    logEvent("Finderprint added", finger.fingerID); // Log fingerprint addition to SD
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    Serial.println("Communication error");
    return p;
  } else if (p == FINGERPRINT_ENROLLMISMATCH) {
    Serial.println("Fingerprints did not match");
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Missmatch");
    delay(1000);
    
    return p;
  } else {
    Serial.println("Unknown error");
    return p;
  }   
  
  Serial.print("ID "); Serial.println(id);
  p = finger.storeModel(id); // Store fingerprint
  if (p == FINGERPRINT_OK) {
    Serial.println("Stored!");
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Stored");
//    logEvent("Fingerprint added", id);
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    Serial.println("Communication error");
    return p;
  } else if (p == FINGERPRINT_BADLOCATION) {
    Serial.println("Could not store in that location");
    return p;
  } else if (p == FINGERPRINT_FLASHERR) {
    Serial.println("Error writing to flash");
    return p;
  } else {
    Serial.println("Unknown error");
    return p;
  }   
}


void logEvent(const char* event, int id) {
  RtcDateTime now = Rtc.GetDateTime();  // Get current DateTime
//  myFile = SD.open("test.txt", FILE_WRITE);

  if (myFile) {
    Serial.println("IN MY FILE");

    char datestring[32]; // datetime in readable format
    snprintf(datestring, sizeof(datestring), "%02u/%02u/%04u %02u:%02u",
             now.Month(), now.Day(), now.Year(), now.Hour(), now.Minute());

    //  ID to the SD
    myFile.print("ID: ");
    myFile.print(id);
    myFile.print(", ");
    
    // event description SD
    myFile.print(event);
    myFile.print(", ");

    // formatted datetime to SD
    myFile.println(datestring);
    Serial.println("WE ADDED INTO LOG FILE");
    myFile.flush();
//    myFile.close();
    delay(1000);
    Serial.println("Event logged and file closed.");\
    
  } else {
    Serial.println("Error opening file to log event.");
  }
}

// Close SD File
void closeFile() {
  if (myFile) {
    myFile.close();
    Serial.println("File closed successfully.");
  }
}

# SmartHouse
This project involves creating a multifunctional Arduino-based system that integrates various components to perform tasks related to security, environmental monitoring, and data logging. The core functionalities include:

Fingerprint Authentication: Utilizes an Adafruit Fingerprint sensor to manage access control. Users can enroll new fingerprints, and existing fingerprints can be used to authenticate users for access.

Temperature Monitoring: Features a Dallas Temperature sensor to continuously monitor and report the ambient temperature. The system activates different LEDs based on the temperature thresholds (above 28°C and below 26°C), providing visual alerts for temperature conditions.

Real-Time Clock (RTC): Employs an RtcDS1302 module to maintain accurate time and date, which is crucial for timestamping logged data and managing time-sensitive operations.

Servo Motor Control: Integrates a servo motor that can be activated upon successful fingerprint verification, typically used to control a lock or other mechanisms requiring physical movement.

LCD Display: Utilizes a LiquidCrystal_I2C display to provide real-time feedback and system status, displaying temperature readings, fingerprint sensor status, and other operational messages.

SD Card Data Logging: Includes functionality to log system events such as access attempts and system status updates to an SD card, allowing for historical data review and management.

Button Interface: Features physical buttons for triggering specific operations like enrolling a new fingerprint or manually reading a fingerprint.

This system combines hardware interfacing, real-time data handling, and environmental monitoring, making it suitable for applications such as secure access control systems and environmental data logging stations.


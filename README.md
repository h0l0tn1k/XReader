## Welcome to XReader page

This page contains information about arduino project that I'm currently working on.

### XReader - under development

XReader is controller for Arduino UNO which in conjuction with PN532 NFC/RFID module and further in the future with finger print sensor will act as door gate. This project is for my diploma thesis. 

### Links
- Arduino source code
  - https://github.com/h0l0tn1k/XReader
- Android app source code
  - https://github.com/h0l0tn1k/MultiAccess

### Hardware components
- Arduino UNO 
- Set of Male to Female cables
- PN532 RFID & NFC reader
- Buzzer
- 5V to 12V Step up DC converter
  - for Buzzer, K212 & Solenoid Lock
- Solenoid lock
- Capacitive switches
- K212 control board
- R305 fingerprint reader
- RFID cards/tokens
- Android smartphone with NFC
- LEDs
- 3D Printer
- PLA fillament

## Progress
1. Draft Android application created:
  - Capable of storing (new/old) keys encrypted and decrypted by fingerprint reader
2. Arduino connected with PN532 reader
3. Draft Arduino Controller created:
  - Capable of Defining Master Card, registering & storing new cards, determining if card is registered or not
4. K212 with R305 powered from arduino using Step Up DC Converter 
5. Case for testing purposes
6. Initial memory storage handling
7. LED's controlling
8. Buzzer sounds handling
9. Opening door using pin 3 and connecting it with transistor to K212 Boar's OPEN signal

#pragma once
#include <Adafruit_PN532.h>
#include "EEPROMStorageHandler.h"

#define PN532_SS   (10)
#define INVALID_SOUND (261)
#define VALID_SOUND (3000)
#define DOOR_OPENED_INTERVAL  3000

class XReader {
private:
	Adafruit_PN532* _board;
  unsigned char _blueLed = 8;
  unsigned char _greenLed = 7;
  unsigned char _redLed = 9;
  unsigned char _buzzer = 2;
  EEPROMStorageHandler* _eepromStorage;
  unsigned int _consecutiveFails = 0;
public:
	XReader();
	void begin();
	void checkConnectionToPn532();
	void loopProcedure();
private:
	void initBoard() const;
	static void switchOnLed(unsigned char ledPin);
	static void switchOffLed(unsigned char ledPin);
	void soundUnsuccessAuthBuzzer() const;
	void switchSuccessAuthBuzzerOn() const;
};

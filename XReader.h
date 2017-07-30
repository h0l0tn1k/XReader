#pragma once
#include <Adafruit_PN532.h>
#include "EEPROMStorageHandler.h"

#define PN532_SS   (10)
class XReader {
private:
	Adafruit_PN532* _board;
  char _blueLed = 8;
  char _greenLed = 7;
  EEPROMStorageHandler* _eepromStorage;
public:
	XReader();
	void begin();
	void checkConnectionToPn532();
	void loopProcedure();
private:
	void initBoard();
};

#pragma once
#include <Adafruit_PN532.h>
#include "EEPROMStorageHandler.h"

#define PN532_SS   (10)
#define INVALID_SOUND (261)
#define VALID_SOUND (3000)
#define DOOR_OPENED_INTERVAL  3000
//#define DEBUG

class XReader {
private:
  Adafruit_PN532*		_board;
  const unsigned char	_blueLed = 8;
  const unsigned char	_greenLed = 7;
  const unsigned char	_redLed = 9;
  const unsigned char	_buzzer = 2;
  EEPROMStorageHandler* _eepromStorage;
  unsigned int			_consecutiveFails = 0;
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


	void unsuccessfulAuth();
	void successfulAuth();
	void registeringNewCard(uint8_t* uid, uint8_t uidLength);
};

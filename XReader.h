#pragma once
#include <Adafruit_PN532.h>
#include "EEPROMStorageHandler.h"

#define PN532_SS   (10)
#define INVALID_SOUND (261)
#define VALID_SOUND (3000)
#define DOOR_OPENED_INTERVAL  5000
//#define DEBUG

class XReader {

  Adafruit_PN532*		_board;
  EEPROMStorageHandler* _eepromStorage;

  const unsigned char	_blueLedPin = 8;
  const unsigned char	_greenLedPin = 7;
  const unsigned char	_redLedPin = 9;
  const unsigned char   _openDoorPin = 3;
  const unsigned char	_buzzerPin = 2;

  unsigned int			_consecutiveFails = 0;

public:
	/**
	* \brief Constructor: Initializes class variables
	*/
	XReader();

	/**
	* \brief Initializes variables, pins and starts PN532 board
	*/
	void begin();

	/**
	* \brief Reads from PN532 in loop
	*/
	void loopProcedure();

private:
	void initBoard() const;

	/**
	* \brief Checks whether connection to PN532 was successful, if yes, inits board otherwise halts
	*/
	void checkConnectionToPn532() const;

	/**
	* \brief Switches on Pin at \param ledPin
	* \param ledPin Pin to be switched on
	*/
	static void switchPinOn(unsigned char ledPin);

	/**
	* \brief Switches off Pin at \param ledPin
	* \param ledPin Pin to be switched off
	*/
	static void switchPinOff(unsigned char ledPin);

	/**
	* \brief Switches on buzzer sound for unsuccessful authorization, shuts down after 1s
	*/
	void soundUnsuccessAuthBuzzerOn() const;

	/**
	* \brief Switches on buzzer sound for successful authorization, shuts down after 1s
	*/
	void switchSuccessAuthBuzzerOn() const;


	/**
	* \brief Handles unsuccessful RFID Card authorization to XReader by sounding buzzer, switching LED's and by delaying of next attempt
	*/
	void unsuccessfulAuth();

	/**
	* \brief Handles successful authorization to XReader by sounding buzzer, triggering of door opening and switching LED's
	*/
	void successfulAuth();

	/**
	* \brief Handles registration of new card after master card was successfully read, switches LED's and buzzer
	*/
	void registeringNewCard();

	void openDoor() const;
};

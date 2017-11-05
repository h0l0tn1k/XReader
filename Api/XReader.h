#pragma once
#include <Adafruit_PN532.h>
#include "EEPROMStorageHandler.h"
#include "SoundHelper.h"
#include "LedHelper.h"
#include "Structs/RfidCard.h"

#define DOOR_OPENED_INTERVAL  5000

class XReader : protected PinHelper {

  Adafruit_PN532*		_board;
  EEPROMStorageHandler* _eepromStorage;
  ISoundHelper*			_soundHelper;
  ILedHelper*			_ledHelper;

  const uint8_t	_button1Pin = 4;
  const uint8_t _openDoorPin = 3;

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

	/**
	 * \brief Inits various settings for PN532 board
	 */
	void initBoard() const;

	/**
	* \brief Checks whether connection to PN532 was successful, if yes, calls initBoard method otherwise halts
	*/
	void checkConnectionToPn532() const;
	
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

	/**
	 * \brief Sends HIGH signal to pin \a _openDoorPin which opens the door
	 */
	void openDoor() const;
};

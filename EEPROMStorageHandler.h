#pragma once
#include <EEPROM.h>
#include "HardwareSerial.h"

typedef unsigned long int uint32_m;

class EEPROMStorageHandler {

private:

	//MEMORY
	//	  0		|	1		2		3		4		5
	//#numOfRec#|#########MASTERCARD############|###CARDS Start HERE

	const uint8_t _masterCardAddress = 1;
	const uint8_t _baseAddress = 5;
	unsigned char _numberOfRecords = 0;
	uint32_m _masterCardId;
	HardwareSerial* _serial;

public:
	EEPROMStorageHandler(HardwareSerial* serial);
	void writeMasterCard(uint8_t* cardId);
	bool isMasterCard(uint8_t* cardId);
	void registerNewCard(uint8_t * cardId);
	bool isCardRegistered(uint8_t * cardId);
private:
	uint32_m getCardAtIndex(unsigned char i);
	void setNumberOfStoredCards(unsigned char newNumber);
	unsigned char getNumberOfStoredCards();
	uint32_m getMasterCardId();
	uint32_m convertToInt32(uint8_t * uid);

};

#pragma once
#include <EEPROM.h>
#include "HardwareSerial.h"

class EEPROMStorageHandler {

private:

	//MEMORY
	//	  0		|	1		2		3		4		5
	//#numOfRec#|#########MASTERCARD############|###CARDS Start HERE

	unsigned int _masterCardAddress = 1;
	unsigned int _baseAddress = 5;
	unsigned char _numberOfRecords = 0;
	HardwareSerial* _serial;

public:
	EEPROMStorageHandler(HardwareSerial* serial);
	void writeMasterCard(uint8_t* cardId);
	bool isMasterCard(uint8_t* cardId);
	void registerNewCard(uint8_t * cardId);
	bool isCardRegistered(uint8_t * cardId);
private:
	unsigned long int getCardAtIndex(unsigned char i);
	void setNumberOfStoredCards(unsigned char newNumber);
	unsigned char getNumberOfStoredCards();
	unsigned long int getMasterCardId();
	unsigned long int convertToInt32(uint8_t * uid);

};

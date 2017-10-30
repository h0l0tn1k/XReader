#pragma once
#include <EEPROM.h>
#include "HardwareSerial.h"

typedef unsigned long int uint32_m;


class EEPROMStorageHandler {

private:

	//MEMORY
	//	  0		|	1		2		3		4		5		6		7	|	 8	  |		9		10		11		12	|
	//#numOfRec#|#########MASTERCARD####################################|#########|#################################|


	//	  13      14       15       16	  |	  17      18       19       20	   |	 21      22       23     24	  |   25     26     27    28    |
	//###############Pnew4################|###############Pnew7################|###############PIN################|#########OCCUPANCY###########|


	// [8]			Mastercard 4/7B indicator, 0 = 4B, 1 = 7B
	// [9-12]		4/7B sector indicator, each bit represents type of 28B block, 0 = 4B, 1 = 7B, index specifies which block
	// [13-16]		Pnew4 - 4B pointer to place in memory where new 4B uid will be stored
	// [17-20]		Pnew7 - 4B pointer to place in memory where new 7B uid will be stored
	// [21-24]		PIN
	// [25-28]		Block occupation

	// [29-1023]	28B blocks / 35 of them
		
	//TODO: below
		
	//bool saveNew4BCard(uint8_t * uid);
	//bool delete4BCard(uint8_t * uid);

	//bool saveNew7BCard(uint8_t * uid);
	//bool delete7BCard(uint8_t * uid);

	unsigned char _numberOfRecords = 0;
	uint32_t _new4BCardAddress = 0;
	uint32_t _new7BCardAddress = 0;
	uint8_t _masterCardId[7] = { 0, 0, 0, 0, 0, 0, 0 };

	const unsigned char _masterCardBaseAddress = 1;
	const unsigned char _masterCardSizeBaseAddress = 8;
	const unsigned char _4B7BBlocksBaseAddress = 9;
	const unsigned char _4BpointerBaseAddress = 13;
	const unsigned char _7BpointerBaseAddress = 17;
	const unsigned char _pinBaseAddress = 21;
	const unsigned char _blockOccupationBaseAddress = 25;
	const unsigned char _cardBlockBaseAddress = 29;
	

public:
	EEPROMStorageHandler();
	bool isMasterCard(uint8_t* uid, uint8_t uid_length) const;
	void registerNewCard(uint8_t * cardId, uint8_t uid_length);
	bool isCardRegistered(uint8_t * cardId, uint8_t uid_length);
	void setCardBlockType(unsigned char block_index, bool is7ByteBlock);
	bool isCardBlock7B(unsigned char block_index);
	
	//TODO: DELETE AFTERWARDS
	static void deleteMemory();
	void setMasterCard(uint8_t* uid, uint8_t uid_length);
	void setMasterCardSizeIndicator(bool is7Byte);  // private
	void registerNew7BCard(uint8_t* cardId);
	void registerNew4BCard(uint8_t* cardId);
	uint32_t getNew4BCardAddress();
	uint32_t getNew7BCardAddress();
	void printMemory();

	bool isNthBitTrue(unsigned char, unsigned char) const;
//private:

	//Number of stored cards
	void setNumberOfCards(unsigned char count);
	unsigned char getNumberOfCards();
	void increaseNumberOfCards();
	void decreaseNumberOfCards();

	//Master Card
	uint8_t* loadMasterCardId();
	uint8_t* getMasterCardId();
	bool getMasterCardSizeIndicator();  // private
	bool isBlockOccupied(unsigned char block_index);

	bool areArraysEqual(uint8_t* array1, uint8_t* array2, uint8_t size) const;

	static uint32_m convertToInt32(uint8_t * uid);


	bool checkPinEquals(uint32_t pin_entered);
	void setPin(const uint32_t pin_entered, uint32_t new_pin);


	void setBlockOccupationType(unsigned char block_index, bool isOccupied);
	unsigned char getBlockOccupationIndicator(unsigned char block_index);
	//4B/7B Block indicators
	unsigned char getCardBlockIndicator(unsigned char block_index);
	bool isCardInBlock(const unsigned char block_index, uint8_t* cardId, const uint8_t uid_length) const;
};

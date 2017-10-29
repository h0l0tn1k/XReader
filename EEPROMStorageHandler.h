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


	//uint32_t getNew4BCardAddress();
	//void shift4BCardAddress(); - to new empty space

	//uint32_t getNew7BCardAddress();
	//void shift7BCardAddress(); - to new empty space
	
	//bool saveNew4BCard(uint8_t * uid);
	//bool delete4BCard(uint8_t * uid);

	//bool saveNew7BCard(uint8_t * uid);
	//bool delete7BCard(uint8_t * uid);

	unsigned char _numberOfRecords = 0;
	const unsigned char _masterCardBaseAddress = 1;
	const unsigned char _masterCardSizeBaseAddress = 8;
	const unsigned char _4B7BBlocksBaseAddress = 9;
	const uint32_t _4BpointerBaseAddress = 13;
	const uint32_t _7BpointerBaseAddress = 17;
	const uint32_t _pinBaseAddress = 21;
	const uint32_t _blockOccupationBaseAddress = 25;
	const uint32_t _cardBlockBaseAddress = 29;
	uint32_t _new4BCardAddress = 0;
	uint32_t _new7BCardAddress = 0;
	
	//TODO 8B
	uint32_m _masterCardId;
	HardwareSerial* _serial;

public:
	EEPROMStorageHandler(HardwareSerial* serial);
	bool isMasterCard(uint8_t* uid, uint8_t uid_length) const;
	void registerNewCard(uint8_t * cardId, uint8_t uid_length);
	bool isCardRegistered(uint8_t * cardId, uint8_t uid_length);
	void setCardBlockType(unsigned char block_index, bool is7ByteBlock);
	bool isCardBlock7B(unsigned char block_index);
	
	//TODO: DELETE AFTERWARDS
	static void deleteMemory();
	void setMasterCard(uint8_t* uid, uint8_t uid_length);
	static void setMasterCardSizeIndicator(bool is7Byte);  // private
	uint32_m getMasterCardId();
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
	bool getMasterCardSizeIndicator();  // private
	bool isBlockOccupied(unsigned char block_index);

	static uint32_m convertToInt32(uint8_t * uid);


	bool checkPinEquals(uint32_t pin_entered);
	void setPin(uint32_t new_pin);


	void setBlockOccupationType(unsigned char block_index, bool isOccupied);
	unsigned char getBlockOccupationIndicator(unsigned char block_index);
	//4B/7B Block indicators
	unsigned char getCardBlockIndicator(unsigned char block_index);
	bool isCardInBlock(const unsigned char block_index, const uint8_t* cardId, const uint8_t uid_length) const;
};

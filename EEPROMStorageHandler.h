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
	//###############Pnew4################|###############Pnew7################|###############PIN################|####EMPTYYYYYYY##############|


	// [8]			Mastercard 4/7B indicator, 0 = 4B, 1 = 7B
	// [9-12]		4/7B sector indicator, each bit represents type of 28B block, 0 = 4B, 1 = 7B, index specifies which block
	// [13-16]		Pnew4 - 4B pointer to place in memory where new 4B uid will be stored
	// [17-20]		Pnew7 - 4B pointer to place in memory where new 7B uid will be stored
	// [21-24]		PIN
	// [25-28]		!!!!EMPTY!!!!!! - Mem alignment

	// [29-1023]	28B blocks / 35 of them
		
	//TODO: below

	//void setCardBlockType(uint8_t block_index, bool is7ByteBlock);
	//bool isCardBlock7B(uint8_t block_index);

	//uint32_t getNew4BCardAddress();
	//void shift4BCardAddress(); - to new empty space

	//uint32_t getNew7BCardAddress();
	//void shift7BCardAddress(); - to new empty space

	//bool checkPin(uint32_t pin_entered);
	//void changePin(uint32_t new_pin);	

	//bool saveNew4BCard(uint8_t * uid);
	//bool delete4BCard(uint8_t * uid);

	//bool saveNew7BCard(uint8_t * uid);
	//bool delete7BCard(uint8_t * uid);

	unsigned char _numberOfRecords = 0;
	
	//TODO 8B
	uint32_m _masterCardId;
	HardwareSerial* _serial;

public:
	EEPROMStorageHandler(HardwareSerial* serial);
	bool isMasterCard(uint8_t* uid, uint8_t uid_length);
	void registerNewCard(uint8_t * cardId);
	bool isCardRegistered(uint8_t * cardId);
private:
	uint32_m getCardAtIndex(unsigned char i);

	//Number of stored cards
	void setNumberOfCards(unsigned char count);
	unsigned char getNumberOfCards();
	void increaseNumberOfCards();
	void decreaseNumberOfCards();


	bool isMasterCard(uint8_t* uid, uint8_t uid_length);
	void setMasterCard(uint8_t* uid, uint8_t uid_length);
	void setMasterCardSizeIndicator(bool is7Byte);  // private
	bool getMasterCardSizeIndicator();  // private


	uint32_m getMasterCardId();
	uint32_m convertToInt32(uint8_t * uid);

};

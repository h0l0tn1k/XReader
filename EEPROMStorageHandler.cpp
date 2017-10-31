#include "EEPROMStorageHandler.h"

//#define DEBUG

EEPROMStorageHandler::EEPROMStorageHandler()
{
	_numberOfRecords = eeprom_read_byte(0);
	loadMasterCardId();
	_new4BCardAddress = eeprom_read_dword((uint32_t*)_4BpointerBaseAddress);
	_new7BCardAddress = eeprom_read_dword((uint32_t*)_7BpointerBaseAddress);
}

#pragma region Helpers

uint32_m EEPROMStorageHandler::convertToInt32(uint8_t* uid) {
	return (uint32_m(uid[0]) << 24) | (uint32_m(uid[1]) << 16) | (uint32_m(uid[2]) << 8) | uint32_m(uid[3]);
}

bool EEPROMStorageHandler::areArraysEqual(uint8_t* array1, uint8_t* array2, uint8_t size) const {
	for (unsigned short i = 0; i < size; i++)
	{
		if (array1[i] != array2[i]) {
			return false;
		}
	}
	return true;
}

bool EEPROMStorageHandler::isNthBitTrue(unsigned char byte, unsigned char bit_index) const
{
	/*
	Bullshit because we can have e.g occupation block 11111111
	if (byte == 255) {
		return false;
	}*/
	const unsigned char bit_position = bit_index % 8;
	const unsigned char bit = (bit_position > 0) ? (1 << bit_position) : 1;
	
	return (byte & bit) > 0;
}

void EEPROMStorageHandler::deleteMemory()
{
	for (int i = 0; i < 1024; ++i)
	{
		eeprom_write_byte(i, 0);
	}

	eeprom_write_byte(0, 0);
}

#pragma endregion 

#pragma region Master Card

bool EEPROMStorageHandler::getMasterCardSizeIndicator() 
{
	//true -> 7B, false -> 4B
	return bool(eeprom_read_byte((uint8_t*)_masterCardSizeBaseAddress));
}

void EEPROMStorageHandler::setMasterCardSizeIndicator(bool is7Byte)
{
	eeprom_write_byte((uint8_t*)_masterCardSizeBaseAddress, is7Byte);
}

uint8_t* EEPROMStorageHandler::getMasterCardId() {
	return _masterCardId;
}

uint8_t* EEPROMStorageHandler::loadMasterCardId()
{
	for (int i = 0; i < 7; ++i)
	{
		//erase entire masterCardId in case that new 4B masterCardID will rewrite previous 7B card
		_masterCardId[i] = 0;
	}

	size_t uid_length = (getMasterCardSizeIndicator()) ? 7 : 4;

	eeprom_read_block(_masterCardId, (uint32_t*)_masterCardBaseAddress, uid_length);
	
	return _masterCardId;
}

bool EEPROMStorageHandler::isMasterCard(uint8_t* uid, uint8_t uid_length) const
{
	return areArraysEqual(uid, (uint8_t*)_masterCardId, uid_length);
}

void EEPROMStorageHandler::setMasterCard(uint8_t* uid, uint8_t uid_length)
{
	setMasterCardSizeIndicator(uid_length == 7);

	eeprom_write_block(uid, (uint32_t*)_masterCardBaseAddress, uid_length);

	loadMasterCardId();
}

#pragma endregion

#pragma region 4B/7B Card Pointers

void EEPROMStorageHandler::registerNew7BCard(uint8_t* cardId)
{
	uint32_t newCardAddress = getNew7BCardAddress();

#ifdef DEBUG
	Serial.println("registerNew7BCard");
	Serial.print("Card Addres to write:"); Serial.println(newCardAddress);
	printCard(cardId, 7);
#endif // DEBUG

	eeprom_write_block(cardId, (uint32_t*)newCardAddress, 7);

#ifdef DEBUG
	eeprom_read_block(cardId, (uint32_t*)newCardAddress, 7);
	printCard(cardId, 7);
#endif // DEBUG

	increaseNumberOfCards();
}

void EEPROMStorageHandler::registerNew4BCard(uint8_t* cardId)
{
	uint32_t newCardAddress = getNew4BCardAddress();

#ifdef DEBUG
	Serial.println("registerNew4BCard");
	Serial.print("Card Addres to write:"); Serial.println(newCardAddress);
	Serial.print("Card Value to write:"); Serial.println(convertToInt32(&cardId[0]));
#endif // DEBUG

	eeprom_write_dword((uint32_t*)newCardAddress, convertToInt32(cardId));

#ifdef DEBUG
	Serial.print("4B Card check that it was written: "); Serial.println(uint32_t(eeprom_read_dword(newCardAddress)));
#endif // DEBUG

	increaseNumberOfCards();
}

uint32_t EEPROMStorageHandler::getNew4BCardAddress()
{
	Serial.println("getNew4BCardAddress");
	
	if (_new4BCardAddress == 0 && getNumberOfCards() == 0)
	{
#ifdef DEBUG
		Serial.println("First condition");
		Serial.print("_new4BCardAddress value: "); Serial.println(_cardBlockBaseAddress);
#endif // DEBUG
		
		_new4BCardAddress = _cardBlockBaseAddress;
		eeprom_write_dword((uint32_t*)_4BpointerBaseAddress, _new4BCardAddress);
		setBlockOccupationType(0, true);
		setCardBlockType(0, false);
		return _new4BCardAddress;
	}
	else
	{
		Serial.println("Second condition");

		for (unsigned int block_index = 0; (block_index < 35); ++block_index)
		{
			if(isCardBlock7B(block_index))
			{
				continue;
			}

			for (int card_index = 0; card_index < 7; ++card_index)
			{
				uint32_t address = _cardBlockBaseAddress + block_index * 28 + card_index * 4;
				uint32_t card = eeprom_read_dword((uint32_t*)address);
#ifdef DEBUG
				Serial.print("Card value in memory: "); Serial.println(card);
				Serial.print("Checking card: block_index:"); Serial.print(block_index); Serial.print(" card_index: "); Serial.println(card_index);
#endif // DEBUG
				
				if (card == 0)
				{
					_new4BCardAddress = address;
					Serial.print("Next 4B card address: "); Serial.println(_new4BCardAddress);
					setBlockOccupationType(block_index, true);
					setCardBlockType(block_index, false);
					return _new4BCardAddress;
				}
			}
		}
	}
	return 5555555;
}

uint32_t EEPROMStorageHandler::getNew7BCardAddress()
{
	Serial.println("getNew7BCardAddress");

	if (_new7BCardAddress == 0 && getNumberOfCards() == 0)
	{
#ifdef DEBUG
		Serial.println("First condition");
		Serial.print("_new7BCardAddress value: "); Serial.println(_cardBlockBaseAddress);
#endif // DEBUG

		_new7BCardAddress = _cardBlockBaseAddress;
		eeprom_write_dword((uint32_t*)_7BpointerBaseAddress, _new7BCardAddress);
		setBlockOccupationType(0, true);
		setCardBlockType(0, true);
		return _new7BCardAddress;
	}
	else
	{
		Serial.println("Second condition");

		for (unsigned int block_index = 0; block_index < 35; ++block_index)
		{
			if(!canMarkAs7Bblock(block_index))
			{
				continue;
			}

			for (int card_index = 0; card_index < 4; ++card_index)
			{
				uint32_t address = _cardBlockBaseAddress + block_index * 28 + card_index * 7;
				uint8_t card[7], emptyCard[7] = { 0, 0, 0, 0, 0, 0, 0 };
				eeprom_read_block(card, (uint32_t*)address, 7);
#ifdef DEBUG
				Serial.print("Checking card: block_index:"); Serial.print(block_index); Serial.print(" card_index: "); Serial.println(card_index);
#endif // DEBUG			
				printCard(card, 7);

				if (areArraysEqual(card, emptyCard, 7))
				{
					Serial.print("Next 7B card address: "); Serial.println(address);
					_new7BCardAddress = address;
					setBlockOccupationType(block_index, true);
					setCardBlockType(block_index, true);
					return _new7BCardAddress;
				}
			}
		}
	}
	return 5555555;
}

bool EEPROMStorageHandler::canMarkAs7Bblock(unsigned char block_index)
{
	bool occupied = isBlockOccupied(block_index);
	bool is7BBlock = isCardBlock7B(block_index);

	Serial.print("canMarkAs7Bblock occupied: "); Serial.print(occupied); Serial.print(" is7BBlock :"); Serial.println(is7BBlock);

	if(occupied)
	{
		return is7BBlock;
	}
	return true;
}


#pragma endregion

#pragma region Card Registration / Comparing

void EEPROMStorageHandler::registerNewCard(uint8_t* cardId, const uint8_t uid_length) {
	bool isAlreadyRegistered = this->isCardRegistered(cardId, uid_length);
	bool isMasterCard = this->isMasterCard(cardId, uid_length);

	if (!isAlreadyRegistered && !isMasterCard) {
#ifdef DEBUG
		Serial.println("Card is not registered. Starting registration process");
#endif //DEBUG
		
		if(uid_length == 7)
		{
			registerNew7BCard(cardId);
		}
		else if (uid_length == 4)
		{
			registerNew4BCard(cardId);
		} 

#ifdef DEBUG
		Serial.print("_numberOfRecords IS "); Serial.println(getNumberOfCards());
		Serial.println("New card successfully registered!");
#endif // DEBUG
	}
	else if (isAlreadyRegistered) {
#ifdef DEBUG
		Serial.println("ERROR: Card is already registered.");
#endif // DEBUG
	}
	else if (isMasterCard) {
#ifdef DEBUG
		Serial.println("ERROR: Cannot save master card.");
#endif // DEBUG
	}
	else {
#ifdef DEBUG
		Serial.println("ERROR: Cannot save new card.");
#endif // DEBUG
	}
}

bool EEPROMStorageHandler::isCardRegistered(uint8_t* cardId, const uint8_t uid_length) {
	printMemory();
	
	const bool cardLength7B = uid_length == 7;
	//Serial.println("isCardRegistered");
	//Serial.print("Card length 7B:"); Serial.println(cardLength7B);
	
	for (unsigned short block_index = 0; block_index <= 31; block_index++) {
		//XNOR - it is 7B index and card length is 7 B
		// or is 4B block and car dlength is 4B
		Serial.print("Block Index: "); Serial.println(block_index);
		Serial.print("Block Occupied: "); Serial.println(isBlockOccupied(block_index));
		Serial.print("ZZZZZ: "); Serial.println((!(isCardBlock7B(block_index) ^ cardLength7B)));


		if((!(isCardBlock7B(block_index) ^ cardLength7B)) && isBlockOccupied(block_index))
		{
			Serial.print("Block Index: "); Serial.print(block_index); Serial.print("  present: ");

			if (isCardInBlock(block_index, cardId, uid_length))
			{
				Serial.println("YES  CARD IS REGISTERED!!!");
				return true;
			}

			Serial.println("NO.");
		}

	}
#ifdef DEBUG
	Serial.println("CARD IS NOT REGISTERED!!!");
#endif // DEBUG
	return false;
}

#pragma endregion

#pragma region Number of Cards


void EEPROMStorageHandler::setNumberOfCards(unsigned char newNumber) {

#ifdef DEBUG
	Serial.print("setNumberOfCards newNumber: ");  Serial.println(newNumber);
#endif // DEBUG

	eeprom_write_byte(0, newNumber);
	_numberOfRecords = newNumber;
}

unsigned char EEPROMStorageHandler::getNumberOfCards() {
#ifdef DEBUG
	Serial.print("getNumberOfCards _numberOfRecords: ");  Serial.println(_numberOfRecords);
#endif // DEBUG

	return _numberOfRecords;
}

void EEPROMStorageHandler::increaseNumberOfCards() {

#ifdef DEBUG
	Serial.println("increaseNumberOfCards");
#endif // DEBUG

	setNumberOfCards(getNumberOfCards() + 1);
}

void EEPROMStorageHandler::decreaseNumberOfCards() {

#ifdef DEBUG
	Serial.println("decreaseNumberOfCards");
#endif // DEBUG

	setNumberOfCards(getNumberOfCards() - 1);
}

#pragma endregion

#pragma region Pin


bool EEPROMStorageHandler::checkPinEquals(const uint32_t pin_entered) {
	const uint32_t pinInMemory = eeprom_read_dword((uint32_t*)_pinBaseAddress);

	return pinInMemory == pin_entered;
}

void EEPROMStorageHandler::setPin(const uint32_t pin_entered, uint32_t new_pin) {
	if (checkPinEquals(pin_entered)) {
		eeprom_write_dword((uint32_t*)_pinBaseAddress, new_pin);
	}
}

#pragma endregion

#pragma region Occupation Indicators
/*
Sets boolean value (isOccupied) to bit at block_index position (0-31), if block_index is out of range it terminates
*/
void EEPROMStorageHandler::setBlockOccupationType(unsigned char block_index, bool isOccupied) {

	if (block_index > 31) {
		return;
	}

	unsigned char block_position = block_index / 8;
	unsigned char bit_position = block_index % 8;
	unsigned char block = getBlockOccupationIndicator(block_index);

	unsigned char bit = (bit_position > 0) ? (1 << bit_position) : 1;

	block ^= (-isOccupied ^ block) & bit;

	eeprom_write_byte((uint8_t*)_blockOccupationBaseAddress + block_position, block);
}


/*
Checks whether block at index block_index has occupied indicator/flag set
*/
bool EEPROMStorageHandler::isBlockOccupied(unsigned char block_index)
{
	return isNthBitTrue(getBlockOccupationIndicator(block_index), block_index);
}


/*
Returns occupation block (1B) based on block_index, when block_index > 31 || block_index < 0 then 255 is returned
*/
unsigned char EEPROMStorageHandler::getBlockOccupationIndicator(const unsigned char block_index) {

	if (block_index > 31) {
		return 255; //ERROR
	}

	const unsigned char block_position = block_index / 8;
	const unsigned char block = eeprom_read_byte((uint8_t*)_blockOccupationBaseAddress + block_position);

	return block;
}

#pragma endregion 

#pragma region Block Indicators

/*
	Sets boolean value (is7ByteBlock) to bit at block_index position (0-31), if block_index is out of range it terminates
*/
void EEPROMStorageHandler::setCardBlockType(unsigned char block_index, bool is7ByteBlock) {

	if (block_index > 31) {
		return;
	}
	unsigned char block_position = block_index / 8;
	unsigned char bit_position = block_index % 8;
	unsigned char block = getCardBlockIndicator(block_index);

	unsigned char bit = (bit_position > 0) ? (1 << bit_position) : 1;

	block ^= (-is7ByteBlock ^ block) & bit;

	eeprom_write_byte((uint8_t*)_4B7BBlocksBaseAddress + block_position, block);
}

/*
	Returns card type block (1B) based on block_index, when block_index > 31 || block_index < 0 then 255 is returned
*/
unsigned char EEPROMStorageHandler::getCardBlockIndicator(const unsigned char block_index) {

	if (block_index > 31) {
		return 255;
	}

	const unsigned char block_position = block_index / 8;
	const uint8_t block = eeprom_read_byte((uint8_t*)_4B7BBlocksBaseAddress + block_position);

	return block;
}

/*
Checks whether card \a cardId with cardId length \a uid_length is in card block determined by \a block_index
*/
bool EEPROMStorageHandler::isCardInBlock(const unsigned char block_index, uint8_t* cardId, const uint8_t uid_length) const
{
	Serial.println("##################### isCardInBlock");
	if (uid_length == 4) {
		uint32_t cardToFind = convertToInt32(cardId);
		Serial.print("Card: "); Serial.println(cardToFind);

		for (unsigned short i = 0; i < 7; ++i)
		{
			uint32_t card = eeprom_read_dword((uint32_t*)(_cardBlockBaseAddress + (block_index * 28) + i * 4));
//#ifdef DEBUG
			Serial.print("i:"); Serial.print(i); Serial.print("  card : "); Serial.println(card);
//#endif //DEBUG
			if (card == cardToFind && card != 0)
			{
				return true;
			}
		}
	}
	else if (uid_length == 7) {
		for (unsigned short i = 0; i < 4; ++i)
		{
			uint8_t cardInMemory[7] = { 0, 0, 0, 0, 0, 0, 0 };
			eeprom_read_block(cardInMemory, (uint32_t*)(_cardBlockBaseAddress + (block_index * 28) + i * 7), 7 * sizeof(uint8_t));
			printCard(cardInMemory, 7);
			if (areArraysEqual(cardInMemory, cardId, uid_length)) {
				return true;
			}
		}
	}

	return false;
}

void EEPROMStorageHandler::printCard(uint8_t* card, uint8_t uid_length) const
{
	Serial.println("printCard:");
	for (int i = 0; i < uid_length; ++i)
	{
		Serial.print(card[i], DEC); Serial.print(" ");
	}
	Serial.println("");
}

/*
	Returns true, when block at index (0-31) has flag set to 1, false otherwise
*/
bool EEPROMStorageHandler::isCardBlock7B(unsigned char block_index) {

	Serial.println("isCardBlock7B");
	Serial.print("Card Block Indicator: "); Serial.println(getCardBlockIndicator(block_index));
	Serial.print("Block Index: "); Serial.println(block_index);
	Serial.print("Result: "); Serial.println(isNthBitTrue(getCardBlockIndicator(block_index), block_index));

	return isNthBitTrue(getCardBlockIndicator(block_index), block_index);
}

#pragma endregion

/*
Prints memory 
*/
void EEPROMStorageHandler::printMemory()
{
	Serial.println("Cards in MEM");
	Serial.println(eeprom_read_dword(_cardBlockBaseAddress));
	Serial.println(eeprom_read_dword(_cardBlockBaseAddress + 4));
	Serial.println(eeprom_read_dword(_cardBlockBaseAddress + 8));

#ifdef DEBUG

	Serial.println("============================MEMORY");

	Serial.print("Number of records: "); Serial.println(eeprom_read_byte(0));
	/*
	uint8_t master[7];
	Serial.print("Master Card:");
	eeprom_read_block(master, 1, 7);
	Serial.println(convertToInt32(master));
	Serial.print("Master Card size indicator: "); Serial.println(bool(eeprom_read_byte(_masterCardSizeBaseAddress)));
	*/
	uint8_t block_indicators[4];
	Serial.print("Block indicators:");
	eeprom_read_block(block_indicators, _4B7BBlocksBaseAddress, 4);
	Serial.println(convertToInt32(block_indicators));

	uint8_t occupancy_indicators[4];
	Serial.print("Occupancy indicators:");
	eeprom_read_block(occupancy_indicators, _blockOccupationBaseAddress, 4);
	Serial.println(convertToInt32(occupancy_indicators));

	Serial.print("4B pointer: ");
	uint32_t pointer4B = eeprom_read_dword(_4BpointerBaseAddress);
	Serial.println(pointer4B);

	Serial.print("7B pointer: ");
	uint32_t pointer7B = eeprom_read_dword(_7BpointerBaseAddress);
	Serial.println(pointer7B);
	/*
	uint32_t pin;
	Serial.print("Pin: ");
	pin = eeprom_read_dword(_pinBaseAddress);
	Serial.println(pin);
	Serial.print("PIN equals:"); Serial.println(checkPinEquals(123456789));*/


#endif // DEBUG

}

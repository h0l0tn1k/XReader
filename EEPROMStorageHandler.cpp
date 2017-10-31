#include "EEPROMStorageHandler.h"

//#define DEBUG

EEPROMStorageHandler::EEPROMStorageHandler()
	:_numberOfRecords(eeprom_read_byte(0))
{
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
	/*for (int i = 0; i < 1024; ++i)
	{
		eeprom_write_byte(i, 0);
	}*/

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
	//Serial.print("Card Value to write:"); Serial.println(convertToInt32(&cardId[0]));
#endif // DEBUG

	eeprom_write_block(cardId, (uint32_t*)newCardAddress, 7);
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
	uint16_t empty_block_index = 0;
	Serial.println("getNew4BCardAddress");
	
	if (_new4BCardAddress == 0 && getNumberOfCards() == 0)
	{
#ifdef DEBUG
		Serial.println("First condition");
		Serial.print("_new4BCardAddress value: "); Serial.println(_cardBlockBaseAddress);
#endif // DEBUG
		
		_new4BCardAddress = _cardBlockBaseAddress;
	}
	else
	{
		Serial.println("Second condition");

		for (unsigned int block_index = 0; (block_index < 35) && !isCardBlock7B(block_index); ++block_index)
		{
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
					Serial.print("Next 4B card address: "); Serial.println(address);
					_new4BCardAddress = address;
					empty_block_index = block_index;
				}
			}
		}
	}

	eeprom_write_dword((uint32_t*)_4BpointerBaseAddress, _new4BCardAddress);
	setBlockOccupationType(empty_block_index, true);
	return _new4BCardAddress;
}

uint32_t EEPROMStorageHandler::getNew7BCardAddress()
{
	return 0;
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
	const bool cardLength7B = uid_length == 7;
															//XNOR - it is 7B index and card length is 7 B
															// or is 4B block and car dlength is 4B
	for (unsigned short block_index = 0; block_index <= 31 && !(isCardBlock7B(block_index) ^ cardLength7B) && isBlockOccupied(block_index); block_index++) {

		Serial.print("Block Index: "); Serial.print(block_index); Serial.print("  present: ");

		if(isCardInBlock(block_index, cardId, uid_length))
		{
			Serial.print("YES  CARD IS REGISTERED!!!");
			return true;
		}

		Serial.println("NO.");
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
	const unsigned char block = eeprom_read_byte((uint8_t*)_4B7BBlocksBaseAddress + block_position);

	return block;
}

/*
Checks whether card \a cardId with cardId length \a uid_length is in card block determined by \a block_index
*/
bool EEPROMStorageHandler::isCardInBlock(const unsigned char block_index, uint8_t* cardId, const uint8_t uid_length) const
{
	if (uid_length == 4) {
		uint32_t cardToFind = convertToInt32(cardId);

		for (unsigned short i = 0; i < 7; ++i)
		{
			uint32_t card = eeprom_read_dword((uint32_t*)_cardBlockBaseAddress + (block_index * 28) + i * 4);

#ifdef DEBUG
			Serial.print("i:"); Serial.print(i); Serial.print("  card : "); Serial.println(card);
#endif //DEBUG
			if (card == cardToFind && card != 0)
			{
				return true;
			}
		}
	}
	else if (uid_length == 7) {
		for (unsigned short i = 0; i < 4; ++i)
		{
			uint8_t cardInMemory[7];
			eeprom_read_block(cardInMemory, (uint32_t*)_cardBlockBaseAddress + (block_index * 28) + i * 7, 7 * sizeof(uint8_t));
			if (areArraysEqual(cardInMemory, cardId, uid_length)) {
				return true;
			}
		}
	}

	return false;
}

/*
	Returns true, when block at index (0-31) has flag set to 1, false otherwise
*/
bool EEPROMStorageHandler::isCardBlock7B(unsigned char block_index) {

	return isNthBitTrue(getCardBlockIndicator(block_index), block_index);
}

#pragma endregion

/*
Prints memory 
*/
void EEPROMStorageHandler::printMemory()
{

#ifdef DEBUG

	Serial.println("============================MEMORY");

	Serial.print("Number of records: "); Serial.println(eeprom_read_byte(0));

	uint8_t master[7];
	Serial.print("Master Card:");
	eeprom_read_block(master, 1, 7);
	Serial.println(convertToInt32(master));
	Serial.print("Master Card size indicator: "); Serial.println(bool(eeprom_read_byte(_masterCardSizeBaseAddress)));

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

	uint32_t pin;
	Serial.print("Pin: ");
	pin = eeprom_read_dword(_pinBaseAddress);
	Serial.println(pin);
	Serial.print("PIN equals:"); Serial.println(checkPinEquals(123456789));


#endif // DEBUG

}

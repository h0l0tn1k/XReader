#include "EEPROMStorageHandler.h"


#define DEBUG

EEPROMStorageHandler::EEPROMStorageHandler(HardwareSerial* serial)
	:_numberOfRecords(getNumberOfCards()), 
	 _masterCardId(getMasterCardId()),
	 _serial(serial)
{


	_new4BCardAddress = eeprom_read_dword(_4BpointerBaseAddress);
	_new7BCardAddress = eeprom_read_dword(_7BpointerBaseAddress);

	Serial.print("4B Card Address: "); Serial.println(_new4BCardAddress);
	Serial.print("7B Card Address: "); Serial.println(_new7BCardAddress);
}


uint32_m EEPROMStorageHandler::convertToInt32(uint8_t* uid) {
	return (uint32_m(uid[0]) << 24) | (uint32_m(uid[1]) << 16) | (uint32_m(uid[2]) << 8) | uint32_m(uid[3]);
}

#pragma region Master Card

bool EEPROMStorageHandler::getMasterCardSizeIndicator() 
{
	//true -> 7B, false -> 4B
	return bool(eeprom_read_byte(8));
}

void EEPROMStorageHandler::setMasterCardSizeIndicator(bool is7Byte)
{
	eeprom_write_byte(8, is7Byte);
}

uint32_m EEPROMStorageHandler::getMasterCardId()
{
	uint8_t uid[] = { 0, 0, 0, 0, 0, 0, 0 };
	size_t uid_length = (getMasterCardSizeIndicator()) ? 7 : 4;

	eeprom_read_block(&uid[0], _masterCardBaseAddress, 7);

	// TODO: CHANGE TO 4B/7B version
	uint32_m i = convertToInt32(&uid[0]);

	Serial.print("MASTER CARD VALUE: "); Serial.println(uint32_m(i));

	return i;
}

bool EEPROMStorageHandler::isMasterCard(uint8_t* uid, uint8_t uid_length) const
{
	return _masterCardId == convertToInt32(uid);
}

void EEPROMStorageHandler::setMasterCard(uint8_t* uid, uint8_t uid_length)
{
	setMasterCardSizeIndicator(uid_length == 7);

	eeprom_write_block(&uid[0], _masterCardBaseAddress, uid_length);

	/*for (size_t i = 0; i < uid_length; i++)
	{
		eeprom_write_byte(i + 1, uid[i]);
	}*/
	_masterCardId = convertToInt32(&uid[0]);
}

#pragma endregion

void EEPROMStorageHandler::registerNew7BCard(uint8_t* cardId)
{
}

void EEPROMStorageHandler::registerNew4BCard(uint8_t* cardId)
{
	uint32_t newCardAddress = getNew4BCardAddress();

	Serial.println("registerNew4BCard");
	Serial.print("Card Addres to write:"); Serial.println(newCardAddress);
	Serial.print("Card Value to write:"); Serial.println(convertToInt32(&cardId[0]));
	eeprom_write_dword(newCardAddress, convertToInt32(&cardId[0]));

	Serial.print("4B Card check that it was written: "); Serial.println(uint32_t(eeprom_read_dword(newCardAddress)));

	increaseNumberOfCards();
}

uint32_t EEPROMStorageHandler::getNew4BCardAddress()
{
	Serial.println("getNew4BCardAddress");
	
	if (_new4BCardAddress == 0 && getNumberOfCards() == 0)
	{
		Serial.println("First condition");
		_new4BCardAddress = _cardBlockBaseAddress;

		Serial.print("_new4BCardAddress value: "); Serial.println(_new4BCardAddress);
		eeprom_write_dword(_4BpointerBaseAddress, _new4BCardAddress);
		setBlockOccupationType(0, true);
		return _new4BCardAddress;
	} 
	else
	{
		Serial.println("Second condition");
		//not null find next empty spot
		//&& isBlockOccupied(block_index) 
		for (int block_index = 0; (block_index < 35) && !isCardBlock7B(block_index); ++block_index)
		{
			for (int card_index = 0; card_index < 7; ++card_index)
			{
				uint32_t address = _cardBlockBaseAddress + block_index * 28 + card_index*4;
				uint32_t card = eeprom_read_dword(address);
				Serial.print("Card value in memory: "); Serial.println(card);
				Serial.print("Checking card: block_index:"); Serial.print(block_index); Serial.print(" card_index: "); Serial.println(card_index);
				
				if (card == 0)
				{
					Serial.print("Next 4B card address: "); Serial.println(address);
					_new4BCardAddress = address;
					eeprom_write_dword(_4BpointerBaseAddress, _new4BCardAddress);
					setBlockOccupationType(block_index, true);

					return _new4BCardAddress;
				}
			}
		}
	}

	//ERROR

}

uint32_t EEPROMStorageHandler::getNew7BCardAddress()
{
}

void EEPROMStorageHandler::registerNewCard(uint8_t* cardId, const uint8_t uid_length) {
	bool isAlreadyRegistered = this->isCardRegistered(cardId, uid_length);
	bool isMasterCard = this->isMasterCard(cardId, uid_length);

	if (!isAlreadyRegistered && !isMasterCard) {
#ifdef DEBUG
		Serial.println("Card is not registered. Starting registration process");

#endif //DEBUG


		if(uid_length == 7)
		{
			//registerNew7BCard(cardId);
		}
		else if (uid_length == 4)
		{
			registerNew4BCard(cardId);
		} 

		/*
		unsigned short int offset = _baseAddress + (_numberOfRecords * 4);
#ifdef DEBUG
		Serial.print("OFFSET IS "); Serial.println((unsigned short int)offset);
#endif // DEBUG
		for (size_t i = 0; i < 4; i++)
		{
			eeprom_write_byte(i + offset, cardId[i]);
		}
		*/

		this->setNumberOfCards(++_numberOfRecords);
#ifdef DEBUG
		Serial.print("_numberOfRecords IS "); Serial.println(uint8_t(_numberOfRecords), DEC);
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
	const uint32_m cardIdInt32 = convertToInt32(&cardId[0]);
	const bool cardLength7B = uid_length == 7;

#ifdef DEBUG
	Serial.print("CARD TO CHECK : "); Serial.println(uint32_m(cardIdInt32), DEC);
#endif // DEBUG

															//XNOR - it is 7B index and card length is 7 B
															// or is 4B block and car dlength is 4B
	for (size_t block_index = 0; block_index <= 31 && !(isCardBlock7B(block_index) ^ cardLength7B) && isBlockOccupied(block_index); block_index++) {

		Serial.print("Block Index: "); Serial.println(block_index);

		const bool cardIsInCurrentBlock = isCardInBlock(block_index, cardId, uid_length);
		
		if(cardIsInCurrentBlock)
		{
			Serial.println("CARD IS REGISTERED!!!");

			return true;
		}
	}
#ifdef DEBUG
	Serial.println("CARD IS NOT REGISTERED!!!");
#endif // DEBUG
	return false;
}

#pragma region Number of Cards


void EEPROMStorageHandler::setNumberOfCards(unsigned char newNumber) {

#ifdef DEBUG
	Serial.print("Setting number of stored cards to : ");  Serial.println(uint8_t(newNumber));
#endif // DEBUG

	eeprom_write_byte(0, newNumber);
	_numberOfRecords = newNumber;
}

unsigned char EEPROMStorageHandler::getNumberOfCards() {
	_numberOfRecords = eeprom_read_byte(0);

#ifdef DEBUG
	Serial.print("Number of stored cards: ");  Serial.println(_numberOfRecords);
#endif // DEBUG

	return _numberOfRecords;
}

void EEPROMStorageHandler::increaseNumberOfCards() {

#ifdef DEBUG
	Serial.print("Increasing number of stored cards.");
#endif // DEBUG

	setNumberOfCards(getNumberOfCards() + 1);
}

void EEPROMStorageHandler::decreaseNumberOfCards() {

#ifdef DEBUG
	Serial.print("Decreasing number of stored cards.");
#endif // DEBUG

	setNumberOfCards(getNumberOfCards() - 1);
}

#pragma endregion


#pragma region Pin


bool EEPROMStorageHandler::checkPinEquals(const uint32_t pin_entered) {
	const uint32_t pinInMemory = eeprom_read_dword(_pinBaseAddress);

	return pinInMemory == pin_entered;
}

void EEPROMStorageHandler::setPin(uint32_t new_pin) {
	eeprom_write_dword(_pinBaseAddress, new_pin);
}

#pragma endregion

#pragma region OccupationIndicators
/*
Sets boolean value (isOccupied) to bit at block_index position (0-31), if block_index is out of range it terminates
*/
void EEPROMStorageHandler::setBlockOccupationType(unsigned char block_index, bool isOccupied) {

	unsigned char block_position = block_index / 8;
	unsigned char bit_position = block_index % 8;
	unsigned char block = getBlockOccupationIndicator(block_index);

	if (block == 255) {
		return;
	}

	unsigned char bit = (bit_position > 0) ? (1 << bit_position) : 1;

	block ^= (-isOccupied ^ block) & bit;

	//Serial.print("Write occ block:"); Serial.println(block);
	eeprom_write_byte(_blockOccupationBaseAddress + block_position, block);
}


bool EEPROMStorageHandler::isBlockOccupied(unsigned char block_index)
{
	return isNthBitTrue(getBlockOccupationIndicator(block_index), block_index);
}


/*
Returns occupation block (1B) based on block_index, when block_index > 31 || block_index < 0 then 255 is returned
*/
unsigned char EEPROMStorageHandler::getBlockOccupationIndicator(const unsigned char block_index) {

	if (block_index > 31 || block_index < 0) {
		return 255; //ERROR
	}

	const unsigned char block_position = block_index / 8;
	const unsigned char block = eeprom_read_byte(_blockOccupationBaseAddress + block_position);

	return block;
}

#pragma endregion 

#pragma region BlockIndicators


/*
	Sets boolean value (is7ByteBlock) to bit at block_index position (0-31), if block_index is out of range it terminates
*/
void EEPROMStorageHandler::setCardBlockType(unsigned char block_index, bool is7ByteBlock) {

	unsigned char block_position = block_index / 8;
	unsigned char bit_position = block_index % 8;
	unsigned char block = getCardBlockIndicator(block_index);

	if (block == 255) {
		return;
	}

	unsigned char bit = (bit_position > 0) ? (1 << bit_position) : 1;

	block ^= (-is7ByteBlock ^ block) & bit;

	eeprom_write_byte(_4B7BBlocksBaseAddress + block_position, block);	
}

/*
	Returns card type block (1B) based on block_index, when block_index > 31 || block_index < 0 then 255 is returned
*/
unsigned char EEPROMStorageHandler::getCardBlockIndicator(const unsigned char block_index) {

	if (block_index > 31 || block_index < 0) {
		return 255; //ERROR
	}

	const unsigned char block_position = block_index / 8;
	const unsigned char block = eeprom_read_byte(_4B7BBlocksBaseAddress + block_position);

	return block;
}

bool EEPROMStorageHandler::isCardInBlock(const unsigned char block_index, const uint8_t* cardId, const uint8_t uid_length) const
{
	int32_t cardToFind = convertToInt32(cardId);

#ifdef DEBUGn
	Serial.print("IsCardInBlock: "); Serial.println(cardToFind);
#endif //DEBUG

	for (int i = 0; i < 7; ++i)
	{
		uint32_t card = eeprom_read_dword(_cardBlockBaseAddress + (block_index * 28) + i * 4);

#ifdef DEBUG
		Serial.print("i:"); Serial.print(i); Serial.print("  card : "); Serial.println(card);
#endif //DEBUG
		if (card == cardToFind && card != 0)
		{
			return true;
		}
	}

	return false;
}

/*
	Returns true, when block at index (0-31) has flag set to 1, false otherwise
*/
bool EEPROMStorageHandler::isCardBlock7B(unsigned char block_index) {

	return isNthBitTrue(getCardBlockIndicator(block_index), block_index);
	/*unsigned char block = getCardBlockIndicator(block_index);
	
	if (block == 255) {
		return false;
	}

	unsigned char bit_position = block_index % 8;
	unsigned char bit = (bit_position > 0) ? (1 << bit_position) : 1;

	return (block & bit) > 0;*/
}

bool EEPROMStorageHandler::isNthBitTrue(unsigned char byte, unsigned char bit_index) const
{
	if (byte == 255) {
		return false;
	}
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



void EEPROMStorageHandler::printMemory()
{
	Serial.println("============================MEMORY");

	Serial.print("Number of records: "); Serial.println(eeprom_read_byte(0));

	uint8_t master[8];
	Serial.print("Master Card:");
	eeprom_read_block(&master[0], 1, 7);
	Serial.println(convertToInt32(&master[0]));
	Serial.print("Master Card size indicator: "); Serial.println(bool(eeprom_read_byte(_masterCardSizeBaseAddress)));

	uint8_t block_indicators[4];
	Serial.print("Block indicators:");
	eeprom_read_block(&block_indicators[0], _4B7BBlocksBaseAddress, 4);
	Serial.println(convertToInt32(&block_indicators[0]));

	uint8_t occupancy_indicators[4];
	Serial.print("Occupancy indicators:");
	eeprom_read_block(&occupancy_indicators[0], _blockOccupationBaseAddress, 4);
	Serial.println(convertToInt32(&occupancy_indicators[0]));

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


}

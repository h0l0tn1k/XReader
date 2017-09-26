#include "EEPROMStorageHandler.h"



EEPROMStorageHandler::EEPROMStorageHandler(HardwareSerial* serial)
	:_serial(serial), 
	 _numberOfRecords(getNumberOfCards()),
	 _masterCardId(getMasterCardId())
{

}


uint32_m EEPROMStorageHandler::convertToInt32(uint8_t* uid) {
	return ((uint32_m)uid[0] << 24) | ((uint32_m)uid[1] << 16) | ((uint32_m)uid[2] << 8) | ((uint32_m)uid[3]);
}

#pragma region Master Card

bool EEPROMStorageHandler::getMasterCardSizeIndicator() 
{
	//true -> 7B, false -> 4B
	return ((bool) eeprom_read_byte(8));
}

void EEPROMStorageHandler::setMasterCardSizeIndicator(bool is7Byte)
{
	eeprom_write_byte(8, is7Byte);
}

uint32_m EEPROMStorageHandler::getMasterCardId()
{
	uint8_t uid[] = { 0, 0, 0, 0, 0, 0, 0 };
	size_t uid_length = (getMasterCardSizeIndicator()) ? 7 : 4;

	for (uint8_t i = 0; i < uid_length; i++)
	{
		uid[i] = eeprom_read_byte(i + 1);
	}

	// TODO: CHANGE TO 4B/7B version
	uint32_m i = convertToInt32(&uid[0]);

	Serial.print("MASTER CARD VALUE: "); Serial.println((uint32_m)i);

	return i;
}

bool EEPROMStorageHandler::isMasterCard(uint8_t* uid, uint8_t uid_length)
{
	return _masterCardId == convertToInt32(uid);
}

void EEPROMStorageHandler::setMasterCard(uint8_t* uid, uint8_t uid_length)
{
	setMasterCardSizeIndicator(uid_length == ((size_t) uid_length));

	for (size_t i = 0; i < uid_length; i++)
	{
		eeprom_write_byte(i + 1, uid[i]);
	}
}

#pragma endregion


void EEPROMStorageHandler::registerNewCard(uint8_t* cardId, uint8_t uid_length) {
	bool isAlreadyRegistered = this->isCardRegistered(cardId, uid_length);
	bool isMasterCard = this->isMasterCard(cardId, uid_length);

	if (!isAlreadyRegistered && !isMasterCard) {
		/*
		unsigned short int offset = _baseAddress + (_numberOfRecords * 4);
#ifdef DEBUG
		_serial->print("OFFSET IS "); _serial->println((unsigned short int)offset);
#endif // DEBUG
		for (size_t i = 0; i < 4; i++)
		{
			eeprom_write_byte(i + offset, cardId[i]);
		}
		*/

		this->setNumberOfCards(++_numberOfRecords);
#ifdef DEBUG
		_serial->print("_numberOfRecords IS "); _serial->println((unsigned char)_numberOfRecords, DEC);
#endif // DEBUG

		_serial->println("New card successfully registered!");
	}
	else if (isAlreadyRegistered) {
		_serial->println("ERROR: Card is already registered.");
	}
	else if (isMasterCard) {
		_serial->println("ERROR: Cannot save master card.");
	}
	else {
		_serial->println("ERROR: Cannot save new card.");
	}
}

bool EEPROMStorageHandler::isCardRegistered(uint8_t* cardId, uint8_t uid_length) {

	uint32_m cardIdInt32 = convertToInt32(&cardId[0]);

#ifdef DEBUG
	_serial->print("CARD TO CHECK : "); _serial->println((uint32_m)cardIdInt32, DEC);
#endif // DEBUG

	for (size_t indexOfRecords = 0; indexOfRecords < _numberOfRecords; indexOfRecords++) {
			
		uint32_m fromMemory = getCardAtIndex(indexOfRecords);

#ifdef DEBUG
		_serial->print("	CARD IN MEMORY: "); _serial->println((uint32_m)fromMemory, DEC);
#endif // DEBUG

		if (cardIdInt32 == fromMemory) {

#ifdef DEBUG
			_serial->println("CARD IS REGISTERED!!!");
#endif // DEBUG

			return true;
		}
	}
#ifdef DEBUG
	_serial->println("CARD IS NOT REGISTERED!!!");
#endif // DEBUG
	return false;
}

uint32_m EEPROMStorageHandler::getCardAtIndex(unsigned char index) {
	uint8_t uid[] = { 0, 0, 0, 0 };

	for (size_t i = 0; i < 4; i++)
	{
		//uid[i] = eeprom_read_byte(_baseAddress + (i * 4) + i);
	}

	return convertToInt32(&uid[0]);
}


#pragma region Number of Cards


void EEPROMStorageHandler::setNumberOfCards(unsigned char newNumber) {

#ifdef DEBUG
	_serial->print("Setting number of stored cards to : ");  _serial->println((unsigned int)newNumber);
#endif // DEBUG

	eeprom_write_byte(0, newNumber);
	_numberOfRecords = newNumber;
}

unsigned char EEPROMStorageHandler::getNumberOfCards() {
	_numberOfRecords = eeprom_read_byte(0);

#ifdef DEBUG
	_serial->print("Number of stored cards: ");  _serial->println((unsigned int)_numberOfRecords);
#endif // DEBUG

	return _numberOfRecords;
}

void EEPROMStorageHandler::increaseNumberOfCards() {

#ifdef DEBUG
	_serial->print("Increasing number of stored cards.");
#endif // DEBUG

	setNumberOfCards(getNumberOfCards() + 1);
}

void EEPROMStorageHandler::decreaseNumberOfCards() {

#ifdef DEBUG
	_serial->print("Decreasing number of stored cards.");
#endif // DEBUG

	setNumberOfCards(getNumberOfCards() - 1);
}

#pragma endregion


#pragma region Pin


bool EEPROMStorageHandler::checkPinEquals(uint32_t pin_entered) {
	uint8_t pin[] = { 0, 0, 0, 0 };

	for (size_t i = 0; i < 4; i++)
	{
		pin[i] = eeprom_read_byte(_pinBaseAddress + (i * 4) + i);
	}

	return convertToInt32(&pin[0]) == pin_entered;
}

void EEPROMStorageHandler::setPin(uint32_t new_pin) {
	//throw "Not implemented method";
}

#pragma endregion


#pragma region BlockIndicators


/*
	Sets boolean value (is7ByteBlock) to bit at block_index position (0-31), if block_index is out of range it terminates
*/
void EEPROMStorageHandler::setCardBlockType(unsigned char block_index, bool is7ByteBlock) {

	unsigned char block_position = block_index / 8;
	unsigned char bit_position = block_index % 8;
	unsigned char block = getCardBlock(block_index);

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
unsigned char EEPROMStorageHandler::getCardBlock(unsigned char block_index) {

	if (block_index > 31 || block_index < 0) {
		return 255; //ERROR
	}

	unsigned char block_position = block_index / 8;
	unsigned char block = eeprom_read_byte(_4B7BBlocksBaseAddress + block_position);

	return block;
}

/*
	Returns true, when block at index (0-31) has flag set to 1, false otherwise
*/
bool EEPROMStorageHandler::isCardBlock7B(unsigned char block_index) {

	unsigned char block = getCardBlock(block_index);

	if (block == 255) {
		return false;
	}

	unsigned char bit_position = block_index % 8;
	unsigned char bit = (bit_position > 0) ? (1 << bit_position) : 1;

	return (block & bit) > 0;
}

#pragma endregion

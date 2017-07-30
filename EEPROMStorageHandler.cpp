#include "EEPROMStorageHandler.h"



EEPROMStorageHandler::EEPROMStorageHandler(HardwareSerial* serial)
	:_serial(serial)
{
	_numberOfRecords = getNumberOfStoredCards();
}

void EEPROMStorageHandler::writeMasterCard(uint8_t * cardId)
{
	for (size_t i = 0; i < 4; i++)
	{
		eeprom_write_byte(i + _masterCardAddress, cardId[i]);
	}
}

unsigned long int EEPROMStorageHandler::convertToInt32(uint8_t* uid) {
	unsigned long int i = ((unsigned long int)uid[0] << 24) | ((unsigned long int)uid[1] << 16) | ((unsigned long int)uid[2] << 8) | ((unsigned long int)uid[3]);
	return i;
}

unsigned long int EEPROMStorageHandler::getMasterCardId()
{
	uint8_t uid[] = { 0, 0, 0, 0};

	for (uint8_t i = 0; i < 4; i++)
	{
		uid[i] = eeprom_read_byte(i + 1);
	}
	
	unsigned long int i = convertToInt32(&uid[0]);

#ifdef DEBUG
	_serial->print("MASTER CARD VALUE: "); _serial->println((unsigned long int)i);
#endif // DEBUG

	return i;
}

bool EEPROMStorageHandler::isMasterCard(uint8_t * cardId)
{
	return getMasterCardId() == convertToInt32(cardId);
}

void EEPROMStorageHandler::registerNewCard(uint8_t* cardId) {
	bool isAlreadyRegistered = this->isCardRegistered(cardId);
	bool isMasterCard = this->isMasterCard(cardId);

	if (!isAlreadyRegistered && !isMasterCard) {
		unsigned short int offset = _baseAddress + (_numberOfRecords * 4);
#ifdef DEBUG
		_serial->print("OFFSET IS "); _serial->println((unsigned short int)offset);
#endif // DEBUG
		for (size_t i = 0; i < 4; i++)
		{
			eeprom_write_byte(i + offset, cardId[i]);
		}

		setNumberOfStoredCards(++_numberOfRecords);
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

bool EEPROMStorageHandler::isCardRegistered(uint8_t* cardId) {

	unsigned long int cardIdInt32 = convertToInt32(&cardId[0]);

#ifdef DEBUG
	_serial->print("CARD TO CHECK : "); _serial->println((unsigned long int)cardIdInt32, DEC);
#endif // DEBUG

	for (size_t indexOfRecords = 0; indexOfRecords < _numberOfRecords; indexOfRecords++) {
			
		unsigned long int fromMemory = getCardAtIndex(indexOfRecords);

#ifdef DEBUG
		_serial->print("	CARD IN MEMORY: "); _serial->println((unsigned long int)fromMemory, DEC);
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

unsigned long int EEPROMStorageHandler::getCardAtIndex(unsigned char index) {
	uint8_t uid[] = { 0, 0, 0, 0 };

	for (size_t i = 0; i < 4; i++)
	{
		uid[i] = eeprom_read_byte(_baseAddress + (index * 4) + i);
	}

	return convertToInt32(&uid[0]);
}

void EEPROMStorageHandler::setNumberOfStoredCards(unsigned char newNumber) {
	eeprom_write_byte(0, newNumber);
}

unsigned char EEPROMStorageHandler::getNumberOfStoredCards() {
	_numberOfRecords = eeprom_read_byte(0);

#ifdef DEBUG
	_serial->print("Number of stored cards: ");  _serial->println((int)_numberOfRecords);
#endif // DEBUG

	return _numberOfRecords;
}

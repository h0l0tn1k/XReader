#include "XReader.h"

XReader::XReader() {

	_board = new Adafruit_PN532(PN532_SS);
}

void XReader::begin()
{
#ifndef ESP8266
	while (!Serial); // for Leonardo/Micro/Zero
#endif
	Serial.begin(115200);
	
  _eepromStorage = new EEPROMStorageHandler(&Serial);
  pinMode(_blueLed, OUTPUT);
  pinMode(_greenLed, OUTPUT);
  digitalWrite(_greenLed, HIGH); 

  _board->begin();
}

void XReader::checkConnectionToPn532()
{
	uint32_t versiondata = _board->getFirmwareVersion();
	if (!versiondata) {
		Serial.print("Didn't find PN53x board");
		while (1); // halt
	}
	// Got ok data, print it out!
	Serial.print("Found chip PN5"); Serial.println((versiondata >> 24) & 0xFF, HEX);
	Serial.print("Firmware ver. "); Serial.print((versiondata >> 16) & 0xFF, DEC);
	Serial.print('.'); Serial.println((versiondata >> 8) & 0xFF, DEC);

	initBoard();
}

void XReader::loopProcedure()
{
	boolean success;
	uint8_t uid[] = { 0, 0, 0, 0, 0, 0, 0 };	// Buffer to store the returned UID
	uint8_t uidLength;				// Length of the UID (4 or 7 bytes depending on ISO14443A card type)

									// Wait for an ISO14443A type cards (Mifare, etc.).  When one is found
									// 'uid' will be populated with the UID, and uidLength will indicate
									// if the uid is 4 bytes (Mifare Classic) or 7 bytes (Mifare Ultralight)
	success = _board->readPassiveTargetID(PN532_MIFARE_ISO14443A, &uid[0], &uidLength);

	//_eepromStorage->writeMasterCard(&uid[0]);
	//return 0;
	if (success) {
		if (_eepromStorage->isMasterCard(&uid[0])) {
			Serial.println("=======THIS IS MASTERCARD======");

			Serial.println("Waiting for new card to register...");
			delay(2000);
			_board->readPassiveTargetID(PN532_MIFARE_ISO14443A, &uid[0], &uidLength);
			_eepromStorage->registerNewCard(&uid[0]);

		}else if (_eepromStorage->isCardRegistered(&uid[0])) {
			Serial.println("=======THIS IS REGISTERED CARD======");

		}
		else {
			Serial.println("#######THIS IS NOT REGISTERED CARD ######");
		}
    
		//Serial.println("Found a card!");
		//Serial.print("UID Length: "); Serial.print(uidLength, DEC); Serial.println(" bytes");
		//Serial.print("UID Value: ");
		for (uint8_t i = 0; i < uidLength; i++)
		{
			//Serial.print(" 0x"); Serial.print(uid[i], HEX);
		}
		//Serial.println("");
		// Wait 1 second before continuing
    digitalWrite(_blueLed, HIGH); 
		delay(1000);
    digitalWrite(_blueLed, LOW); 
	}
	else
	{
		// PN532 probably timed out waiting for a card
		Serial.println("Timed out waiting for a card");
	}
}

void XReader::initBoard()
{
	// Set the max number of retry attempts to read from a card
	// This prevents us from waiting forever for a card, which is
	// the default behaviour of the PN532.
	_board->setPassiveActivationRetries(0xFF);

	// configure board to read RFID tags
	_board->SAMConfig();

	Serial.println("Waiting for an ISO14443A card");
}

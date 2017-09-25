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
  switchOnLed(_greenLed);

  _board->begin();
}

void XReader::checkConnectionToPn532()
{
	uint32_t versiondata = _board->getFirmwareVersion();
	if (!versiondata) {
		Serial.print("Didn't find PN53x board");
		while (1); // halt
	}

#ifdef DEBUG
	Serial.print("Found chip PN5"); Serial.println((versiondata >> 24) & 0xFF, HEX);
	Serial.print("Firmware ver. "); Serial.print((versiondata >> 16) & 0xFF, DEC);
	Serial.print('.'); Serial.println((versiondata >> 8) & 0xFF, DEC);
#endif // DEBUG

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

	if (success) {
		if (_eepromStorage->isMasterCard(&uid[0])) {
			Serial.println("=======THIS IS MASTERCARD======"); 
			Serial.println("Waiting for new card to register...");

			delay(1000); // delay otherwise it'd register master card
			switchOnLed(_blueLed);

			_board->readPassiveTargetID(PN532_MIFARE_ISO14443A, &uid[0], &uidLength);
			_eepromStorage->registerNewCard(&uid[0]);

			switchOffLed(_blueLed);

		}else if (_eepromStorage->isCardRegistered(&uid[0])) {
			Serial.println("=======THIS IS REGISTERED CARD======");

			switchOnLed(_blueLed);
			switchOnLed(_buzzer);
			//TODO: open door for 3s
			//TODO: sound buzzer
			delay(2000);

			//TODO: close door
			//TODO: mute buzzer
			switchOffLed(_blueLed);
			switchOffLed(_buzzer);
		}
		else {
			Serial.println("#######THIS IS NOT REGISTERED CARD ######");
			
			//TODO: 
			switchOnLed(_redLed); 
			switchOnLed(_buzzer);
			delay(2000);
			//TODO: 
			switchOffLed(_redLed);
			switchOffLed(_buzzer);
		}
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

void XReader::switchOnLed(unsigned char ledPin) {
	digitalWrite(ledPin, HIGH);
}

void XReader::switchOffLed(unsigned char ledPin) {
	digitalWrite(ledPin, LOW);
}
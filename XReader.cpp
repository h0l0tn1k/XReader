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
  pinMode(_redLed, OUTPUT);
  pinMode(_buzzer, OUTPUT);
  switchOnLed(_blueLed);

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
	Serial.println("LOOP");
	uint8_t uid[] = { 0, 0, 0, 0, 0, 0, 0 };	// Buffer to store the returned UID
	uint8_t uidLength;				// Length of the UID (4 or 7 bytes depending on ISO14443A card type)

									// Wait for an ISO14443A type cards (Mifare, etc.).  When one is found
									// 'uid' will be populated with the UID, and uidLength will indicate
									// if the uid is 4 bytes (Mifare Classic) or 7 bytes (Mifare Ultralight)

	const boolean success = _board->readPassiveTargetID(PN532_MIFARE_ISO14443A, &uid[0], &uidLength);

	if (success) {

		if (_eepromStorage->isMasterCard(&uid[0], uidLength)) {
			Serial.println("=======THIS IS MASTERCARD======"); 
			Serial.println("Waiting for new card to register...");
			_consecutiveFails = 0;

			delay(1000); // delay otherwise it'd register master card
			switchOnLed(_greenLed);
			switchOffLed(_blueLed);

			_board->readPassiveTargetID(PN532_MIFARE_ISO14443A, &uid[0], &uidLength);
			_eepromStorage->registerNewCard(&uid[0], uidLength);

			switchOffLed(_greenLed);
			switchOnLed(_blueLed);

		}else if (_eepromStorage->isCardRegistered(&uid[0], uidLength)) {
			Serial.println("=======THIS IS REGISTERED CARD======");
			_consecutiveFails = 0;

			switchOffLed(_blueLed);
			switchOnLed(_greenLed);
			switchSuccessAuthBuzzerOn();

			delay(DOOR_OPENED_INTERVAL);

			switchOffLed(_greenLed);
			switchOnLed(_blueLed);
		}
		else {
			Serial.println("#######THIS IS NOT REGISTERED CARD ######");
			_consecutiveFails++;

			//TODO: 
			switchOffLed(_blueLed);
			switchOnLed(_redLed); 
			soundUnsuccessAuthBuzzer();

			const unsigned int logDelay = log(_consecutiveFails) * 1000;

			//incremental delay
			delay(logDelay);
			
			//TODO: 
			switchOffLed(_redLed);
			switchOnLed(_blueLed);
		}
	}
	else
	{
		// PN532 probably timed out waiting for a card
		Serial.println("Timed out waiting for a card");


		Serial.print("Time: "); Serial.println(millis());
	}
}

void XReader::initBoard() const
{
	// Set the max number of retry attempts to read from a card
	// This prevents us from waiting forever for a card, which is
	// the default behaviour of the PN532.
	//0xFF
	_board->setPassiveActivationRetries(0x01);

	// configure board to read RFID tags
	_board->SAMConfig();

	Serial.println("Waiting for an ISO14443A card");

}

void XReader::switchOnLed(const unsigned char ledPin) {
	digitalWrite(ledPin, HIGH);
}

void XReader::switchOffLed(const unsigned char ledPin) {
	digitalWrite(ledPin, LOW);
}

void XReader::soundUnsuccessAuthBuzzer() const
{
	for (int i = 0; i < 3; ++i)
	{
		tone(_buzzer, INVALID_SOUND, 200);
		delay(300);
	}
}

void XReader::switchSuccessAuthBuzzerOn() const
{
	tone(_buzzer, VALID_SOUND, 1000);
}
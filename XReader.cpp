#include "XReader.h"

XReader::XReader() 
{
	_board = new Adafruit_PN532(PN532_SS);
	_eepromStorage = nullptr;
}

void XReader::begin()
{
#ifndef ESP8266
	while (!Serial); // for Leonardo/Micro/Zero
#endif
	Serial.begin(115200);
	
  _eepromStorage = new EEPROMStorageHandler(&Serial);
  pinMode(_blueLedPin, OUTPUT);
  pinMode(_greenLedPin, OUTPUT);
  pinMode(_redLedPin, OUTPUT);
  pinMode(_buzzerPin, OUTPUT);
  pinMode(_openDoorPin, OUTPUT);

  switchOnLed(_blueLedPin);

  _board->begin();

  checkConnectionToPn532();
}

void XReader::checkConnectionToPn532() const
{
	const uint32_t versionData = _board->getFirmwareVersion();
	if (!versionData) {
		Serial.print("Didn't find PN53x board");
		while (true); // halt
	}

#ifdef DEBUG
	Serial.print("Found chip PN5"); Serial.println((versiondata >> 24) & 0xFF, HEX);
	Serial.print("Firmware ver. "); Serial.print((versiondata >> 16) & 0xFF, DEC);
	Serial.print('.'); Serial.println((versiondata >> 8) & 0xFF, DEC);
#endif

	initBoard();
}

void XReader::loopProcedure()
{
	uint8_t uid[] = { 0, 0, 0, 0, 0, 0, 0 };
	uint8_t uidLength;

	const boolean success = _board->readPassiveTargetID(PN532_MIFARE_ISO14443A, &uid[0], &uidLength);

	if (success) 
	{
		if (_eepromStorage->isMasterCard(&uid[0], uidLength)) 
		{
			registeringNewCard();
		}
		else if (_eepromStorage->isCardRegistered(&uid[0], uidLength)) 
		{
			successfulAuth();
		}
		else 
		{
			successfulAuth();
			//unsuccessfulAuth();
		}
	}
	else
	{
#ifdef DEBUG
		Serial.println("Timed out waiting for a card");
		Serial.print("Time: "); Serial.println(millis());
#endif
	}
}

void XReader::initBoard() const
{
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

void XReader::soundUnsuccessAuthBuzzerOn() const
{
	for (int i = 0; i < 3; ++i)
	{
		tone(_buzzerPin, INVALID_SOUND, 200);
		delay(333);
	}
}

void XReader::switchSuccessAuthBuzzerOn() const
{
	tone(_buzzerPin, VALID_SOUND, 1000);
}

void XReader::unsuccessfulAuth()
{
	Serial.println("#######THIS IS NOT REGISTERED CARD ######");
	_consecutiveFails++;

	switchOffLed(_blueLedPin);
	switchOnLed(_redLedPin);
	soundUnsuccessAuthBuzzerOn();

	const unsigned int logDelay = log(_consecutiveFails) * 1000;


	//incremental delay
	delay(logDelay);

	switchOffLed(_redLedPin);
	switchOnLed(_blueLedPin);
}

void XReader::successfulAuth()
{
	Serial.println("=======THIS IS REGISTERED CARD======");
	_consecutiveFails = 0;

	switchOffLed(_blueLedPin);
	switchOnLed(_greenLedPin);
	//switchSuccessAuthBuzzerOn();

	openDoor();

	switchOffLed(_greenLedPin);
	switchOnLed(_blueLedPin);
}

void XReader::registeringNewCard()
{
	uint8_t uid[] = { 0, 0, 0, 0, 0, 0, 0 };
	uint8_t uidLength;
	Serial.println("=======THIS IS MASTERCARD======");
	_consecutiveFails = 0;

	Serial.println("Waiting for new card to register...");
	delay(1000); // delay otherwise it'd register master card
	switchOnLed(_greenLedPin);
	switchOffLed(_blueLedPin);

	_board->readPassiveTargetID(PN532_MIFARE_ISO14443A, &uid[0], &uidLength);
	_eepromStorage->registerNewCard(&uid[0], uidLength);

	//TODO: maybe add sound confirmation? Blink?

	switchOffLed(_greenLedPin);
	switchOnLed(_blueLedPin);
}

void XReader::openDoor() const
{
	switchOnLed(_openDoorPin);
	delay(DOOR_OPENED_INTERVAL);
	switchOffLed(_openDoorPin);	
}

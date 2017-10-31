#include "XReader.h"

XReader::XReader() 
{
	_board = new Adafruit_PN532(PN532_SS);
	_eepromStorage = nullptr;
	_soundHelper = new SoundHelper(_buzzerPin);
}

void XReader::begin()
{
#ifndef ESP8266
	// ReSharper disable once CppPossiblyErroneousEmptyStatements
	while (!Serial); // for Leonardo/Micro/Zero
#endif
	Serial.begin(115200);
	
  _eepromStorage = new EEPROMStorageHandler();
  pinMode(_blueLedPin, OUTPUT);
  pinMode(_greenLedPin, OUTPUT);
  pinMode(_redLedPin, OUTPUT);
  pinMode(_openDoorPin, OUTPUT);
  pinMode(_button1Pin, INPUT);
  
  _board->begin();

  checkConnectionToPn532();

  switchPinOn(_blueLedPin);


  //Test section starts
  _eepromStorage->deleteMemory();

  uint8_t masterCardId[7] = { 240, 39, 150, 187, 0, 0, 0 };
  _eepromStorage->setMasterCard(masterCardId, 4 * sizeof(uint8_t));
  _eepromStorage->setPin(0, 123456789);


 // Serial.print("MasterCard: "); Serial.println(_eepromStorage->getMasterCardId());

  //4,045,903,547
  //uint8_t newCardId[4] = { 241, 39, 150, 187 };
  //_eepromStorage->registerNew4BCard(&newCardId[0]);

  //_eepromStorage->getNew4BCardAddress();

  _eepromStorage->printMemory();

}

void XReader::checkConnectionToPn532() const
{
	const uint32_t versionData = _board->getFirmwareVersion();
	if (!versionData) {
		Serial.print("Didn't find PN53x board");
		while (true); // halt
	}


	initBoard();
}

void XReader::loopProcedure()
{
	uint8_t uid[] = { 0, 0, 0, 0, 0, 0, 0 };
	uint8_t uidLength;

	if(digitalRead(_button1Pin) == HIGH)
	{
		// ReSharper disable once CppPossiblyErroneousEmptyStatements
		while (digitalRead(_button1Pin) == HIGH);
#ifdef DEBUG
		Serial.println("BUTTON 1 PRESSED.");
#endif
		//TODO: BUTTON 1 PRESSED
	}

	const boolean success = _board->readPassiveTargetID(PN532_MIFARE_ISO14443A, &uid[0], &uidLength);
	
	if (success) 
	{
		if (_eepromStorage->isMasterCard(uid, uidLength)) 
		{
			registeringNewCard();
		}
		else if (_eepromStorage->isCardRegistered(uid, uidLength)) 
		{
			successfulAuth();
		}
		else 
		{
			unsuccessfulAuth();
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

void XReader::switchPinOn(const unsigned char ledPin) {
	digitalWrite(ledPin, HIGH);
}

void XReader::switchPinOff(const unsigned char ledPin) {
	digitalWrite(ledPin, LOW);
}


void XReader::unsuccessfulAuth()
{
	Serial.println("###### THIS IS NOT REGISTERED CARD ######");
	_consecutiveFails++;

	switchPinOff(_blueLedPin);
	switchPinOn(_redLedPin);
	_soundHelper->soundUnsuccessAuthBuzzerOn();

	//incremental delay
	const unsigned int logDelay = log(_consecutiveFails) * 1000;
	delay(logDelay);

	switchPinOff(_redLedPin);
	switchPinOn(_blueLedPin);
}

void XReader::successfulAuth()
{
	Serial.println("=======THIS IS REGISTERED CARD======");
	_consecutiveFails = 0;

	switchPinOff(_blueLedPin);
	switchPinOn(_greenLedPin);
	//_soundHelper->switchSuccessAuthBuzzerOn();

	openDoor();

	switchPinOff(_greenLedPin);
	switchPinOn(_blueLedPin);
}

void XReader::registeringNewCard()
{
	uint8_t uid[] = { 0, 0, 0, 0, 0, 0, 0 };
	uint8_t uidLength;
	Serial.println("=======THIS IS MASTERCARD======");
	_consecutiveFails = 0;

	Serial.println("Waiting for new card to register...");
	switchPinOn(_greenLedPin);
	switchPinOff(_blueLedPin);

	_soundHelper->soundSuccessNoticeSound();
	delay(500);
	_soundHelper->waitingForNewCardSound();


	_board->setPassiveActivationRetries(0xFF);//wait for new card until it is read.

#ifdef DEBUG
	const unsigned long mills = millis();
#endif

	const bool success = _board->readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength, 10000);
	_soundHelper->stopSound();
	
#ifdef DEBUG
	Serial.print("Milliseconds: "); Serial.print(millis() - mills); Serial.println("ms.");
#endif
		
	if(success)
	{
		_eepromStorage->registerNewCard(uid, uidLength);
		delay(300);
		_soundHelper->switchSuccessAuthBuzzerOn();

		for (size_t i = 0; i < 10; i++)
		{
			delay(50);
			switchPinOn(_greenLedPin);
			delay(50);
			switchPinOff(_greenLedPin);
		}
	} 
	else
	{
		switchPinOff(_greenLedPin);
		switchPinOn(_redLedPin);
		_soundHelper->soundUnsuccessAuthBuzzerOn();
		delay(500);
		switchPinOff(_redLedPin);
	}

	delay(1000);
	switchPinOn(_blueLedPin);
	_board->setPassiveActivationRetries(0x01);
}

void XReader::openDoor() const
{
	switchPinOn(_openDoorPin);
	delay(DOOR_OPENED_INTERVAL);
	switchPinOff(_openDoorPin);	
}

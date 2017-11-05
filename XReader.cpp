#include "XReader.h"
#include "RfidCard.h"

XReader::XReader() 
{
	_eepromStorage = new EEPROMStorageHandler();

	const uint8_t _pn532SS_Pin = 10;
	_board = new Adafruit_PN532(_pn532SS_Pin);

	const uint8_t	_buzzerPin = 2;
	_soundHelper = new SoundHelper(_buzzerPin);

	const uint8_t	_redLedPin = 9;
	const uint8_t	_blueLedPin = 8;
	const uint8_t	_greenLedPin = 7;
	_ledHelper = new LedHelper(_blueLedPin, _greenLedPin, _redLedPin);
}

void XReader::begin()
{
#ifndef ESP8266
	// ReSharper disable once CppPossiblyErroneousEmptyStatements
	while (!Serial); // for Leonardo/Micro/Zero
#endif
	Serial.begin(115200);
	
  pinMode(_openDoorPin, OUTPUT);
  pinMode(_button1Pin, INPUT);
  
  _board->begin();

  checkConnectionToPn532();

  _ledHelper->switchPowerLedOn();


  //Test section starts
  //_eepromStorage->deleteMemory();

  /*
  uint8_t masterCardId[7] = { 240, 39, 150, 187, 0, 0, 0 };
  _eepromStorage->setMasterCard(masterCardId, 4 * sizeof(uint8_t));
  _eepromStorage->setPin(0, 123456789);
  */

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
		_ledHelper->switchFailLedOn();
		while (true); // halt
	}
	
	initBoard();
}

void XReader::loopProcedure()
{
	RfidCard card;
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

	const boolean success = _board->readPassiveTargetID(PN532_MIFARE_ISO14443A, card.cardUid, &uidLength);

	card.length = UidSize(uidLength);	
	
	if (success) 
	{
		if (_eepromStorage->isMasterCard(card)) 
		{
			registeringNewCard();
		}
		else if (_eepromStorage->isCardRegistered(card)) 
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

	Serial.println("Running...");
}

void XReader::unsuccessfulAuth()
{
	Serial.println("###### THIS IS NOT REGISTERED CARD ######");
	_consecutiveFails++;

	_ledHelper->switchFailAuthIndicationOn();
	_soundHelper->soundUnsuccessAuthBuzzerOn();

	//incremental delay
	const unsigned int logDelay = log(_consecutiveFails) * 1000;
	delay(logDelay);

	_ledHelper->switchFailAuthIndicationOff();
}

void XReader::successfulAuth()
{
	Serial.println("=======THIS IS REGISTERED CARD======");
	_consecutiveFails = 0;

	_ledHelper->switchSuccessAuthIndicationOn();

	openDoor();

	_ledHelper->switchSuccessAuthIndicationOff();
}

void XReader::registeringNewCard()
{
	RfidCard card;
	uint8_t uidLength;
	Serial.println("=======THIS IS MASTERCARD======");
	_consecutiveFails = 0;

	Serial.println("Waiting for a new card to register...");
	_ledHelper->switchSuccessAuthIndicationOn();

	_soundHelper->soundSuccessNoticeSound();
	delay(500);
	_soundHelper->waitingForNewCardSound();

	_board->setPassiveActivationRetries(0xFF);//wait for new card until it is read.

#ifdef DEBUG
	const unsigned long mills = millis();
#endif

	const bool success = _board->readPassiveTargetID(PN532_MIFARE_ISO14443A, card.cardUid, &uidLength, 10000);
	card.length = UidSize(uidLength);
	_soundHelper->stopSound();
	
#ifdef DEBUG
	Serial.print("Milliseconds: "); Serial.print(millis() - mills); Serial.println("ms.");
#endif
		
	if(success)
	{
		_eepromStorage->registerNewCard(card);
		delay(300);
		_soundHelper->switchSuccessAuthBuzzerOn();
		_ledHelper->switchSuccessRegistrationIndicationOn();
	} 
	else
	{
		_ledHelper->switchFailedRegistrationIndicationOn();
		_soundHelper->soundUnsuccessAuthBuzzerOn();
		delay(500);
		_ledHelper->switchFailLedOff();
	}

	delay(1000);
	_ledHelper->switchPowerLedOn();
	_board->setPassiveActivationRetries(0x01);
}

void XReader::openDoor() const
{
	switchPinOn(_openDoorPin);
	delay(DOOR_OPENED_INTERVAL);
	switchPinOff(_openDoorPin);
}

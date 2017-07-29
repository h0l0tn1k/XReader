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
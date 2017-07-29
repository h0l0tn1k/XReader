#pragma once
#include <Adafruit_PN532.h>


#define PN532_SS   (10)
class XReader {
private:
	Adafruit_PN532* _board;

public:
	XReader();
	void begin();
	void checkConnectionToPn532();
private:
	void initBoard();
};
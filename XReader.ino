#include <Adafruit_PN532.h>
#include <Wire.h>
#include <SPI.h>
#include "XReader.h"

XReader* xreader = new XReader();

void setup()
{
	xreader->begin();

	xreader->checkConnectionToPn532();
}

void loop()
{
	xreader->loopProcedure();
}
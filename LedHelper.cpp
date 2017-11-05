#include "Api\LedHelper.h"


LedHelper::LedHelper(uint8_t powerPin, uint8_t successPin, uint8_t failPin)
{
	powerLedPin = powerPin;
	successLedPin = successPin;
	failLedPin = failPin;

	pinMode(powerLedPin, OUTPUT);
	pinMode(successLedPin, OUTPUT);
	pinMode(failLedPin, OUTPUT);
}

LedHelper::~LedHelper()
{
}

void LedHelper::switchPowerLedOn() const
{
	switchPinOn(powerLedPin);
}

void LedHelper::switchPowerLedOff() const
{
	switchPinOff(powerLedPin);
}

void LedHelper::switchSuccessLedOn() const
{
	switchPinOn(successLedPin);
}

void LedHelper::switchSuccessLedOff() const
{
	switchPinOff(successLedPin);
}

void LedHelper::switchFailLedOn() const
{
	switchPinOn(failLedPin);
}

void LedHelper::switchFailLedOff() const
{
	switchPinOff(failLedPin);
}

void LedHelper::switchFailAuthIndicationOn() const
{
	switchPowerLedOff();
	switchFailLedOn();
}

void LedHelper::switchFailedRegistrationIndicationOn() const
{
	switchSuccessLedOff();
	switchFailLedOn();
}

void LedHelper::switchFailAuthIndicationOff() const
{
	switchFailLedOff();
	switchPowerLedOn();
}

void LedHelper::switchSuccessAuthIndicationOn() const
{
	switchPowerLedOff();
	switchSuccessLedOn();
}

void LedHelper::switchSuccessAuthIndicationOff() const
{
	switchSuccessLedOff();
	switchPowerLedOn();
}

void LedHelper::switchSuccessRegistrationIndicationOn() const
{
	//blink with Successful LED
	for (uint8_t i = 0; i < 10; i++)
	{
		delay(50);
		switchSuccessLedOn();
		delay(50);
		switchSuccessLedOff();
	}
}

#include "Api\PinHelper.h"

void PinHelper::switchPinOn(uint8_t pin) const
{
	digitalWrite(pin, HIGH);
}

void PinHelper::switchPinOff(uint8_t pin) const
{
	digitalWrite(pin, LOW);
}

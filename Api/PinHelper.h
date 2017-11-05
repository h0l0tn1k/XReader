#pragma once
#include "Interfaces\IPinHelper.h"
#include <Arduino.h>

class PinHelper : public IPinHelper
{
public:
	void switchPinOn(uint8_t pin) const override;
	void switchPinOff(uint8_t pin) const override;
};


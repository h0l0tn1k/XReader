#pragma once
#include <stdint.h>

class IPinHelper
{
public:
	virtual ~IPinHelper() = default;

	/**
	* \brief Switches on Pin at \param pin
	* \param pin Pin to be switched on
	*/
	virtual void switchPinOn(uint8_t pin) const = 0;

	/**
	* \brief Switches off Pin at \param pin
	* \param pin Pin to be switched off
	*/
	virtual void switchPinOff(uint8_t pin) const = 0;
};
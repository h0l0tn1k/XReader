#pragma once
#include <stdint.h>
#include <Arduino.h>
#include "Interfaces\ILedHelper.h"
#include "PinHelper.h"


class LedHelper : public ILedHelper, protected PinHelper
{
private:
	uint8_t successLedPin;
	uint8_t failLedPin;
	uint8_t powerLedPin;

public:
	/**
	 * \brief Initializes variables & pins to OUTPUT mode
	 * \param powerPin pin number where LED indicating device is powered on is located
	 * \param successPin pin number where LED indicating Success is located
	 * \param failPin pin number where LED indicating Fail is located
	 */
	LedHelper(uint8_t powerPin, uint8_t successPin, uint8_t failPin);

	/**
	 * \brief Class destructor
	 */
	~LedHelper();

	/**
	 * \brief Switches Power LED On
	 */
	void switchPowerLedOn() const override;

	/**
	* \brief Switches Power LED Off
	*/
	void switchPowerLedOff() const override;

	/**
	* \brief Switches Success LED On
	*/
	void switchSuccessLedOn() const override;

	/**
	* \brief Switches Success LED Off
	*/
	void switchSuccessLedOff() const override;

	/**
	* \brief Switches Fail LED On
	*/
	void switchFailLedOn() const override;

	/**
	* \brief Switches Fail LED Off
	*/
	void switchFailLedOff() const override;

	/**
	* \brief Switches on Indicates Failed Authorization
	*/
	void switchFailAuthIndicationOn() const override;

	/**
	* \brief Switches off Indicates Failed Authorization
	*/
	void switchFailAuthIndicationOff() const override;

	/**
	* \brief Switches on Indicates Successful Authorization
	*/
	void switchSuccessAuthIndicationOn() const override;

	/**
	* \brief Switches off Indicates Successful Authorization
	*/
	void switchSuccessAuthIndicationOff() const override;

	/**
	* \brief Indicates Successful Registration of a new card
	*/
	void switchSuccessRegistrationIndicationOn() const override;

	/**
	* \brief Indicates Unsuccessful Registration of a new card
	*/
	void switchFailedRegistrationIndicationOn() const override;
};


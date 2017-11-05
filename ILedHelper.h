#pragma once

class ILedHelper
{
public:
	virtual ~ILedHelper() = default;

	/**
	* \brief Switches Power LED On
	*/
	virtual void switchPowerLedOn() const = 0;

	/**
	* \brief Switches Power LED Off
	*/
	virtual void switchPowerLedOff() const = 0;

	/**
	* \brief Switches Success LED On
	*/
	virtual void switchSuccessLedOn() const = 0;

	/**
	* \brief Switches Success LED Off
	*/
	virtual void switchSuccessLedOff() const = 0;

	/**
	* \brief Switches Fail LED On
	*/
	virtual void switchFailLedOn() const = 0;

	/**
	* \brief Switches Fail LED Off
	*/
	virtual void switchFailLedOff() const = 0;

	/**
	* \brief Switches on Indicates Failed Authorization
	*/
	virtual void switchFailAuthIndicationOn() const = 0;

	/**
	* \brief Switches off Indicates Failed Authorization
	*/
	virtual void switchFailAuthIndicationOff() const = 0;

	/**
	* \brief Switches on Indicates Successful Authorization
	*/
	virtual void switchSuccessAuthIndicationOn() const = 0;

	/**
	* \brief Switches off Indicates Successful Authorization
	*/
	virtual void switchSuccessAuthIndicationOff() const = 0;

	/**
	* \brief Indicates Successful Registration of a new card
	*/
	virtual void switchSuccessRegistrationIndicationOn() const = 0;

	/**
	* \brief Indicates Unsuccessful Registration of a new card
	*/
	virtual void switchFailedRegistrationIndicationOn() const = 0;
};
#pragma once
// ReSharper disable once CppUnusedIncludeDirective
#include <Arduino.h>
#include "Interfaces\ISoundHelper.h"

#define INVALID_SOUND (261)
#define VALID_SOUND (3000)
#define NEW_CARD_SOUND (3520)

class SoundHelper : public ISoundHelper
{
	const unsigned char _buzzerPin;

public:
	
	/**
	 * \brief Constructor initializes variables and sets _buzzerPin as OUTPUT
	 * \param buzzerPin pin on which buzzer is connected
	 */
	SoundHelper(const unsigned char buzzerPin);

	/**
	* \brief Switches on buzzer sound for unsuccessful authorization, shuts down after 1s
	*/
	void soundUnsuccessAuthBuzzerOn() const override;

	/**
	* \brief Triggers sound for noticing that master card was read
	*/
	void soundSuccessNoticeSound() const override;

	/**
	* \brief Switches on buzzer sound for successful authorization, shuts down after 1s
	*/
	void switchSuccessAuthBuzzerOn() const override;

	/**
	* \brief Triggers sound for noticing that Reader is waiting for new card to read
	*/
	void waitingForNewCardSound() const override;

	/**
	* \brief Mutes all sounds on _buzzerPin
	*/
	void stopSound() const override;
};
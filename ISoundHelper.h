#pragma once
class ISoundHelper
{
public:
	virtual ~ISoundHelper() = default;

	/**
	* \brief Switches on buzzer sound for unsuccessful authorization, shuts down after 1s
	*/
	virtual void soundUnsuccessAuthBuzzerOn() const = 0;

	/**
	* \brief Triggers sound for noticing that master card was read
	*/
	virtual void soundSuccessNoticeSound() const = 0;

	/**
	* \brief Switches on buzzer sound for successful authorization, shuts down after 1s
	*/
	virtual void switchSuccessAuthBuzzerOn() const = 0;

	/**
	* \brief Triggers sound for noticing that Reader is waiting for new card to read
	*/
	virtual void waitingForNewCardSound() const = 0;

	/**
	* \brief Mutes all sounds on _buzzerPin
	*/
	virtual void stopSound() const = 0;
};


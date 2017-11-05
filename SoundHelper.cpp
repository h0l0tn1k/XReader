#include "Api\SoundHelper.h"

SoundHelper::SoundHelper(const unsigned char buzzerPin)
	:_buzzerPin(buzzerPin)
{
	pinMode(_buzzerPin, OUTPUT);
}

void SoundHelper::soundUnsuccessAuthBuzzerOn() const
{
	for (int i = 0; i < 3; ++i)
	{
		tone(_buzzerPin, INVALID_SOUND, 200);
		delay(333);
	}
}

void SoundHelper::soundSuccessNoticeSound() const
{
	for (int i = 0; i < 5; ++i)
	{
		tone(_buzzerPin, VALID_SOUND);
		delay(50);
		stopSound();
		delay(50);
	}
}

void SoundHelper::switchSuccessAuthBuzzerOn() const
{
	tone(_buzzerPin, VALID_SOUND, 1000);
}

void SoundHelper::waitingForNewCardSound() const
{
	tone(_buzzerPin, NEW_CARD_SOUND, 10000);
}

void SoundHelper::stopSound() const
{
	noTone(_buzzerPin);
}

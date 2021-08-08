#include "PhoneSim.h"
#include "Tone.h"
#include "Arduino.h"

void PhoneSim::Play(int freq1, int freq2, int duration, bool wait)
{
	if (duration > 0)
		setDacToneMaxDuration(pin, duration);

	dacToneBegin(pin, freq1, freq2);

	if (wait)
		delay(duration);
}

PhoneSim::PhoneSim(int pin)
	:pin(pin)
{
	
}


void PhoneSim::Stop()
{
	dacToneEnd(pin);
}

void PhoneSim::DialTone(int duration, bool wait)
{
	setDacToneInfinite(pin);
	Play(350, 440, duration, wait);
}

void PhoneSim::Dial(const char* phoneNumber, int toneDelay, int toneDuration)
{
	const size_t len = strlen(phoneNumber);
	for(size_t i = 0; i < len; i++)
	{
		const char value = phoneNumber[i];
		if (!(value == 35 || value == 42 || // # and *
			(value >= 48 && value <= 57) || // 0-9
			(value >= 65 && value <= 68) || // A-D
			(value >= 97 && value <= 100))) // a-d
		{
			// invalid dtmf digit
			continue;
		}

		Dtmf(value, toneDuration);
		
		if(i != len -1)
		{
			delay(toneDelay);
		}
	}
}

void PhoneSim::Dtmf(char c, int duration, bool wait)
{
	int freq1 = 0;
	int freq2 = 0;
	switch (c)
	{
		case '1':
			freq1 = 697;
			freq2 = 1209;
			break;
		case '2':
			freq1 = 697;
			freq2 = 1336;
			break;
		case '3':
			freq1 = 697;
			freq2 = 1477;
			break;
		case '4':
			freq1 = 770;
			freq2 = 1209;
			break;
		case '5':
			freq1 = 770;
			freq2 = 1336;
			break;
		case '6':
			freq1 = 770;
			freq2 = 1477;
			break;
		case '7':
			freq1 = 852;
			freq2 = 1209;
			break;
		case '8':
			freq1 = 852;
			freq2 = 1336;
			break;
		case '9':
			freq1 = 852;
			freq2 = 1477;
			break;
		case '0':
			freq1 = 941;
			freq2 = 1336;
			break;
		case 'A':
		case 'a':
			freq1 = 697;
			freq2 = 1633;
			break;
		case 'B':
		case 'b':
			freq1 = 770;
			freq2 = 1633;
			break;
		case 'C':
		case 'c':
			freq1 = 852;
			freq2 = 1633;
			break;
		case 'D':
		case 'd':
			freq1 = 941;
			freq2 = 1633;
			break;
		case '*':
			freq1 = 941;
			freq2 = 1209;
			break;
		case '#':
			freq1 = 941;
			freq2 = 1477;
			break;
	}

	if (freq1 > 0 || freq2 > 0)
	{
		setDacToneInfinite(pin);
		Play(freq1, freq2, duration, wait);
	}
}

void PhoneSim::Ring(int duration, bool wait)
{
	setDacToneInfinite(pin);
	setDacToneDuration(pin, 2000);
	setDacToneSilenceDuration(pin, 4000);
	Play(440, 480, duration, wait);	
}

void PhoneSim::Busy(int duration, bool wait)
{
	setDacToneInfinite(pin);
	setDacToneDuration(pin, 500);
	setDacToneSilenceDuration(pin, 500);
	Play(480, 620, duration, wait);	
}






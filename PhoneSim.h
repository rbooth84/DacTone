#pragma once
class PhoneSim
{
private:
	int pin = 0;
	void Play(int freq1, int freq2, int duration, bool wait);
public:
	PhoneSim(int pin);
	void Stop();
	void DialTone(int duration = 0, bool wait = false);
	void Dial(const char* phoneNumber, int toneDelay = 75, int toneDuration = 150);
	void Dtmf(char c, int duration = 150, bool wait = true);	
	void Ring(int duration = 0, bool wait = false);
	void Busy(int duration = 0, bool wait = false);
};


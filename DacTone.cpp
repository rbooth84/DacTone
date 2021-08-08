#include "DacTone.h"
#include "Arduino.h"

#define Tau 6.283185307179586
#define MAX_DAC_TONES 2
#define DAC_TONE_DEFAULT_SAMPLE 24000

typedef struct
{
	uint8_t pin;
	bool active;
	bool processing;
	uint32_t freq1;
	uint32_t freq2;
	float volume;
	unsigned long maxDuration;
	unsigned long toneDuration;
	unsigned long silenceDuration;
} dacToneSetting;


dacToneSetting settings[MAX_DAC_TONES];

void RestDacToneSetting(dacToneSetting* setting)
{
	setting->pin = 0;
	setting->active = false;
	setting->processing = false;
	setting->freq1 = 0;
	setting->freq2 = 0;
	setting->volume = 1.0;
	setting->maxDuration = 0;
	setting->toneDuration = 0;
	setting->silenceDuration = 0;
}

void printSetting(dacToneSetting* setting, int index = 0)
{
	Serial.printf("settings[%d]\n", index);
	Serial.printf(" .pin=%d\n", setting->pin);
	Serial.printf(" .active=%s\n", setting->active ? "true" : "false");
	Serial.printf(" .processing=%s\n", setting->processing ? "true" : "false");
	Serial.printf(" .freq1=%d\n", setting->freq1);
	Serial.printf(" .freq2=%d\n", setting->freq2);
	Serial.printf(" .maxDuration=%d\n", setting->maxDuration);
	Serial.printf(" .toneDuration=%d\n", setting->toneDuration);
	Serial.printf(" .silenceDuration=%d\n", setting->silenceDuration);
	Serial.printf(" .volume=%d\n", setting->volume);
}

dacToneSetting* GetDacToneSetting(uint8_t pin)
{
	int firstZero = -1;
	for (int i = 0; i < MAX_DAC_TONES; i++)
	{		
		if (firstZero == -1 && settings[i].pin == 0)
			firstZero = i;

		if (settings[i].pin == pin)
			return &settings[i];
	}
	
	if (firstZero == -1)
		return nullptr;


	RestDacToneSetting(&settings[firstZero]);
	settings[firstZero].pin = pin;
	return &settings[firstZero];
}

int GetActiveDacTones()
{
	int count = 0;
	for (int i = 0; i < MAX_DAC_TONES; i++)
	{
		if (settings[i].pin == 0)
			continue;

		if (settings[i].active)
			count++;
	}
	return count;
}

uint32_t dacToneSampleRate = DAC_TONE_DEFAULT_SAMPLE;
uint32_t currentToneSampleRate = DAC_TONE_DEFAULT_SAMPLE;
hw_timer_t* dacToneTimer = NULL;
portMUX_TYPE dacToneTimerMux = portMUX_INITIALIZER_UNLOCKED;
volatile uint32_t counter = 0;

void IRAM_ATTR onDacToneTimer()
{
	portENTER_CRITICAL_ISR(&dacToneTimerMux);
	if (counter == currentToneSampleRate)
		counter = 0;
	else
		counter++;
	portEXIT_CRITICAL_ISR(&dacToneTimerMux);
}

void enableTimer()
{
	if (dacToneTimer == NULL)
	{
		currentToneSampleRate = dacToneSampleRate;
		counter = 0;
		dacToneTimer = timerBegin(0, 80, true);
		timerAttachInterrupt(dacToneTimer, &onDacToneTimer, true);
		timerAlarmWrite(dacToneTimer, 1000000 / currentToneSampleRate, true);
		timerAlarmEnable(dacToneTimer);
	}
}

void disableTimer()
{
	if(GetActiveDacTones() == 0)
	{
		timerAlarmDisable(dacToneTimer);
		timerDetachInterrupt(dacToneTimer);
		timerEnd(dacToneTimer);
		dacToneTimer = NULL;
	}
}

void dacToneLoop(void* parm)
{	
	dacToneSetting* settings = (dacToneSetting*)parm;
	
	unsigned long now = millis();
	unsigned long nextStateSwitch = now + settings->toneDuration;
	unsigned long stopAt = now + settings->maxDuration;

	double dacToneWaveValue = 0.0;
	double dacToneTime = 0.0;
	uint8_t dacToneDacValue = 0;
	bool dacTonePlaying = true;
	int lastCounter = counter;
	
	while (settings->active)
	{
		taskYIELD();
		
		if (lastCounter == counter)
			continue;
		
		lastCounter = counter;
		
		now = millis();
		
		if(settings->maxDuration > 0 && now > stopAt)
		{
			settings->active = false;
			disableTimer();
			break;
		}
		
		if(settings->toneDuration != 0)
		{
			if(now >= nextStateSwitch)
			{				
				dacTonePlaying = !dacTonePlaying;
				nextStateSwitch = now + (dacTonePlaying ? settings->toneDuration : settings->silenceDuration);
				if(!dacTonePlaying)
					dacWrite(settings->pin, 0);
			}
			
			if(!dacTonePlaying)
			{
				continue;
			}
		}
		
		portENTER_CRITICAL(&dacToneTimerMux);
		dacToneTime = (double)counter / currentToneSampleRate;
		portEXIT_CRITICAL(&dacToneTimerMux);

		dacToneWaveValue = (sin(dacToneTime * settings->freq1 * Tau) + sin(dacToneTime * settings->freq2 * Tau)) * .5 * settings->volume;
		dacToneDacValue = (uint8_t)(dacToneWaveValue * 127.0 + 128.0);
		dacWrite(settings->pin, dacToneDacValue);
	}
	dacWrite(settings->pin, 0);
	settings->processing = false;
	vTaskDelete(NULL);
}

void Tone(uint8_t pin, uint32_t freq1, uint32_t duration)
{
	ToneMix(pin, freq1, 0, duration);
}

void ToneMix(uint8_t pin, uint32_t freq1, uint32_t freq2, uint32_t duration)
{
	setDacToneInfinite(pin);

	if (duration > 0)
		setDacToneMaxDuration(pin, duration);

	dacToneBegin(pin, freq1, freq2);

	if (duration > 0)
		delay(duration);
}

void noTone(uint8_t pin)
{
	dacToneEnd(pin);
}

void dacToneBegin(uint8_t pin, uint32_t freq1, uint32_t freq2)
{
	dacToneSetting* setting = GetDacToneSetting(pin);
	
	if (setting->active)
		dacToneEnd(pin);
	
	setting->active = true;
	setting->processing = true;
	setting->freq1 = freq1;
	setting->freq2 = freq2;
	
	enableTimer();

	xTaskCreatePinnedToCore(
		dacToneLoop,
		"dacToneLoop",
		2000,
		setting,
		1,
		NULL,
		1
	);
}

void dacToneEnd(uint8_t pin)
{
	dacToneSetting* setting = GetDacToneSetting(pin);
	if (setting->active)
	{
		setting->active = false;
		disableTimer();
		while (setting->processing)
		{
			taskYIELD();
		}
	}
}

void setDacToneInfinite(uint8_t pin)
{
	dacToneSetting* setting = GetDacToneSetting(pin);
	setting->toneDuration = 0;
	setting->silenceDuration = 0;
	setting->maxDuration = 0;
}

void setDacToneMaxDuration(uint8_t pin, unsigned long duration)
{
	GetDacToneSetting(pin)->maxDuration = duration;
}

unsigned long getDacToneMaxDuration(uint8_t pin)
{
	return GetDacToneSetting(pin)->maxDuration;
}

void setDacToneDuration(uint8_t pin, unsigned long duration)
{
	GetDacToneSetting(pin)->toneDuration = duration;
}

unsigned long getDacToneDuration(uint8_t pin)
{
	return GetDacToneSetting(pin)->toneDuration;
}

void setDacToneSilenceDuration(uint8_t pin, unsigned long duration)
{
	GetDacToneSetting(pin)->silenceDuration = duration;
}

unsigned long getDacToneSilenceDuration(uint8_t pin)
{
	return GetDacToneSetting(pin)->silenceDuration;
}

void setDacToneVolume(uint8_t pin, float volume)
{
	GetDacToneSetting(pin)->volume = volume;
}

unsigned long getDacToneVolume(uint8_t pin)
{
	return GetDacToneSetting(pin)->volume;
}

void setDacToneSampleRate(uint32_t rate)
{
	dacToneSampleRate = rate;
	if(dacToneTimer != NULL)
	{
		timerAlarmDisable(dacToneTimer);
		timerDetachInterrupt(dacToneTimer);
		timerEnd(dacToneTimer);
		dacToneTimer = NULL;
		enableTimer();
	}
}

uint32_t getDacToneSampleRate()
{
	return dacToneSampleRate;
}

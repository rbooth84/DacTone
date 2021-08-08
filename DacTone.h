#pragma once

#include "stdint.h"

// Arduino Compatible Methods
void Tone(uint8_t pin, uint32_t freq1, uint32_t duration = 0);
void ToneMix(uint8_t pin, uint32_t freq1, uint32_t freq2, uint32_t duration = 0);
void noTone(uint8_t pin);

// Begins tone generation
void dacToneBegin(uint8_t pin, uint32_t freq1, uint32_t freq2 = 0);

// Stops the Tone and releases resources
void dacToneEnd(uint8_t pin);

// Reset to default
// Allows for a continuous tone that plays until dacToneEnd() is called
void setDacToneInfinite(uint8_t pin);

// How long the Tone should play before it stops
// This allows for playing of Tone + Silence with an Auto Shutdown
void setDacToneMaxDuration(uint8_t pin, unsigned long duration);
unsigned long getDacToneMaxDuration(uint8_t pin);

// How long the Tone should play
// Does not do anything if the Silence Duration is not set
void setDacToneDuration(uint8_t pin, unsigned long duration);
unsigned long getDacToneDuration(uint8_t pin);


// Silence Duration after the Tone plays
void setDacToneSilenceDuration(uint8_t pin, unsigned long duration);
unsigned long getDacToneSilenceDuration(uint8_t pin);

// Volume of the generated tone
void setDacToneVolume(uint8_t pin, float volume);
unsigned long getDacToneVolume(uint8_t pin);


void setDacToneSampleRate(uint32_t rate);
uint32_t getDacToneSampleRate();
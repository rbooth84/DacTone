#include "DacTone.h"
#include "PhoneSim.h"

PhoneSim phone(25);

void setup() {
	Serial.begin(115200);
}

void loop()
{	
	setDacToneSampleRate(48000); // default: 24000
	setDacToneVolume(25, 0.8f); // 0.0 -> 1.0

	// Arduino Tone Compatible methods example
	/*
	Tone(25, 480, 1000);
	
	Tone(25, 350);
	delay(1000);
	noTone(25);

	// Not an Arduino method but allows for mixing 2 frequencies
	// This is an example of a US dial tone.
	ToneMix(25, 350, 440, 1000);

	ToneMix(25, 350, 440);
	delay(1000);
	noTone(25);
	*/
	
	Serial.println("Dial Tone");
	phone.DialTone(2000, true);

	
	delay(500); // pause before dialing the phone number

	Serial.println("  Dialing: 8655309");
	phone.Dial("865-5309");
	delay(750);

	Serial.println("  Ring"); 
	phone.Ring(6000 + 2000, true); // this will give 2 complete rings
	delay(1000);

	Serial.println("  Busy");
	phone.Busy(5000, true);

	Serial.println("  Stop");
	phone.Stop();
	delay(1500);
}

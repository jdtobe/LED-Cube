#include "main.h"

#include <Arduino.h>
#include <SPI.h>
#include <TimerOne.h>
#include <digitalWriteFast.h>

#include "animations.h"
#include "drawing.h"

void setup() {
	// Serial.begin(9600);
	Serial.begin(115200);

	for (int z = 0; z < tall; z++) {
		pinModeFast(zPins[z], OUTPUT);
		digitalWriteFast(zPins[z], HIGH);
	}
	pinModeFast(latchPin, OUTPUT);
	SPI.begin();
	SPI.beginTransaction(SPISettings(16000000, MSBFIRST, SPI_MODE0));
	Timer1.initialize(frameTime);
	Timer1.attachInterrupt(refreshLEDs, frameTime);

	Serial.println("App Starting!");
}

void loop() { 
	const uint8_t numAnimations = 5;
	static uint8_t cycle = 0;
	static uint32_t startMillis = millis();
	animationFnPtr animations[] = {&noise, &fillDrain, &rain, &slidingPlanes, &squares};
	// bool stillRunning = noise(millis() - startMillis);
	// bool stillRunning = rain(millis() - startMillis);
	// bool stillRunning = squares(millis() - startMillis);
	// bool stillRunning = slidingPlanes(millis() - startMillis);
	// bool stillRunning = test(millis() - startMillis);
	bool stillRunning = animations[cycle%numAnimations](millis() - startMillis);
	if (!stillRunning) {
		cycle = (cycle+1);
		allOff();
		startMillis = millis();
	}
}

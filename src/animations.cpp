#include "animations.h"

#include <Arduino.h>

#include "drawing.h"

bool fillDrain(uint32_t t) {
	const uint16_t timeDelta = 50;
	static uint32_t deadline;
	static uint8_t led = 0;
	static bool ledVal = true;

	// exit condition: on, off, bye!
	if (ledVal && t > 2*rows*cols*tall*timeDelta) {
		led = 0;
		deadline = 0;
		return false;
	}

	if (t < deadline) {
		return true;
	}
	deadline = t + timeDelta;

	uint8_t x = (led % (rows * cols)) % rows;
	uint8_t y = (led % (rows * cols)) / rows;
	uint8_t z = led / (rows * cols);
	// pixelToggle(x, y, z);
	if (ledVal) {
		pixelOn(x, y, z);
	} else {
		pixelOff(x, y, z);
	}

	led = (led + 1) % (rows * cols * tall);
	if (led == 0) {
		ledVal = !ledVal;
	}

	return true;
}

bool noise(uint32_t t) {
	const uint8_t numLEDs = 2;
	struct led {
		point point;
		bool on;
		uint32_t deadline;
	};
	static led leds[numLEDs];

	if (t > 15000) {
		for (uint8_t i=0; i<numLEDs; i++) {
			leds[i].on = false;
			return false;
		}
	}

	for (uint8_t i=0; i<numLEDs; i++) {
		if (t > leds[i].deadline) {
			pixelOff(leds[i].point.x, leds[i].point.y, leds[i].point.z);
			leds[i].on = false;
		}
		if (!leds[i].on) {
			leds[i].point.x = random(rows);
			leds[i].point.y = random(cols);
			leds[i].point.z = random(tall);
			leds[i].deadline = t + random(30, 60);
			pixelOn(leds[i].point.x, leds[i].point.y, leds[i].point.z);
			leds[i].on = true;
		}
	}
	
	return true;
}

bool rain(uint32_t t) {
	struct drop {
		uint16_t speed;
		uint32_t deadline;
		uint8_t height;
		bool enabled;
	};
	static drop drops[rows * cols];

	// exit condition (5s right now)
	if (t > 10000) {
		for (int i=0; i<rows*cols; i++) {
			drops[i].enabled = false;
		}
		return false;
	}

	// update drops
	for (uint8_t i = 0; i < rows * cols; i++) {
		if (drops[i].enabled && t > drops[i].deadline) {
			pixelOff(i / cols, i % cols, drops[i].height);
			if (drops[i].height == 0) {
				drops[i].enabled = false;
				break;
			}
			drops[i].speed *= 0.5;
			drops[i].height--;
			drops[i].deadline = t + drops[i].speed;
			pixelOn(i / cols, i % cols, drops[i].height);
		}
	}

	// new drops
	if (random(100) < 1.5) {
		uint8_t i = random(rows * cols);
		if (!drops[i].enabled) {
			drops[i].height = tall - 1;
			drops[i].speed = random(200, 350);
			drops[i].deadline =
				t +
				random(500, 1000);	// "hang time" when a drop first shows up
			drops[i].enabled = true;
			pixelOn(i / cols, i % cols, drops[i].height);
		}
	}

	return true;
}

bool slidingPlanes(uint32_t t) {
	static uint16_t timeDelta = 100;
	static uint32_t deadline;

	const uint16_t moveLow = 1500;
	const uint16_t moveHigh = 2500;

	struct plane {
		uint8_t i;
		int8_t di;
		uint8_t max;
		uint32_t deadline;
	};
	static plane planes[3] = {
		{0,1,rows,t+random(moveLow, moveHigh)},
		{0,1,cols,t+random(moveLow, moveHigh)},
		{0,1,tall,t+random(moveLow, moveHigh)},
	};

	if (t > 15000) {
		for (uint8_t p=0; p<3; p++) {
			planes[p].i = 0;
			planes[p].di = 1;
			planes[p].deadline = random(moveLow, moveHigh);
		}
		return false;
	}

	for (uint8_t p=0; p<3; p++) {
		if (t > planes[p].deadline) {
			planes[p].deadline = t + timeDelta;
			uint8_t newI = planes[p].i + planes[p].di;
			if (newI < 0 || newI >= planes[p].max) {
				planes[p].di *= -1;
				planes[p].deadline = t + random(moveLow, moveHigh);
				continue;
			}
			planes[p].i = newI;
		}
	}

	noInterrupts();
	allOff();
	planeOn(X, planes[0].i);
	planeOn(Y, planes[1].i);
	planeOn(Z, planes[2].i);
	interrupts();

	return true;
}

bool squares(uint32_t t) {
	const uint16_t timeDelta = 150;
	static uint32_t deadline;

	const uint8_t numSteps = 74;
	struct step {
		point d1;
		point d2;
		axis ax;
	};
	static const step steps[numSteps]{
		// X Forwards
		{{0,  0,  0}, {0,  0,  1}, X},
		{{0,  1,  0}, {0,  0,  0}, X},
		{{0,  0,  0}, {0, -1,  0}, X},
		{{0,  0,  1}, {0,  0,  0}, X},
		{{0,  0,  0}, {0,  0, -1}, X},
		{{0, -1,  0}, {0,  0,  0}, X},
		{{0,  0,  0}, {0,  1,  0}, X},
		{{0,  0, -1}, {0,  0,  0}, X},
		// X Backwards - Fast
		{{0,  0,  1}, {0, -1,  0}, X},
		{{0,  1,  0}, {0,  0,  1}, X},
		{{0,  0, -1}, {0,  1,  0}, X},
		{{0, -1,  0}, {0,  0, -1}, X},
		// X Backwards
		{{0,  0,  1}, {0,  0,  0}, X},
		{{0,  0,  0}, {0, -1,  0}, X},
		{{0,  1,  0}, {0,  0,  0}, X},
		{{0,  0,  0}, {0,  0,  1}, X},
		{{0,  0, -1}, {0,  0,  0}, X},
		{{0,  0,  0}, {0,  1,  0}, X},
		{{0, -1,  0}, {0,  0,  0}, X},
		{{0,  0,  0}, {0,  0, -1}, X},
		// X Forwards - Fast
		{{0,  1,  0}, {0,  0,  1}, X},
		{{0,  0,  1}, {0, -1,  0}, X},
		{{0, -1,  0}, {0,  0, -1}, X},
		{{0,  0, -1}, {0,  1,  0}, X},

		// Y Forwards
		{{ 0, 0,  0}, { 0, 0,  1}, Y},
		{{ 1, 0,  0}, { 0, 0,  0}, Y},
		{{ 0, 0,  0}, {-1, 0,  0}, Y},
		{{ 0, 0,  1}, { 0, 0,  0}, Y},
		{{ 0, 0,  0}, { 0, 0, -1}, Y},
		{{-1, 0,  0}, { 0, 0,  0}, Y},
		{{ 0, 0,  0}, { 1, 0,  0}, Y},
		{{ 0, 0, -1}, { 0, 0,  0}, Y},
		// Y Backwards - Fast
		{{ 0, 0,  1}, {-1, 0,  0}, Y},
		{{ 1, 0,  0}, { 0, 0,  1}, Y},
		{{ 0, 0, -1}, { 1, 0,  0}, Y},
		{{-1, 0,  0}, { 0, 0, -1}, Y},
		// Y Backwards
		{{ 0, 0,  1}, { 0, 0,  0}, Y},
		{{ 0, 0,  0}, {-1, 0,  0}, Y},
		{{ 1, 0,  0}, { 0, 0,  0}, Y},
		{{ 0, 0,  0}, { 0, 0,  1}, Y},
		{{ 0, 0, -1}, { 0, 0,  0}, Y},
		{{ 0, 0,  0}, { 1, 0,  0}, Y},
		{{-1, 0,  0}, { 0, 0,  0}, Y},
		{{ 0, 0,  0}, { 0, 0, -1}, Y},
		// Y Forwards - Fase
		{{ 1, 0,  0}, { 0, 0,  1}, Y},
		{{ 0, 0,  1}, {-1, 0,  0}, Y},
		{{-1, 0,  0}, { 0, 0, -1}, Y},
		{{ 0, 0, -1}, { 1, 0,  0}, Y},

		// // Z Forwards
		{{ 0,  0, 0}, { 0,  0,  1}, Z}, // raise the roof
		{{ 0,  0, 0}, {-1,  0,  0}, Z},
		{{ 1,  0, 0}, { 0,  0,  0}, Z},
		{{ 0,  0, 0}, { 0, -1,  0}, Z},
		{{ 0,  1, 0}, { 0,  0,  0}, Z},
		{{ 0,  0, 0}, { 1,  0,  0}, Z},
		{{-1,  0, 0}, { 0,  0,  0}, Z},
		{{ 0,  0, 0}, { 0,  1,  0}, Z},
		{{ 0, -1, 0}, { 0,  0,  0}, Z},
		// Z Backwards - Fast
		{{ 0,  1, 0}, { 0, -1,  0}, Z},
		{{ 1,  0, 0}, {-1,  0,  0}, Z},
		{{ 0, -1, 0}, { 0,  1,  0}, Z},
		{{-1,  0, 0}, { 1,  0,  0}, Z},
		// Z Backwards
		{{ 0,  0, 0}, { 0, -1,  0}, Z},
		{{ 0,  1, 0}, { 0,  0,  0}, Z},
		{{ 0,  0, 0}, {-1,  0,  0}, Z},
		{{ 1,  0, 0}, { 0,  0,  0}, Z},
		{{ 0,  0, 0}, { 0,  1,  0}, Z},
		{{ 0, -1, 0}, { 0,  0,  0}, Z},
		{{ 0,  0, 0}, { 1,  0,  0}, Z},
		{{-1,  0, 0}, { 0,  0,  0}, Z},
		// // Z Forwards - Fast
		{{ 1,  0, 0}, {-1,  0,  0}, Z},
		{{ 0,  1, 0}, { 0, -1,  0}, Z},
		{{-1,  0, 0}, { 1,  0,  0}, Z},
		{{ 0, -1, 0}, { 0,  1,  0}, Z},
		{{ 0,  0, 0}, { 0,  0, -1}, Z}, // lower the roof

	};

	static uint8_t cycle=0;
	// bottom two corners
	static point p1={0,0,0};
	static point p2={rows-1,cols-1,0};
	static point d1=steps[cycle].d1;
	static point d2=steps[cycle].d2;

	if (t < deadline) {
		return true;
	}
	deadline = t + timeDelta;

	noInterrupts();
	allOff();
	rectOn(steps[cycle].ax, {p1.x, p1.y, p1.z}, {p2.x, p2.y, p2.z});
	interrupts();

	p1.x += d1.x;
	p1.y += d1.y;
	p1.z += d1.z;
	p2.x += d2.x;
	p2.y += d2.y;
	p2.z += d2.z;

	if (p1.x<0 || p1.x==rows || p1.y<0 || p1.y==cols || p1.z<0 || p1.z == tall || p2.x<0 || p2.x==rows || p2.y<0 || p2.y==cols || p2.z<0 || p2.z == tall) {
		cycle++;
		if (cycle == numSteps) {
			cycle = 0;
			p1 = {0,0,0};
			p2 = {rows-1,cols-1,0};
			d1 = steps[cycle].d1;
			d2 = steps[cycle].d2;
			deadline = 0;

			return false;
		}

		// undo delta
		p1.x -= d1.x;
		p1.y -= d1.y;
		p1.z -= d1.z;
		p2.x -= d2.x;
		p2.y -= d2.y;
		p2.z -= d2.z;

		// load new deltas
		d1 = steps[cycle].d1;
		d2 = steps[cycle].d2;

		deadline = 0; // bypass deadline update to keep the ball rolling.

		return true;
	}

	return true;
}

bool test(uint32_t t) {
	static uint16_t timeDelta = 500;
	static uint32_t deadline = t + timeDelta;

	if (t < deadline) {
		return true;
	}

	deadline = t + timeDelta;

	planeOn(X, 1);
	planeOn(Y, 2);
	planeOn(Z, 1);

	return true;
}

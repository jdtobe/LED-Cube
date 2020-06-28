#include "drawing.h"

#include <Arduino.h>
#include <SPI.h>
#include <digitalWriteFast.h>

// ----- MAIN SCREEN DRAWING ROUTINE ----- //
void refreshLEDs() {
	for (uint8_t z = 0; z < tall; z++) {
		digitalWriteFast(latchPin, LOW);
		SPI.transfer16(pixels[z]);
		digitalWriteFast(latchPin, HIGH);
		digitalWriteFast(zPins[z], LOW);
		delayMicroseconds(250); // display delay
		digitalWriteFast(zPins[z], HIGH);
	}
}

void allOn() {
	for (uint8_t z=0; z<tall; z++) {
		pixels[z]=0xFFFF;
	}
}

void allOff() {
	for (uint8_t z=0; z<tall; z++) {
		pixels[z]=0;
	}
}

// ----- PIXEL OPERATIONS ----- //
void pixelOn(uint8_t x, uint8_t y, uint8_t z) {
	pixels[z] |= (0x1 << (y * rows + x));
}
void pixelOff(uint8_t x, uint8_t y, uint8_t z) {
	pixels[z] &= ~(0x1 << (y * rows + x));
}
void pixelToggle(uint8_t x, uint8_t y, uint8_t z) {
	pixels[z] ^= (0x1 << (y * rows + x));
}

void lineOn(point a, point b) {
	uint8_t dx = ((a.x>b.x)?(a.x-b.x):(b.x-a.x));
	uint8_t dy = ((a.y>b.y)?(a.y-b.y):(b.y-a.y));
	uint8_t dz = ((a.z>b.z)?(a.z-b.z):(b.z-a.z));
	long m = max(max(dx, dy), max(dx, dz)) + 1;
	for (int i=0; i<m; i++) {
		uint8_t x = map(i, 0, m-1, a.x*1000, b.x*1000) / 1000;
		uint8_t y = map(i, 0, m-1, a.y*1000, b.y*1000) / 1000;
		uint8_t z = map(i, 0, m-1, a.z*1000, b.z*1000) / 1000;
		pixelOn(x, y, z);
	}
}
void lineOff(point a, point b) {
	uint8_t dx = ((a.x>b.x)?(a.x-b.x):(b.x-a.x));
	uint8_t dy = ((a.y>b.y)?(a.y-b.y):(b.y-a.y));
	uint8_t dz = ((a.z>b.z)?(a.z-b.z):(b.z-a.z));
	long m = max(max(dx, dy), max(dx, dz)) + 1;
	for (int i=0; i<m; i++) {
		uint8_t x = map(i, 0, m-1, a.x, b.x);
		uint8_t y = map(i, 0, m-1, a.y, b.y);
		uint8_t z = map(i, 0, m-1, a.z, b.z);
		pixelOff(x, y, z);
	}
}

void rectOn(axis ax, point a, point b) {
	switch (ax) {
	case X:
		lineOn({a.x, a.y, a.z}, {b.x, a.y, a.z});
		lineOn({a.x, b.y, b.z}, {b.x, b.y, b.z});
		lineOn({a.x, a.y, a.z}, {a.x, b.y, b.z});
		lineOn({b.x, a.y, a.z}, {b.x, b.y, b.z});
		break;
	case Y:
		lineOn({a.x, a.y, a.z}, {a.x, b.y, a.z});
		lineOn({b.x, a.y, b.z}, {b.x, b.y, b.z});
		lineOn({a.x, a.y, a.z}, {b.x, a.y, b.z});
		lineOn({a.x, b.y, a.z}, {b.x, b.y, b.z});
		break;
	case Z:
		lineOn({a.x, a.y, a.z}, {b.x, b.y, a.z});
		lineOn({a.x, a.y, b.z}, {b.x, b.y, b.z});
		lineOn({a.x, a.y, a.z}, {a.x, a.y, b.z});
		lineOn({b.x, b.y, a.z}, {b.x, b.y, b.z});
		break;
	}
}

void planeOn(axis ax, uint8_t i) {
	uint16_t mask;
	switch (ax) {
	case X:
		mask = (1 << i) | (1 << i+1*rows)  | (1 << i+2*rows)  | (1 << i+3*rows);
		for (int z=0; z<tall; z++) {
			pixels[z] |= mask;
		}
		break;
	case Y:
		mask = (0b1111 << i*rows);
		for (int z=0; z<tall; z++) {
			pixels[z] |= mask;
		}
		break;
	case Z: pixels[i] = 0xFFFF; break;
	}
}

void planeOff(axis ax, uint8_t i) {
	uint16_t mask;
	switch (ax) {
	case X:
		mask = (1 << i) | (1 << i+1*rows)  | (1 << i+2*rows)  | (1 << i+3*rows);
		for (int z=0; z<tall; z++) {
			pixels[z] &= ~mask;
		}
		break;
	case Y:
		mask = (0b1111 << i*rows);
		for (int z=0; z<tall; z++) {
			pixels[z] &= ~mask;
		}
		break;
	case Z: pixels[i] = 0; break;
	}
}

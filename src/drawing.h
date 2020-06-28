#include <Arduino.h>

const int8_t dataPin = 11;
const int8_t clockPin = 13;
const int8_t latchPin = 8;

const uint8_t rows = 4;
const uint8_t cols = 4;
const uint8_t tall = 4;

const uint8_t zPins[tall] = {4, 5, 6, 7};

static volatile uint16_t pixels[tall];
const uint16_t frameRate = 60 * 4;
const uint32_t frameTime = 1000000 / frameRate;

// ----- TYPES ----- //
enum axis {X, Y, Z};
enum orientation {VERTICAL, HORIZONTAL};
struct point {
    int16_t x;
    int16_t y;
    int16_t z;
};

// ----- MAIN SCREEN DRAWING ROUTINE ----- //
void refreshLEDs();
void allOn();
void allOff();

// ----- PIXEL OPERATIONS ----- //
void pixelOn(uint8_t x, uint8_t y, uint8_t z);
void pixelOff(uint8_t x, uint8_t y, uint8_t z);
void pixelToggle(uint8_t x, uint8_t y, uint8_t z);

// ----- SHAPES ----- //
void lineOn(point a, point b);
void lineOff(point a, point b);
void rectOn(axis ax, point a, point b);
void rectOff(axis ax, point a, point b);
void planeOn(axis ax, uint8_t i);
void planeOff(axis ax, uint8_t i);

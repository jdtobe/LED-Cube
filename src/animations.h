#include <Arduino.h>

typedef bool(*animationFnPtr)(uint32_t);

bool fillDrain(uint32_t t);
bool noise(uint32_t t);
bool rain(uint32_t t);
bool slidingPlanes(uint32_t t);
bool squares(uint32_t t);
bool test(uint32_t t);

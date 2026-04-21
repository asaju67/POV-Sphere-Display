#ifndef LED_OUTPUT_H
#define LED_OUTPUT_H

#include <Arduino.h>
#include <stdint.h>

void initLedPins();
void shiftOut64(uint64_t value);

#endif

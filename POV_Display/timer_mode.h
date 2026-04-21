#ifndef TIMER_MODE_H
#define TIMER_MODE_H

#include <Arduino.h>
#include <stdint.h>
#include <stdbool.h>

bool initTimerMode();
bool setTimerModeFromTimeString(const char* timeText);
bool setHexModeFromString(const String& hexSequence);
bool isTimerModeEnabled();
const String& getTimerModeText();
void updateTimerCountdown();

#endif

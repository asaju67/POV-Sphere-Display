#ifndef APP_STATE_H
#define APP_STATE_H

#include <Arduino.h>
#include <WebServer.h>
#include <stdint.h>
#include "config.h"

struct AppState {
  WebServer server;

  uint64_t patternBuffer[MAX_PATTERNS];
  int patternLength;
  int currentPatternIndex;

  unsigned long lastShiftMicros;
  unsigned long fixedShiftIntervalMicros;

  String lastReceivedHex;
  bool timerMode;
  bool timerSpritesReady;
  String timerText;
  int countdownHours;
  int countdownMinutes;
  int countdownSeconds;
  unsigned long lastCountdownUpdateMillis;

  // Hall timing globals
  volatile unsigned long lastHallEdgeMicros;
  volatile unsigned long latestRotationIntervalMicros;
  volatile bool newRotationCaptured;

  // Hall-derived speed state
  unsigned long rotationIntervalsMicros[HALL_RPM_AVERAGE_SAMPLES];
  int rotationIntervalIndex;
  int rotationIntervalCount;
  unsigned long averageRotationPeriodMicros;
  float hallRPM;
  unsigned long liveStepIntervalMicros;
  bool hallSignalValid;

  AppState()
    : server(80),
      patternLength(0),
      currentPatternIndex(0),
      lastShiftMicros(0),
      fixedShiftIntervalMicros(0),
      lastReceivedHex(""),
      timerMode(false),
      timerSpritesReady(false),
      timerText(""),
      countdownHours(0),
      countdownMinutes(0),
      countdownSeconds(0),
      lastCountdownUpdateMillis(0),
      lastHallEdgeMicros(0),
      latestRotationIntervalMicros(0),
      newRotationCaptured(false),
      rotationIntervalIndex(0),
      rotationIntervalCount(0),
      averageRotationPeriodMicros(0),
      hallRPM(0.0f),
      liveStepIntervalMicros(0),
      hallSignalValid(false) {
    for (int i = 0; i < HALL_RPM_AVERAGE_SAMPLES; i++) {
      rotationIntervalsMicros[i] = 0;
    }
  }
};

extern AppState appState;

#endif

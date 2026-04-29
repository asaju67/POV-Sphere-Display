#include "hall_sensor.h"
#include "app_state.h"
#include "config.h"

static void resetHallAveraging() {
  for (int i = 0; i < HALL_RPM_AVERAGE_SAMPLES; i++) {
    appState.rotationIntervalsMicros[i] = 0;
  }

  appState.rotationIntervalIndex = 0;
  appState.rotationIntervalCount = 0;
  appState.averageRotationPeriodMicros = 0;
  appState.hallRPM = 0.0f;
}

void IRAM_ATTR hallISR() {
  unsigned long now = micros();
  unsigned long lastEdge = appState.lastHallEdgeMicros;

  if (lastEdge == 0) {
    appState.lastHallEdgeMicros = now;
    return;
  }

  unsigned long delta = now - lastEdge;
  if (delta < HALL_DEBOUNCE_US) return;

  appState.latestRotationIntervalMicros = delta;
  appState.lastHallEdgeMicros = now;
  appState.newRotationCaptured = true;
}

void initHallSensor() {
#if USE_HALL_SENSOR
  pinMode(HALL_PIN, INPUT);
  attachInterrupt(digitalPinToInterrupt(HALL_PIN), hallISR, FALLING);
#endif
}

void updateHallDerivedTiming() {
#if USE_HALL_SENSOR
  unsigned long now = micros();

  if (appState.newRotationCaptured) {
    noInterrupts();
    unsigned long rotationUs = appState.latestRotationIntervalMicros * HALL_PULSES_PER_REV;
    appState.newRotationCaptured = false;
    interrupts();

    if (rotationUs > 0) {
      appState.rotationIntervalsMicros[appState.rotationIntervalIndex] = rotationUs;
      appState.rotationIntervalIndex = (appState.rotationIntervalIndex + 1) % HALL_RPM_AVERAGE_SAMPLES;
      if (appState.rotationIntervalCount < HALL_RPM_AVERAGE_SAMPLES) {
        appState.rotationIntervalCount++;
      }

      if (appState.rotationIntervalCount == HALL_RPM_AVERAGE_SAMPLES) {
        unsigned long totalRotationUs = 0;
        for (int i = 0; i < HALL_RPM_AVERAGE_SAMPLES; i++) {
          totalRotationUs += appState.rotationIntervalsMicros[i];
        }

        appState.averageRotationPeriodMicros = totalRotationUs / HALL_RPM_AVERAGE_SAMPLES;
        appState.hallRPM = 60000000.0f / appState.averageRotationPeriodMicros;
        appState.hallSignalValid = true;

        if (appState.patternLength > 0) {
          appState.liveStepIntervalMicros = appState.averageRotationPeriodMicros / appState.patternLength;
          if (appState.liveStepIntervalMicros == 0) appState.liveStepIntervalMicros = 1;
        }
      }
    }
  }

  if (appState.lastHallEdgeMicros != 0 && (now - appState.lastHallEdgeMicros) > HALL_TIMEOUT_US) {
    noInterrupts();
    appState.lastHallEdgeMicros = 0;
    appState.newRotationCaptured = false;
    interrupts();

    resetHallAveraging();
    appState.hallSignalValid = false;
  }
#else
  appState.hallSignalValid = false;
  appState.hallRPM = 0.0f;
  appState.liveStepIntervalMicros = appState.fixedShiftIntervalMicros;
#endif
}

void printRawHallSensorValue() {
#if USE_HALL_SENSOR
  static unsigned long lastPrintMillis = 0;
  unsigned long now = millis();

  if (now - lastPrintMillis < HALL_RAW_PRINT_MS) return;
  lastPrintMillis = now;

  Serial.print("Hall raw digital=");
  Serial.println(digitalRead(HALL_PIN));
#endif
}

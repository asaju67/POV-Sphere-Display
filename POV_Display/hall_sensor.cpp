#include "hall_sensor.h"
#include "app_state.h"
#include "config.h"

void IRAM_ATTR hallISR() {
  unsigned long now = micros();
  unsigned long delta = now - appState.lastHallEdgeMicros;

  if (delta < HALL_DEBOUNCE_US) return;

  appState.latestPulseIntervalMicros = delta;
  appState.lastHallEdgeMicros = now;
  appState.newPulseCaptured = true;
}

void initHallSensor() {
#if USE_HALL_SENSOR
  pinMode(HALL_PIN, INPUT);
  attachInterrupt(digitalPinToInterrupt(HALL_PIN), hallISR, RISING);
#endif
}

void updateHallDerivedTiming() {
#if USE_HALL_SENSOR
  unsigned long now = micros();

  if (appState.newPulseCaptured) {
    noInterrupts();
    unsigned long pulseUs = appState.latestPulseIntervalMicros;
    appState.newPulseCaptured = false;
    interrupts();

    if (pulseUs > 0) {
      float revPeriodUs = pulseUs * HALL_PULSES_PER_REV;

      if (appState.smoothedRevPeriodMicros <= 0.0f) {
        appState.smoothedRevPeriodMicros = revPeriodUs;
      } else {
        appState.smoothedRevPeriodMicros =
          (RPM_SMOOTHING_ALPHA * revPeriodUs) +
          ((1.0f - RPM_SMOOTHING_ALPHA) * appState.smoothedRevPeriodMicros);
      }

      appState.smoothedRPM = 60000000.0f / appState.smoothedRevPeriodMicros;
      appState.hallSignalValid = true;

      if (appState.patternLength > 0) {
        appState.liveStepIntervalMicros = (unsigned long)(appState.smoothedRevPeriodMicros / appState.patternLength);
        if (appState.liveStepIntervalMicros == 0) appState.liveStepIntervalMicros = 1;
      }
    }
  }

  if ((now - appState.lastHallEdgeMicros) > HALL_TIMEOUT_US) {
    appState.hallSignalValid = false;
    appState.smoothedRPM = 0.0f;
  }
#else
  appState.hallSignalValid = false;
  appState.smoothedRPM = 0.0f;
  appState.liveStepIntervalMicros = appState.fixedShiftIntervalMicros;
#endif
}

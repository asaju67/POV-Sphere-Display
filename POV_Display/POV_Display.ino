#include <WiFi.h>
#include <WebServer.h>
#include <SPI.h>
#include <SD.h>
#include <stdint.h>

#include "app_state.h"
#include "config.h"
#include "hall_sensor.h"
#include "led_output.h"
#include "patterns.h"
#include "web_ui.h"
#include "sprite_loader.h"
#include "timer_mode.h"

void setup() {
  Serial.begin(115200);
  delay(1000);

  initLedPins();

  appState.fixedShiftIntervalMicros = (unsigned long)(1000000.0f / LATCH_RATE_HZ);

  initHallSensor();

  loadDefaultPattern();
  rebuildLastReceivedHexString();

  Serial.println();
  Serial.println("Starting ESP32 in Access Point mode...");

#if USE_HALL_SENSOR
  Serial.println("Latch mode: HALL SENSOR");
#else
  Serial.println("Latch mode: FIXED RATE");
#endif

  WiFi.mode(WIFI_AP);
  bool apStarted = WiFi.softAP(AP_SSID, AP_PASSWORD);

  if (!apStarted) {
    Serial.println("Failed to start Access Point.");
    while (true) delay(1000);
  }

  Serial.println("Access Point started.");
  Serial.print("SSID: ");
  Serial.println(AP_SSID);
  Serial.print("Password: ");
  Serial.println(AP_PASSWORD);
  Serial.print("Open in browser: http://");
  Serial.println(WiFi.softAPIP());

  setupWebServer();

  Serial.println("HTTP server started.");

  if (initTimerMode()) {
    Serial.println("Timer mode initialized and sprite file available.");
  } else {
    Serial.println("Timer mode unavailable: SD card or sprite loading failed.");
  }
}

void loop() {
  appState.server.handleClient();
  updateHallDerivedTiming();
  printRawHallSensorValue();
  updateTimerCountdown();

  if (appState.patternLength <= 0) return;

  unsigned long now = micros();
  unsigned long activeIntervalUs = appState.fixedShiftIntervalMicros;

#if USE_HALL_SENSOR
  if (appState.hallSignalValid && appState.liveStepIntervalMicros > 0) {
    activeIntervalUs = appState.liveStepIntervalMicros;
  } else {
    activeIntervalUs = appState.fixedShiftIntervalMicros; // safe fallback
  }
#endif

  if (now - appState.lastShiftMicros >= activeIntervalUs) {
    appState.lastShiftMicros = now;

    shiftOut64(appState.patternBuffer[appState.currentPatternIndex]);

    appState.currentPatternIndex++;
    if (appState.currentPatternIndex >= appState.patternLength) {
      appState.currentPatternIndex = 0;
    }
  }
}

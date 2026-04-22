#include "timer_mode.h"
#include "app_state.h"
#include "patterns.h"
#include "sprite_loader.h"

static bool parseTimeString(const char* text, int& outHours, int& outMinutes, int& outSeconds) {
  if (!text) return false;

  int h = 0;
  int m = 0;
  int s = 0;

  int fields = sscanf(text, "%d:%d:%d", &h, &m, &s);
  if (fields != 3) return false;
  if (h < 0 || m < 0 || s < 0) return false;
  if (m >= 60 || s >= 60) return false;

  outHours = h;
  outMinutes = m;
  outSeconds = s;
  return true;
}

bool initTimerMode() {
  appState.timerSpritesReady = false;

  if (!initSpriteSd()) {
    return false;
  }

  SpriteFile36x64 spriteFile;
  if (!loadSpriteFile(spriteFile)) {
    return false;
  }

  appState.timerSpritesReady = true;
  return true;
}

bool setTimerModeFromTimeString(const char* timeText) {
  if (!timeText) return false;

  int hours = 0;
  int minutes = 0;
  int seconds = 0;
  if (!parseTimeString(timeText, hours, minutes, seconds)) {
    return false;
  }

  const SpriteFile36x64* spriteFile = getLoadedSpriteFile();
  if (!spriteFile) {
    if (!initTimerMode()) {
      return false;
    }
    spriteFile = getLoadedSpriteFile();
    if (!spriteFile) {
      return false;
    }
  }

  char normalized[9];
  snprintf(normalized, sizeof(normalized), "%02d:%02d:%02d", hours, minutes, seconds);

  int loadedPatterns = 0;
  if (!buildPatternsFromText(*spriteFile, normalized, appState.patternBuffer, loadedPatterns, MAX_PATTERNS)) {
    return false;
  }

  appState.patternLength = loadedPatterns;
  appState.currentPatternIndex = 0;
  appState.timerMode = true;
  appState.timerText = String(normalized);
  appState.countdownHours = hours;
  appState.countdownMinutes = minutes;
  appState.countdownSeconds = seconds;
  appState.lastCountdownUpdateMillis = millis();
  rebuildLastReceivedHexString();
  return true;
}

bool setHexModeFromString(const String& hexSequence) {
  if (!parseHexSequence(hexSequence)) {
    return false;
  }

  appState.timerMode = false;
  appState.timerText = "";
  return true;
}

bool isTimerModeEnabled() {
  return appState.timerMode;
}

const String& getTimerModeText() {
  return appState.timerText;
}

void updateTimerCountdown() {
  if (!appState.timerMode) return;

  unsigned long now = millis();
  if (now - appState.lastCountdownUpdateMillis < 1000UL) return; // Update every second

  appState.lastCountdownUpdateMillis = now;

  // Decrement the countdown
  if (appState.countdownSeconds > 0) {
    appState.countdownSeconds--;
  } else if (appState.countdownMinutes > 0) {
    appState.countdownMinutes--;
    appState.countdownSeconds = 59;
  } else if (appState.countdownHours > 0) {
    appState.countdownHours--;
    appState.countdownMinutes = 59;
    appState.countdownSeconds = 59;
  } else {
    // Timer reached zero, stop countdown
    return;
  }

  // Update the display
  const SpriteFile36x64* spriteFile = getLoadedSpriteFile();
  if (!spriteFile) return;

  char timeText[9];
  snprintf(timeText, sizeof(timeText), "%02d:%02d:%02d", 
           appState.countdownHours, appState.countdownMinutes, appState.countdownSeconds);

  int loadedPatterns = 0;
  if (buildPatternsFromText(*spriteFile, timeText, appState.patternBuffer, loadedPatterns, MAX_PATTERNS)) {
    appState.patternLength = loadedPatterns;
    appState.currentPatternIndex = 0;
    appState.timerText = String(timeText);
    rebuildLastReceivedHexString();
  }
}

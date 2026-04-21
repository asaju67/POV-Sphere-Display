#include "patterns.h"
#include "app_state.h"
#include "config.h"

String escapeJson(const String& input) {
  String out = "";
  for (size_t i = 0; i < input.length(); i++) {
    char c = input[i];
    if (c == '\\') out += "\\\\";
    else if (c == '\"') out += "\\\"";
    else if (c == '\n') out += "\\n";
    else if (c == '\r') {}
    else out += c;
  }
  return out;
}

String uint64ToHexString(uint64_t value) {
  char buf[19];
  snprintf(buf, sizeof(buf), "0x%016llX", (unsigned long long)value);
  return String(buf);
}

bool parseHex64Token(String token, uint64_t& outValue) {
  token.trim();
  if (token.length() == 0) return false;

  if (token.startsWith("0x") || token.startsWith("0X")) {
    token = token.substring(2);
  }

  if (token.length() == 0 || token.length() > 16) return false;

  uint64_t value = 0;
  for (size_t i = 0; i < token.length(); i++) {
    char c = token[i];
    uint8_t nibble = 0;

    if (c >= '0' && c <= '9') nibble = c - '0';
    else if (c >= 'a' && c <= 'f') nibble = 10 + (c - 'a');
    else if (c >= 'A' && c <= 'F') nibble = 10 + (c - 'A');
    else return false;

    value = (value << 4) | nibble;
  }

  outValue = value;
  return true;
}

void rebuildLastReceivedHexString() {
  appState.lastReceivedHex = "";
  for (int i = 0; i < appState.patternLength; i++) {
    appState.lastReceivedHex += uint64ToHexString(appState.patternBuffer[i]);
    if (i < appState.patternLength - 1) appState.lastReceivedHex += ",";
  }
}

void loadDefaultPattern() {
  appState.patternLength = MAX_PATTERNS;

  for (int col = 0; col < MAX_PATTERNS; col++) {
    int pos = col % 64;

    uint64_t value = 0ULL;
    value |= (1ULL << (63 - pos));

    if (pos < 63) {
      value |= (1ULL << (62 - pos));
    } else {
      value |= (1ULL << 0);
    }

    appState.patternBuffer[col] = value;
  }

  appState.currentPatternIndex = 0;
}

bool parseHexSequence(const String& input) {
  int tempCount = 0;
  uint64_t tempBuffer[MAX_PATTERNS];
  String token = "";

  for (size_t i = 0; i <= input.length(); i++) {
    char c = (i < input.length()) ? input[i] : ',';

    bool isSeparator =
      (c == ',') || (c == ' ') || (c == '\n') || (c == '\r') ||
      (c == '\t') || (c == ';');

    if (!isSeparator) {
      token += c;
    } else if (token.length() > 0) {
      if (tempCount >= MAX_PATTERNS) return false;

      uint64_t value = 0;
      if (!parseHex64Token(token, value)) return false;

      tempBuffer[tempCount++] = value;
      token = "";
    }
  }

  if (tempCount == 0) return false;

  appState.patternLength = tempCount;
  for (int i = 0; i < appState.patternLength; i++) {
    appState.patternBuffer[i] = tempBuffer[i];
  }

  appState.currentPatternIndex = 0;
  rebuildLastReceivedHexString();
  return true;
}

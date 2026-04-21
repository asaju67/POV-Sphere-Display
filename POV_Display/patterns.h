#ifndef PATTERNS_H
#define PATTERNS_H

#include <Arduino.h>
#include <stdint.h>

String escapeJson(const String& input);
String uint64ToHexString(uint64_t value);
bool parseHex64Token(String token, uint64_t& outValue);

void loadDefaultPattern();
void rebuildLastReceivedHexString();
bool parseHexSequence(const String& input);

#endif

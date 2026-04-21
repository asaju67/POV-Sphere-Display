#include "led_output.h"
#include "config.h"

void initLedPins() {
  pinMode(DATA_PIN, OUTPUT);
  pinMode(CLOCK_PIN, OUTPUT);
  pinMode(LATCH_PIN, OUTPUT);
  pinMode(OE_PIN, OUTPUT);

  digitalWrite(DATA_PIN, LOW);
  digitalWrite(CLOCK_PIN, LOW);
  digitalWrite(LATCH_PIN, HIGH);
  digitalWrite(OE_PIN, LOW);   // always enabled = full brightness
}

void shiftOut64(uint64_t value) {
  uint64_t outValue = INVERT_OUTPUT_BITS ? ~value : value;

  digitalWrite(LATCH_PIN, LOW);

  // MSB byte first
  for (int byteIndex = 7; byteIndex >= 0; byteIndex--) {
    uint8_t thisByte = (outValue >> (byteIndex * 8)) & 0xFF;
    shiftOut(DATA_PIN, CLOCK_PIN, MSBFIRST, thisByte);
  }

  digitalWrite(LATCH_PIN, HIGH);
}

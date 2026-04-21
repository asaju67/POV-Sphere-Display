#include <Arduino.h>

const int HALL_PIN = 34;

void setup() {
  Serial.begin(115200);
  pinMode(HALL_PIN, INPUT);   // GPIO34 is input-only
}

void loop() {
  int state = digitalRead(HALL_PIN);

  Serial.print("Hall state: ");
  Serial.println(state);

  delay(100);
}

#ifndef HALL_SENSOR_H
#define HALL_SENSOR_H

#include <Arduino.h>

void initHallSensor();
void IRAM_ATTR hallISR();
void updateHallDerivedTiming();

#endif

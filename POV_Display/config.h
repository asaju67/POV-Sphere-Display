#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

// =====================================================
// USER SETTINGS
// =====================================================
static constexpr const char* AP_SSID     = "LED_Sphere_Setup";
static constexpr const char* AP_PASSWORD = "sphere123";

// GPIOs
static constexpr int DATA_PIN  = 13;
static constexpr int CLOCK_PIN = 12;
static constexpr int LATCH_PIN = 14;
static constexpr int HALL_PIN  = 34;   // input only
static constexpr int OE_PIN    = 21;   // ~OE/DM2 via PWM

// Each pattern is one full 64-bit latch sequence
static constexpr int MAX_PATTERNS = 360;

// -------------------------------
// MODE SELECT
// 0 = fixed latch rate
// 1 = hall-sensor-controlled rate
// -------------------------------
#define USE_HALL_SENSOR 1

// Fixed mode latch/update rate in Hz
static constexpr float LATCH_RATE_HZ = 30.0f;

// Hall sensor settings
static constexpr int HALL_PULSES_PER_REV = 1;              // change if needed
static constexpr unsigned long HALL_DEBOUNCE_US = 1500;    // ignore very fast glitches
static constexpr int HALL_RPM_AVERAGE_SAMPLES = 5;         // average this many rotation intervals
static constexpr unsigned long HALL_TIMEOUT_US = 1000000;  // if no pulse in 1 sec, treat as stopped
static constexpr unsigned long HALL_RAW_PRINT_MS = 250;    // serial print interval for raw hall pin value

// Set true if your LED driver logic is active-low
static constexpr bool INVERT_OUTPUT_BITS = true;

#endif

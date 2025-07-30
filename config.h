#pragma once

// Pin definitions
const int RELAY_PIN = 19;

// Power configuration
const float POWER_THRESHOLD = 500.0;
const float CHARGER_CONSUMPTION = 500.0;

// Timing configuration
const unsigned long HYSTERESIS_TIME = 5UL * 60UL * 1000UL;  // 5 minutes
const unsigned long MEASUREMENT_INTERVAL = 10000UL;         // 10 seconds

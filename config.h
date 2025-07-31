#pragma once

// Pin definitions
const int RELAY_PIN = 19;

// Power configuration
const float POWER_THRESHOLD = 1000.0; // 1000W threshold for turning the charger on/off

// Timing configuration
const unsigned long HYSTERESIS_TIME = 5UL * 60UL * 1000UL;  // 5 minutes
const unsigned long MEASUREMENT_INTERVAL = 10000UL;         // 10 seconds

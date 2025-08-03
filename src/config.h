#pragma once

// Pin definitions
const int RELAY_PIN = 19;

// Power configuration
const float POWER_THRESHOLD = 1000.0; // 1000W threshold for turning the charger on/off

// Timing configuration
const unsigned long HYSTERESIS_TIME = 2UL * 60UL * 1000UL;  // 2 minutes hysteresis time
const unsigned long MEASUREMENT_INTERVAL = 10000UL;         // 10 seconds interval

// LCD configuration
const int LCD_ADDRESS = 0x27; // I2C address of the LCD
const int LCD_COLS = 16;      // Number of columns on the LCD
const int LCD_ROWS = 2;       // Number of rows on the LCD

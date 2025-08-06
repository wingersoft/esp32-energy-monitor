#pragma once

// =================================================================
// Pin Definitions
// =================================================================
const int RELAY_PIN = 19;   // Pin connected to the relay that controls the charger
const int DIP_PIN_1 = 25;   // DIP switch for setting hysteresis time (120s or 240s)
const int DIP_PIN_2 = 26;   // DIP switch for setting power threshold (500W, 1000W, 1500W, 2000W)
const int DIP_PIN_3 = 23;   // DIP switch for setting power threshold (500W, 1000W, 1500W, 2000W)

// =================================================================
// DIP Switch Configuration
// =================================================================
// The HYSTERESIS_TIME and POWER_THRESHOLD can be configured dynamically
// using a 3-position DIP switch. This allows for easy adjustment
// without needing to re-flash the firmware.
//
// A '0' corresponds to the OFF position, and a '1' corresponds to the ON position.
//
// | DIP 1 | DIP 2 | DIP 3 | Hysteresis Time | Power Threshold |
// |       |       |       |                 |                 |
// |   0   |   0   |   0   |      120s       |       500W      |
// |   0   |   0   |   1   |      120s       |      1000W      |
// |   0   |   1   |   0   |      120s       |      1500W      |
// |   0   |   1   |   1   |      120s       |      2000W      |
// |   1   |   0   |   0   |      240s       |       500W      |
// |   1   |   0   |   1   |      240s       |      1000W      |
// |   1   |   1   |   0   |      240s       |      1500W      |
// |   1   |   1   |   1   |      240s       |      2000W      |
//

// =================================================================
// Power Configuration
// =================================================================
// Default power threshold (in watts). This value is updated dynamically
// by the DIP switch settings at startup.
float POWER_THRESHOLD = 1000.0;

// =================================================================
// Timing Configuration
// =================================================================
// Default hysteresis time (in milliseconds). This value is updated
// dynamically by the DIP switch settings at startup.
unsigned long HYSTERESIS_TIME = 2UL * 60UL * 1000UL;

// Interval (in milliseconds) at which power data is fetched from the API.
const unsigned long MEASUREMENT_INTERVAL = 10000UL; // 10 seconds

// =================================================================
// LCD Configuration
// =================================================================
const int LCD_ADDRESS = 0x27; // I2C address of the LCD
const int LCD_COLS = 16;      // Number of columns on the LCD
const int LCD_ROWS = 2;       // Number of rows on the LCD

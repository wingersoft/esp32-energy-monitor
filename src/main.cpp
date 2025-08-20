// 
// This code is an ESP32-based controller that manages
// battery charging based on solar power monitoring.
//
// Core Functionality:
//   Monitors power consumption via HTTP requests to an API endpoint
//   Controls charging signal based on power thresholds
//   Implements a hysteresis timer to prevent rapid switching
//
// Main Components:
//   WiFi connectivity with automatic reconnection
//   HTTP client for fetching power data
//   JSON parsing for API responses
//   Digital output control for charging signal
//

#include "config.h"            // Project configuration constants
#include "secrets.h"           // WiFi credentials and API configuration
#include <ArduinoJson.h>       // JSON parsing for API responses
#include <HTTPClient.h>        // HTTP client for API requests
#include <LiquidCrystal_I2C.h> // LCD display control
#include <WiFi.h>              // WiFi connectivity
#include <Wire.h>              // I2C communication for LCD

float POWER_THRESHOLD = 1000.0;      // Default power threshold in watts
unsigned long HYSTERESIS_TIME = 120000UL; // Default hysteresis time in milliseconds (120s)

// Initialize 16x2 I2C LCD display for user interface
LiquidCrystal_I2C lcd(LCD_ADDRESS, LCD_COLS, LCD_ROWS);

// Global state variables
bool chargerOn = false;                // Tracks the current state of the charger (on/off)
unsigned long lastSwitchTime = 0;      // Timestamp of the last time the charger was switched on or off
unsigned long lastMeasurementTime = 0; // Timestamp of the last power measurement
unsigned long powerHighStartTime = 0;  // Timestamp when power first exceeded the threshold
unsigned long powerLowStartTime = 0;   // Timestamp when power first dropped below the threshold

// LCD display buffers to track what's currently shown
char lcdLine0[LCD_COLS + 1] = "";
char lcdLine1[LCD_COLS + 1] = "";

//
// Handles WiFi events like connection and disconnection.
//
void WiFiEvent(WiFiEvent_t event)
{
    Serial.printf("[WiFi-event] event: %d\n", event);
    switch (event)
    {
    // When the ESP32 obtains an IP address, print it to the serial monitor.
    case SYSTEM_EVENT_STA_GOT_IP:
        Serial.print("WiFi connected. IP address: ");
        Serial.println(WiFi.localIP());
        break;
    // If the ESP32 loses connection, it will attempt to reconnect.
    case SYSTEM_EVENT_STA_DISCONNECTED:
        Serial.println("WiFi lost connection. Reconnecting...");
        WiFi.begin(ssid, password);
        break;
    default:
        break;
    }
}

//
// Fetches the current solar power generation from the API endpoint.
//
bool getSolarPower(int &power)
{
    // Check for WiFi connection before making an HTTP request.
    if (WiFi.status() != WL_CONNECTED)
    {
        Serial.println("WiFi not connected. Skipping measurement.");
        return false;
    }

    HTTPClient http;
    http.begin(apiUrl);                // Initialize HTTP client with the API URL.
    int httpResponseCode = http.GET(); // Send a GET request.

    // Check if the request was successful.
    if (httpResponseCode == 200)
    {
        JsonDocument doc;
        // Parse the JSON response from the HTTP stream.
        DeserializationError error = deserializeJson(doc, http.getStream());
        if (error)
        {
            Serial.print("deserializeJson() failed: ");
            Serial.println(error.c_str());
            http.end();
            return false;
        }
        // Extract the power value and negate it to represent solar generation.
        power = -doc["active_power_w"].as<int>();
    }
    else
    {
        Serial.print("HTTP response error: ");
        Serial.println(httpResponseCode);
        http.end();
        return false;
    }

    http.end(); // Free resources.
    return true;
}

void turnChargerOn(unsigned long currentTime)
{
    if (powerHighStartTime == 0)
    {
        powerHighStartTime = currentTime; // Start the timer
    }
    if (currentTime - powerHighStartTime >= HYSTERESIS_TIME)
    {
        digitalWrite(RELAY_PIN, HIGH); // Turn on the relay.
        chargerOn = true;
        lastSwitchTime = currentTime;
        powerLowStartTime = 0; // Reset the low power timer
        Serial.println("Charger ON");
    }
}

void turnChargerOff(unsigned long currentTime)
{
    if (powerLowStartTime == 0)
    {
        powerLowStartTime = currentTime; // Start the timer
    }
    if (currentTime - powerLowStartTime >= HYSTERESIS_TIME)
    {
        digitalWrite(RELAY_PIN, LOW); // Turn off the relay.
        chargerOn = false;
        lastSwitchTime = currentTime;
        powerHighStartTime = 0; // Reset the high power timer
        Serial.println("Charger OFF");
    }
}

//
// Controls the charger relay based on the available solar power.
//
void controlCharger(int solarPower)
{
    unsigned long currentTime = millis(); // Get the current time.

    if (!chargerOn)
    {
        // Turn the charger on if there is enough surplus power and the condition has been met for the hysteresis time.
        if (solarPower >= POWER_THRESHOLD)
        {
            turnChargerOn(currentTime);
        }
        else
        {
            powerHighStartTime = 0; // Reset the high power timer
        }
    }
    else
    {
        // Turn the charger off if the power drops below the threshold and the condition has been met for the hysteresis time.
        if (solarPower < POWER_THRESHOLD)
        {
            turnChargerOff(currentTime);
        }
        else
        {
            powerLowStartTime = 0; // Reset the low power timer
        }
    }
}

//
// Updates a specific line on the LCD if the content has changed.
//
void updateLCDLine(int line, const char* text)
{
    if (line == 0)
    {
        if (strcmp(lcdLine0, text) != 0)
        {
            strncpy(lcdLine0, text, LCD_COLS);
            lcdLine0[LCD_COLS] = '\0'; // Ensure null termination
            lcd.setCursor(0, 0);
            lcd.print(text);
            // Clear the rest of the line if the new text is shorter
            int len = strlen(text);
            for (int i = len; i < LCD_COLS; i++)
            {
                lcd.print(' ');
            }
        }
    }
    else if (line == 1)
    {
        if (strcmp(lcdLine1, text) != 0)
        {
            strncpy(lcdLine1, text, LCD_COLS);
            lcdLine1[LCD_COLS] = '\0'; // Ensure null termination
            lcd.setCursor(0, 1);
            lcd.print(text);
            // Clear the rest of the line if the new text is shorter
            int len = strlen(text);
            for (int i = len; i < LCD_COLS; i++)
            {
                lcd.print(' ');
            }
        }
    }
}

//
// Prints the current status of the system to the serial monitor and LCD.
//
void printStatus(int solarPower)
{
    Serial.print("Solar panel power: ");
    Serial.print(solarPower);
    Serial.print("W, Charger: ");
    Serial.println(chargerOn ? "ON" : "OFF");

    // Row 0: Power and Charger status
    char line0Buffer[LCD_COLS + 1];
    snprintf(line0Buffer, sizeof(line0Buffer), "P:%dW     C:%s", solarPower, chargerOn ? "On" : "Off");
    updateLCDLine(0, line0Buffer);

    // Row 1: Display countdown or Hysteresis and Threshold
    bool showCountdown = chargerOn && solarPower < POWER_THRESHOLD && powerLowStartTime > 0;
    unsigned long elapsedTime = millis() - powerLowStartTime;

    if (showCountdown && elapsedTime < HYSTERESIS_TIME)
    {
        unsigned long remainingTime = (HYSTERESIS_TIME - elapsedTime) / 1000;
        char line1Buffer[LCD_COLS + 1];
        snprintf(line1Buffer, sizeof(line1Buffer), "Off in: %lus", remainingTime);
        updateLCDLine(1, line1Buffer);
    }
    else
    {
        char line1Buffer[LCD_COLS + 1];
        snprintf(line1Buffer, sizeof(line1Buffer), "H:%lus   T:%dW", HYSTERESIS_TIME / 1000, (int)POWER_THRESHOLD);
        updateLCDLine(1, line1Buffer);
    }
}

//
// Initializes the hardware and software components.
//
void setup()
{
    pinMode(RELAY_PIN, OUTPUT); // Set the relay pin as an output.
    pinMode(DIP_PIN_1, INPUT_PULLUP);
    pinMode(DIP_PIN_2, INPUT_PULLUP);
    pinMode(DIP_PIN_3, INPUT_PULLUP);
    Serial.begin(115200); // Start serial communication for debugging.

    // Read DIP switches using bitwise operations for efficiency
    int dipValue = (digitalRead(DIP_PIN_1) == HIGH ? 4 : 0) |
                   (digitalRead(DIP_PIN_2) == HIGH ? 2 : 0) |
                   (digitalRead(DIP_PIN_3) == HIGH ? 1 : 0);

    // Set HYSTERESIS_TIME and POWER_THRESHOLD based on DIP switch settings
    switch (dipValue)
    {
    case 0: // 0 0 0
        HYSTERESIS_TIME = 120UL * 1000UL;
        POWER_THRESHOLD = 500.0;
        break;
    case 1: // 0 0 1
        HYSTERESIS_TIME = 120UL * 1000UL;
        POWER_THRESHOLD = 1000.0;
        break;
    case 2: // 0 1 0
        HYSTERESIS_TIME = 120UL * 1000UL;
        POWER_THRESHOLD = 1500.0;
        break;
    case 3: // 0 1 1
        HYSTERESIS_TIME = 120UL * 1000UL;
        POWER_THRESHOLD = 2000.0;
        break;
    case 4: // 1 0 0
        HYSTERESIS_TIME = 240UL * 1000UL;
        POWER_THRESHOLD = 500.0;
        break;
    case 5: // 1 0 1
        HYSTERESIS_TIME = 240UL * 1000UL;
        POWER_THRESHOLD = 1000.0;
        break;
    case 6: // 1 1 0
        HYSTERESIS_TIME = 240UL * 1000UL;
        POWER_THRESHOLD = 1500.0;
        break;
    case 7: // 1 1 1
        HYSTERESIS_TIME = 240UL * 1000UL;
        POWER_THRESHOLD = 2000.0;
        break;
    }

    // Initialize the LCD
    Wire.begin();
    lcd.init();
    lcd.backlight();
    lcd.print("Starting...");

    WiFi.onEvent(WiFiEvent);    // Register the WiFi event handler.
    WiFi.mode(WIFI_STA);        // Set the ESP32 to station mode.
    WiFi.begin(ssid, password); // Connect to the WiFi network.

    Serial.println("Connecting to WiFi...");
    int retries = 60;
    // Wait for the WiFi connection to be established.
    while (WiFi.status() != WL_CONNECTED && retries > 0)
    {
        Serial.print(".");
        delay(1000);
        retries--;
    }

    // If the connection fails after multiple retries, restart the ESP32.
    if (WiFi.status() != WL_CONNECTED)
    {
        Serial.println("\nFailed to connect to WiFi. Restarting...");
        ESP.restart();
    }

    Serial.println("\nWiFi connected.");
    
    // Clear LCD buffers after initial display
    memset(lcdLine0, ' ', LCD_COLS);
    memset(lcdLine1, ' ', LCD_COLS);
    lcdLine0[LCD_COLS] = '\0';
    lcdLine1[LCD_COLS] = '\0';
}

//
// The main loop of the program, executed repeatedly.
//
void loop()
{
    unsigned long currentTime = millis();
    // Periodically measure the solar power and control the charger.
    if (currentTime - lastMeasurementTime >= MEASUREMENT_INTERVAL)
    {
        lastMeasurementTime = currentTime;

        int solarPower = 0;
        // If the power is successfully fetched, control the charger and print the status.
        if (getSolarPower(solarPower))
        {
            controlCharger(solarPower);
            printStatus(solarPower);
        }
    }
    
    // Handle WiFi reconnection in loop as well
    if (WiFi.status() != WL_CONNECTED)
    {
        Serial.println("WiFi disconnected. Attempting to reconnect...");
        WiFi.reconnect();
        delay(5000); // Wait a bit before checking again
    }
}

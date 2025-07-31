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

#include "../config.h"
#include "../secrets.h"
#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <WiFi.h>

// Global state variables
bool chargerOn = false;                // Tracks the current state of the charger (on/off)
unsigned long lastSwitchTime = 0;      // Timestamp of the last time the charger was switched on or off
unsigned long lastMeasurementTime = 0; // Timestamp of the last power measurement
unsigned long powerHighStartTime = 0;  // Timestamp when power first exceeded the threshold
unsigned long powerLowStartTime = 0;   // Timestamp when power first dropped below the threshold

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

//
// Controls the charger relay based on the available solar power.
//
void controlCharger(int solarPower)
{
    unsigned long currentTime = millis(); // Get the current time.

    // Turn the charger on if there is enough surplus power and the condition has been met for the hysteresis time.
    if (!chargerOn && solarPower >= POWER_THRESHOLD)
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
    else
    {
        powerHighStartTime = 0; // Reset the high power timer
    }

    // Turn the charger off if the power drops below the threshold and the condition has been met for the hysteresis time.
    if (chargerOn && solarPower < POWER_THRESHOLD)
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
    else
    {
        powerLowStartTime = 0; // Reset the low power timer
    }
}

//
// Prints the current status of the system to the serial monitor.
//
void printStatus(int solarPower)
{
    Serial.print("Solar panel power: ");
    Serial.print(solarPower);
    Serial.print("W, Charger: ");
    Serial.println(chargerOn ? "ON" : "OFF");
}

//
// Initializes the hardware and software components.
//
void setup()
{
    pinMode(RELAY_PIN, OUTPUT); // Set the relay pin as an output.
    Serial.begin(115200);       // Start serial communication for debugging.

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
}
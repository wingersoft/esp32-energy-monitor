//
// This code is an ESP32-based controller that manages
// battery charging based on solar power monitoring.
//
// Here are the key components:
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
// Key Parameters:
//   CHARGE_SIGNAL_PIN: Pin 3 for charging control
//   THRESHOLDWATTAGE: 200W power threshold
//   THRESHOLDTIME: 60 seconds timing threshold
//   CHECK_INTERVAL: 1 second monitoring interval
//
// Operation Flow:
//   Connects to WiFi using credentials from config file
//   Periodically polls API for power consumption data
//   Activates charging when power exceeds threshold for set duration
//   Deactivates charging when power falls below threshold for set duration
//
// Safety Features:
//   WiFi event handling for connection monitoring
//   Automatic restart if initial WiFi connection fails
//   Error handling for HTTP requests
//   The code is designed for monitoring solar power production and
//   controlling battery charging based on excess power availability.
//

#include <Arduino.h>
#include <ArduinoJson.h>
#include <WiFi.h>
#include <HTTPClient.h>

// Include (hidden) config file to get WiFi credentials and API url

// const char* ssid = "";
// const char* password = "";
// const char *mqtt_server = "";
// const char *apiUrl = "";

#include "../../config.h"

const int THRESHOLDWATTAGE = 200; // Power threshold in watts
const int THRESHOLDTIME = 300000; // Time threshold in milliseconds

const int CHARGE_SIGNAL_PIN = 20; // Pin for charging signal

unsigned long powerAbove = 0;     // Time power is above threshold
unsigned long powerBelow = 0;     // Time power is below threshold
bool isCharging   = false;        // Charging status
bool noconnection = false;        // WiFi connection status

//
// WiFi event handler function for handling WiFi events
//
void WiFiEvent(WiFiEvent_t event)
{
    Serial.printf("[WiFi-event] event: %d  - ", event);
    switch (event)
    {
    case SYSTEM_EVENT_STA_GOT_IP:
        Serial.print("WiFi connected - ");
        Serial.print("IP address: ");
        Serial.println(WiFi.localIP());
        noconnection = false;
        break;
    case SYSTEM_EVENT_STA_DISCONNECTED:
        Serial.println("WiFi lost connection");
        WiFi.disconnect();
        WiFi.begin(ssid, password);
        noconnection = true;
        break;
    case SYSTEM_EVENT_STA_START:
        Serial.println("ESP32 station start");
        break;
    case SYSTEM_EVENT_STA_CONNECTED:
        Serial.println("ESP32 station connected to AP");
        break;
    }
}

//
// Http request function for getting data from API
//
int getHTTP()
{
    int active_power = 0;
    // Make a HTTP GET request
    HTTPClient http;
    http.begin(apiUrl);
    // Send the GET request
    int httpResponseCode = http.GET();
    // Check for successful response
    if (httpResponseCode == 200)
    {
        // Get the response payload
        String payload = http.getString();
        // Parse JSON
        JsonDocument doc;
        // Deserialize JSON
        deserializeJson(doc, payload);
        // Access the "active_power_w" value
        String active = doc["active_power_w"];
        // Convert to int and invert sign
        active_power = -active.toInt();
    }
    else
    {
        Serial.print("HTTP response error: ");
        Serial.println(httpResponseCode);
    }
    // Close the connection
    http.end();
    return active_power;
}

//
// Setup function
//
void setup()
{
    // Set pin mode for charging signal
    pinMode(CHARGE_SIGNAL_PIN, OUTPUT);
    // Init Serial port
    Serial.begin(115200);
    // Init WiFI
    WiFi.enableAP(false);
    // Handle WiFi event
    WiFi.onEvent(WiFiEvent);
    WiFi.mode(WIFI_STA);
    // WiFi connect
    WiFi.begin(ssid, password);
    // wait 60 S before reboot
    int trying = 60;
    while (WiFi.status() != WL_CONNECTED)
    {
        Serial.print(".");
        delay(1000);
        if (trying == 0)
            ESP.restart();
        else
            trying--;
    } // Start serial communication
    Serial.println("");
}

//
// Main loop
//
void loop()
{
    static unsigned long lastCheck = 0;        // Last time we checked the power
    const unsigned long CHECK_INTERVAL = 1000; // Check every 1 second

    if (millis() - lastCheck >= CHECK_INTERVAL)
    {
        lastCheck = millis();

        int power = getHTTP();
        Serial.print("Power: ");
        Serial.print(power);
        Serial.print(" W  ");
        Serial.print(isCharging);
        Serial.println(" ");

        // If power is below THRESHOLDWATTAGE, start the below timer
        if (power >= THRESHOLDWATTAGE)
        {
            // Reset the below timer if power is restored
            powerBelow = 0;

            if (powerAbove == 0)
            {
                powerAbove = millis();
            }

            // If power has been above THRESHOLDWATTAGE for THRESHOLDTIME
            // activate charging signal
            if (millis() - powerAbove >= THRESHOLDTIME && !isCharging)
            {
                digitalWrite(CHARGE_SIGNAL_PIN, HIGH);
                isCharging = true;
            }
        }
        else
        {
            // If power drops below THRESHOLDWATTAGE, start the below timer
            if (powerBelow == 0)
            {
                powerBelow = millis();
            }

            // If power remains below THRESHOLD, deactivate charging signal
            if (millis() - powerBelow >= THRESHOLDTIME && isCharging)
            {
                digitalWrite(CHARGE_SIGNAL_PIN, LOW);
                isCharging = false;
                powerAbove = 0; // Reset the above timer
            }
        }
    }
}

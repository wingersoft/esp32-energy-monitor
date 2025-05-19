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

//#include <Arduino.h>
#include <ArduinoJson.h>
#include <WiFi.h>
#include <HTTPClient.h>

// Include (hidden) config file to get WiFi credentials and API url

// const char* ssid = "";
// const char* password = "";
// const char *mqtt_server = "";
// const char *apiUrl = "";

#include "../../config.h"

// Pin definitions
const int RELAY_PIN = 19;                 // Digital pin for relay

// Constants
const float POWER_THRESHOLD = 500.0;      // 500W threshold
const float CHARGER_CONSUMPTION = 500.0;  // 500W charger consumption
const unsigned long HYSTERESIS_TIME = 5UL * 60UL * 1000UL;  // 5 minutes in milliseconds
const unsigned long MEASUREMENT_INTERVAL = 10000UL;         // 10 seconds interval for measurements

// Variables
bool chargerOn = false;
unsigned long lastSwitchTime = 0;
unsigned long lastMeasurementTime = 0;

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


void setup() {
    // Set pin mode for charging signal
    pinMode(RELAY_PIN, OUTPUT);
    // Init Serial port
    Serial.begin(115200);
    // Init WiFI
    WiFi.enableAP(false);
    // Handle WiFi event
    WiFi.onEvent(WiFiEvent);
    // WiFi mode station
    WiFi.mode(WIFI_STA);
    // WiFi connect
    WiFi.begin(ssid, password);
    // wait 60 seconds before reboot
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

void loop()
{
    unsigned long currentTime = millis();
    // Check if it's time for a new measurement (every 10 seconds)
    if (currentTime - lastMeasurementTime >= MEASUREMENT_INTERVAL)
    {
        // Read power from solar panels
        int solarPower = getHTTP();
        // DEBUG CHARGER ON SIMULATION
        if (chargerOn) solarPower = solarPower - CHARGER_CONSUMPTION;
        // DEBUG CHARGER ON SIMULATION
        unsigned long currentTime = millis();

        // Check if we need to turn on the charger
        if (!chargerOn && solarPower >= (POWER_THRESHOLD + CHARGER_CONSUMPTION))
        {
            if (currentTime - lastSwitchTime >= HYSTERESIS_TIME)
            {
                digitalWrite(RELAY_PIN, HIGH);  // Turn charger on
                chargerOn = true;
                lastSwitchTime = currentTime;
                Serial.println("Charger ON");
            }
        }
        // Check if we need to turn off the charger
        else if (chargerOn && solarPower < POWER_THRESHOLD)
        {
            if (currentTime - lastSwitchTime >= HYSTERESIS_TIME)
            {
                digitalWrite(RELAY_PIN, LOW);  // Turn charger off
                chargerOn = false;
                lastSwitchTime = currentTime;
                Serial.println("Charger OFF");
            }
        }

        // Print status for debugging
        Serial.print("Solar panel power: ");
        Serial.print(solarPower);
        Serial.print("W, Charger: ");
        Serial.println(chargerOn ? "ON" : "OFF");

        lastMeasurementTime = currentTime;  // Update time of last measurement
    }
}

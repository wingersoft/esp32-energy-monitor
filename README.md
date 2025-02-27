# ESP32 Energy Monitoring System

This project is an ESP32-based energy monitoring system that retrieves power usage data from an API, processes it, and control a relay based battery charging unit. Designed for use with the Arduino framework, this project integrates WiFi and HTTP functionalities.

## Features
- **WiFi Connectivity**: Connects to a WiFi network with automatic reconnection on disconnect.
- **HTTP Data Retrieval**: Fetches real-time energy data (active power usage) from a specified API.
- **Daily Energy Calculation**: Tracks and calculates daily power import. Switches on/off a relay based on power from a solar panel.
- **Error Handling**: Robust WiFi reconnection logic with status logging via Serial.

## Hardware Requirements
- ESP32 microcontroller (e.g., ESP32 DevKit V1)
- Stable WiFi connection
- (Optional) Serial monitor for debugging

## Software Requirements
- Arduino IDE or compatible environment
- ESP32 board support package
- Libraries:
  - `ArduinoJson` (for parsing API responses)
  - `WiFi` (for WiFi connectivity)
  - `HTTPClient` (for HTTP requests)

Install these libraries via the Arduino Library Manager or PlatformIO.

## Setup Instructions

1. **Clone the Repository**:
   ```bash
   git clone https://github.com/wingersoft/esp32-energy-monitor.git
   cd esp32-energy-monitor
   ```

2. **Configure Credentials**:
   - Create a `config.h` file in the project root with the following:
     ```cpp
     const char* ssid = "your_wifi_ssid";
     const char* password = "your_wifi_password";
     const char* apiUrl = "your_api_endpoint";
     ```
   - Replace the placeholders with your WiFi credentials, MQTT server address, and API URL.

3. **Upload the Code**:
   - Open `main.cpp` in the Arduino IDE.
   - Select your ESP32 board and port under `Tools`.
   - Upload the sketch to your ESP32.

4. **Monitor Output**:
   - Open the Serial Monitor (`Ctrl+Shift+M`) at 115200 baud to view connection status and debug messages.

## Usage
- The ESP32 connects to WiFi and the MQTT broker on startup.
- Every second (configurable via `CHECK_INTERVAL`), it:
  - Fetches energy data from the API.
  - Calculates and controls a relay based on the energy from the solar panels.

## Project Structure
```
esp32-energy-monitor/
├── main.cpp         # Main source file
├── config.h         # Configuration file (not tracked, create locally)
└── README.md        # This file
```

## Troubleshooting
- **WiFi Not Connecting**: Verify `ssid` and `password` in `config.h`. Ensure the ESP32 is within range.
- **API Errors**: Confirm the `apiUrl` is correct and returns JSON data in the expected format.

## Contributing
Feel free to submit issues or pull requests for enhancements, bug fixes, or additional features!

## License
This project is licensed under the MIT License. See [LICENSE](LICENSE) for details.

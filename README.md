# ESP32 Energy Monitoring System

This project is an ESP32-based energy monitoring system that retrieves power usage data from an API, processes it, and controls a relay-based battery charging unit. Designed for use with the Arduino framework, this project integrates WiFi and HTTP functionalities.

## Features

- **WiFi Connectivity**: Connects to a WiFi network with automatic reconnection on disconnect.
- **HTTP Data Retrieval**: Fetches real-time energy data (active power usage) from a specified API.
- **Intelligent Charging Control**: Controls a battery charger with hysteresis logic. The charger is only activated or deactivated after the power threshold has been met for a specified duration, preventing rapid on/off cycles.
- **Robust Error Handling**: Includes resilient error handling for network and API interactions.
- **Centralized Configuration**: All settings are managed in a single `config.h` file.

## Hardware Requirements

- ESP32 microcontroller (e.g., ESP32 DevKit V1)
- Stable WiFi connection
- Relay module for charger control
- (Optional) Serial monitor for debugging

## Software Requirements

- Arduino IDE or a compatible environment (like PlatformIO)
- ESP32 board support package
- **Libraries**:
  - `ArduinoJson` (for parsing API responses)
  - `WiFi` (for WiFi connectivity)
  - `HTTPClient` (for HTTP requests)

Install these libraries via the Arduino Library Manager or PlatformIO.

## Configuration

All settings are managed in the `config.h` and `secrets.h` files.

- **`config.h`**:
  - `RELAY_PIN`: The GPIO pin connected to the relay module.
  - `POWER_THRESHOLD`: The power threshold (in watts) for turning the charger on or off.
  - `HYSTERESIS_TIME`: The duration (in milliseconds) that the power threshold must be met before the charger state is changed.
  - `MEASUREMENT_INTERVAL`: The interval (in milliseconds) at which power data is fetched from the API.

- **`secrets.h`**:
  - `ssid`: Your WiFi network's SSID.
  - `password`: Your WiFi network's password.
  - `apiUrl`: The URL of the energy monitoring API.

## Setup Instructions

1.  **Clone the Repository**:

    ```bash
    git clone https://github.com/wingersoft/esp32-energy-monitor.git
    cd esp32-energy-monitor
    ```

2.  **Create `secrets.h`**:

    - Create a new file named `secrets.h` in the root directory of the project. You can do this by copying the `secrets.h.example` file:
      ```bash
      cp secrets.h.example secrets.h
      ```
    - Open `secrets.h` and update the following values:
      - `ssid` and `password` with your WiFi credentials.
      - `apiUrl` with your API endpoint.

3.  **Configure `config.h` (Optional)**:

    - Open the `config.h` file and adjust the `RELAY_PIN`, `POWER_THRESHOLD`, `HYSTERESIS_TIME`, and `MEASUREMENT_INTERVAL` values as needed.

4.  **Upload the Code**:

    - Open the project in your Arduino IDE or PlatformIO.
    - Select your ESP32 board and port.
    - Upload the sketch to your ESP32.

5.  **Monitor Output**:

    - Open the Serial Monitor at `115200` baud to view connection status, power readings, and debug messages.

## Usage

- On startup, the ESP32 connects to your WiFi network.
- Every 10 seconds (configurable via `MEASUREMENT_INTERVAL`), it fetches power data from the API.
- The system intelligently controls the charger based on the available solar power. The charger is only switched on or off after the power threshold has been met for the duration specified by `HYSTERESIS_TIME`.

## Project Structure

```
esp32-energy-monitor/
├── src/
│   └── main.cpp          # Main source file with setup() and loop()
├── config.h              # Centralized configuration file
├── secrets.h             # WiFi and API credentials
├── secrets.h.example     # Example for secrets.h
├── platformio.ini        # PlatformIO project configuration
└── README.md             # This file
```

## Troubleshooting

-   **WiFi Not Connecting**: Double-check the `ssid` and `password` in `secrets.h`. Ensure your ESP32 is within WiFi range.
-   **API Errors**: Verify that the `apiUrl` is correct and returns JSON data in the expected format. Check the Serial Monitor for HTTP error codes.

## Contributing

Feel free to submit issues or pull requests for enhancements, bug fixes, or new features!

## License

This project is licensed under the MIT License. See the `license.txt` file for details.
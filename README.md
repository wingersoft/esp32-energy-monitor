# ESP32 Energy Monitoring System

This project is an ESP32-based energy monitoring system that retrieves power usage data from a **HomeWizard P1 Meter**, processes it, and controls a relay-based battery charging unit. It also includes an LCD display to show the current solar power and charger status. Designed for use with the Arduino framework, this project integrates WiFi, HTTP, and I2C functionalities.

## Features

- **WiFi Connectivity**: Connects to a WiFi network with automatic reconnection on disconnect.
- **HTTP Data Retrieval**: Fetches real-time energy data from a HomeWizard P1 Meter API.
- **Intelligent Charging Control**: Controls a battery charger with hysteresis logic. The charger is only activated or deactivated after the power threshold has been met for a specified duration, preventing rapid on/off cycles.
- **LCD Display**: Shows the current solar power and charger status on a 16x2 LCD display.
- **Robust Error Handling**: Includes resilient error handling for network and API interactions.
- **Centralized Configuration**: All settings are managed in a single `config.h` file.

## Hardware Requirements

- ESP32 microcontroller (e.g., ESP32 DevKit V1)
- Stable WiFi connection
- Relay module for charger control
- 16x2 I2C LCD display
  - The default I2C pins for the ESP32 are GPIO 21 (SDA) and GPIO 22 (SCL).
- (Optional) Serial monitor for debugging

## Software Requirements

- Arduino IDE or a compatible environment (like PlatformIO)
- ESP32 board support package
- **Libraries**:
  - `ArduinoJson` (for parsing API responses)
  - `WiFi` (for WiFi connectivity)
  - `HTTPClient` (for HTTP requests)
  - `LiquidCrystal_I2C` (for the LCD display)

Install these libraries via the Arduino Library Manager or PlatformIO.

## Configuration

All settings are managed in the `config.h` and `secrets.h` files.

- **`config.h`**:
  - `RELAY_PIN`: The GPIO pin connected to the relay module.
  - `MEASUREMENT_INTERVAL`: The interval (in milliseconds) at which power data is fetched from the API.
  - `LCD_ADDRESS`: The I2C address of the LCD display.
  - `LCD_COLS`: The number of columns on the LCD display.
  - `LCD_ROWS`: The number of rows on the LCD display.
  - `DIP_PIN_1`, `DIP_PIN_2`, `DIP_PIN_3`: GPIO pins connected to the 3-position DIP switch.

- **`secrets.h`**:
  - `ssid`: Your WiFi network's SSID.
  - `password`: Your WiFi network's password.
  - `apiUrl`: The URL of the HomeWizard P1 Meter API (e.g., `http://<ip-address>/api/v1/data`).

- **DIP Switch Configuration**:
  - The `HYSTERESIS_TIME` and `POWER_THRESHOLD` can be configured dynamically using a 3-position DIP switch. This allows for easy adjustment without needing to re-flash the firmware.
  - The DIP switches should be connected to the following GPIO pins:
    - `DIP_PIN_1`: GPIO 25
    - `DIP_PIN_2`: GPIO 26
    - `DIP_PIN_3`: GPIO 23
  - The following table outlines the settings. A `0` corresponds to the `OFF` position, and a `1` corresponds to the `ON` position.

| DIP 1 | DIP 2 | DIP 3 | Hysteresis Time | Power Threshold |
| :---: | :---: | :---: | :---: | :---: |
| 0 | 0 | 0 | 120s | 500W |
| 0 | 0 | 1 | 120s | 1000W |
| 0 | 1 | 0 | 120s | 1500W |
| 0 | 1 | 1 | 120s | 2000W |
| 1 | 0 | 0 | 240s | 500W |
| 1 | 0 | 1 | 240s | 1000W |
| 1 | 1 | 0 | 240s | 1500W |
| 1 | 1 | 1 | 240s | 2000W |

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

    - Open the `config.h` file and adjust the `RELAY_PIN`, `POWER_THRESHOLD`, `HYSTERESIS_TIME`, `MEASUREMENT_INTERVAL`, `LCD_ADDRESS`, `LCD_COLS`, and `LCD_ROWS` values as needed.

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
- The LCD display shows the current solar power, charger status, hysteresis time, and power threshold. When the charger is on and the power is below the threshold, a countdown is displayed to indicate when the charger will turn off. The layout is as follows:

**Normal Mode:**

```
+-----------------+
|P:1234W  C:Off   |
|H:120s   T:1000W |
+-----------------+
```

**Charger Off Countdown:**

```
+-----------------+
|P:1234W   C:On    |
|Off in: 87s      |
+-----------------+
```

## Project Structure

```
esp32-energy-monitor/
├── src/
│   ├── main.cpp          # Main source file with setup() and loop()
│   ├── config.h          # Centralized configuration file
│   ├── secrets.h         # WiFi and API credentials
│   └── secrets.h.example # Example for secrets.h
├── platformio.ini        # PlatformIO project configuration
└── README.md             # This file
```

## Troubleshooting

-   **WiFi Not Connecting**: Double-check the `ssid` and `password` in `secrets.h`. Ensure your ESP32 is within WiFi range.
-   **API Errors**: Verify that the `apiUrl` for your HomeWizard device is correct and returns JSON data in the expected format. Check the Serial Monitor for HTTP error codes.
-   **LCD Not Working**: Check the I2C address of your LCD and make sure it matches the `LCD_ADDRESS` in `config.h`. Also, check the wiring between the ESP32 and the LCD.

## Contributing

Feel free to submit issues or pull requests for enhancements, bug fixes, or new features!

## License

This project is licensed under the MIT License. See the `license.txt` file for details.

## HomeWizard API Documentation

This project is designed to work with the HomeWizard Energy API.

Based on the HomeWizard API documentation, here is a summary of the Recent Measurement API endpoint (`/api/v1/data`).

The `/api/v1/data` endpoint allows you to retrieve the most recent measurement from a supported device. The API does not send data points that are null or unavailable, so your application should be able to handle optional fields.

### P1 Meter (HWE-P1)

Provides detailed smart meter readings.

**Key Parameters:**
*   `smr_version`: DSMR version of the smart meter.
*   `total_power_import_kwh`: Total energy usage in kWh.
*   `total_power_export_kwh`: Total energy feed-in in kWh.
*   `active_power_w`: Current active power usage in watts.
*   `active_power_l1_w`, `active_power_l2_w`, `active_power_l3_w`: Active power for each phase.
---
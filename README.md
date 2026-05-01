# simppeliTCU 🍃
**A lightweight, open-source DIY replacement for the Nissan Leaf ZE1 Telematics Control Unit (TCU).**

*Read this in Finnish: [README_FI.md](README_FI.md)*

With Nissan shutting down the NissanConnect EV services, `simppeliTCU` offers a local, Wi-Fi-based ESP32 solution to regain control of your car's climate and charging via the IT CAN bus (often referred to as CAR-CAN by the community). Similarly as the original TCU.

NOTE: This project is currently an draft implementation generated mainly with AI.

## Features
* 🌡️ **Remote Climate Control:** Start and active abort of the cabin heater/AC.
* 🔋 **Remote Charging:** Override the charge timer to start charging.
* 📊 **Live Data:** Reads State of Charge (SOC %) and Cabin Temperature from the CAN bus.
* 🌐 **Web UI:** Simple, lightweight mobile-friendly web interface.
* 📡 **MQTT Support:** Integrate with home automation or mobile apps over encrypted transport. Note: unless proper certificate validation is implemented, the MQTT server is not authenticated. See the [MQTT Setup Guide](docs/mqtt.md).

![Screenshot of the Web UI from mobile phone browser](docs/screenshot.jpg)

## Videos
* **First trials:** [https://youtu.be/qn05-901b3Y](https://youtu.be/qn05-901b3Y)
* **Web interface through wifi:** [https://youtube.com/shorts/XqAgwizNodE](https://youtube.com/shorts/XqAgwizNodE)

## Hardware Requirements
1. **Lilygo T-2CAN** (ESP32 with built-in CAN transceiver).
2. **Automotive 12V to 5V Step-Down Converter** (Highly recommended: a flush-mount 12V USB car socket).
3. Connectors/pins to mate with the original TCU harness.

**See the [Installation Guide](docs/INSTALLATION.md) for full step-by-step instructions, wiring diagrams, and required tools.**

### ⚠️ CRITICAL WIRING WARNING
**DO NOT connect the car's 12V line directly to the Lilygo board!** Automotive 12V systems experience massive voltage spikes (transients) that will most likely damage the ESP32. 
* Unplug the IT-CAN wires from original TCU.
* Route the 12V from the TCU harness through an automotive-grade 12V-to-5V USB adapter (with an inline fuse).
* Power the Lilygo using a standard USB cable from the adapter.
* Connect `CAN-H` and `CAN-L` to the Lilygo's TWAI pins (Port B: TX 7, RX 6 in this code).

## Configuration

The device configuration (Wi-Fi, MQTT credentials, etc.) is stored in persistent memory. You configure the system through the USB serial connection using a terminal (such as the Arduino IDE Serial Monitor, PuTTY, or similar) set to **115200 baud** with Newline (`\n`) or Carriage Return (`\r`) enabled.

Available commands:
* `list` - Shows all current configuration properties.
* `get <key>` - Prints the current value of the specified key.
* `set <key> <value>` - Updates the specified key with a new value (saved persistently).
* `reboot` - Restarts the device to apply the changes immediately.

**Example configuration session:**
```text
set ssid MyWiFiNetwork
set password MySecretPassword
set ap_ssid LeafLocalAP
set ap_password LeafLocalPassword
set mqtt_server 192.168.1.100
reboot
```

*Supported Configuration Keys:*
* `ssid`, `password` (For connecting to an existing Wi-Fi network / STA mode)
* `ap_ssid`, `ap_password` (For creating a local Wi-Fi Access Point / AP mode)
* `hostName`
* `mqtt_server`, `mqtt_port`, `mqtt_user`, `mqtt_password`, `vehicle_id`

**Wi-Fi Modes:**
* **STA Mode:** Active when `ssid` is configured. The device connects to your home/garage Wi-Fi.
* **AP Mode:** Active when `ap_ssid` is configured. The device broadcasts its own Wi-Fi network (useful when away from home).
* **AP + STA Mode:** Active when both `ssid` and `ap_ssid` are configured. The device will connect to your home Wi-Fi while simultaneously broadcasting its own Access Point.

## Compiling and Uploading

For instructions on how to compile and upload the firmware using the Arduino IDE or the Command Line Interface (CLI), please refer to the [Compiling and Uploading Guide](docs/compiling.md).

## Flashing Pre-compiled Binaries

For instructions on downloading and flashing pre-compiled release binaries (using GUI tools or `esptool`), please refer to the [Pre-compiled Binaries Flashing Guide](docs/flashing.md).

## The CAN communication
Please see the [CAN Communication Details](docs/can.md) document for information regarding CAN bus messages, commands, known issues, and limitations related to the IT-CAN integration.

## License & Disclaimer (MIT License)
This project is licensed under the MIT License. 

**DISCLAIMER OF LIABILITY:**
This software and hardware modification interacts with the high-voltage control systems of a vehicle. It is provided "AS IS", without warranty of any kind, express or implied. By using this code, you assume all risks. The authors or contributors will NOT be held liable for any damage to your vehicle, voided warranties, property damage, or personal injury resulting from the use of this software or hardware. **USE AT YOUR OWN RISK.**

## Credits
* Inspired by the incredible work of the [OVMS Project](https://github.com/openvehicles/Open-Vehicle-Monitoring-System-3) and [Dalathegreat](https://github.com/dalathegreat) (Dala's EV Repair) for documenting the Nissan Leaf CAN buses.
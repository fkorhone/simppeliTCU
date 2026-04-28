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

![Screenshot of the Web UI from mobile phone browser](screenshot.jpg)

## Videos
* **First trials:** [https://youtu.be/qn05-901b3Y](https://youtu.be/qn05-901b3Y)
* **Web interface through wifi:** [https://youtube.com/shorts/XqAgwizNodE](https://youtube.com/shorts/XqAgwizNodE)

## Hardware Requirements
1. **Lilygo T-2CAN** (ESP32 with built-in CAN transceiver).
2. **Automotive 12V to 5V Step-Down Converter** (Highly recommended: a flush-mount 12V USB car socket).
3. Connectors/pins to mate with the original TCU harness.

**See the [Installation Guide](INSTALLATION.md) for full step-by-step instructions, wiring diagrams, and required tools.**

### ⚠️ CRITICAL WIRING WARNING
**DO NOT connect the car's 12V line directly to the Lilygo board!** Automotive 12V systems experience massive voltage spikes (transients) that will most likely damage the ESP32. 
* Unplug the IT-CAN wires from original TCU.
* Route the 12V from the TCU harness through an automotive-grade 12V-to-5V USB adapter (with an inline fuse).
* Power the Lilygo using a standard USB cable from the adapter.
* Connect `CAN-H` and `CAN-L` to the Lilygo's TWAI pins (Port B: TX 7, RX 6 in this code).

## Configuration

Before compiling, set up your Wi-Fi credentials so the device can connect to your desired wireless network or mobile hotspot. This is required to access the device's web interface.

Open `simppeliTCU/configuration.h` and update the following variables with your network details:
```cpp
const char* ssid = "";
const char* password = "";
```

## Compiling and Uploading with Arduino IDE

To compile and upload the firmware for the Lilygo T-2CAN board using the Arduino IDE, please configure the following settings:

1. **Board Selection**: Select **ESP32S3 Dev Module** as your target board.
   * If you haven't set up the ESP32 core yet, please follow the [LilyGO T-2CAN Quick Start Guide](https://wiki.lilygo.cc/get_started/en/High_speed/T-2Can/T-2Can.html#Quick-Start) to download the required board files.
   
   ![Select ESP32S3 Dev Module](arduino_ide_esp32s3_dev_module.jpg)

2. **Serial Output Configuration**: **USB CDC On Boot** needs to be set to **"Enabled"** for the printouts to appear in the Serial Monitor.
   
   ![Enable USB CDC on Boot](set-cdc-on-boot-enabled.jpg)

## Compiling and Uploading with Arduino CLI

If you prefer using the command line, you can build and flash the firmware using the [Arduino CLI](https://arduino.github.io/arduino-cli/).

1. **Install the ESP32 core and required libraries**:
   ```bash
   arduino-cli core update-index --additional-urls https://espressif.github.io/arduino-esp32/package_esp32_index.json
   arduino-cli core install esp32:esp32 --additional-urls https://espressif.github.io/arduino-esp32/package_esp32_index.json
   arduino-cli lib install PubSubClient
   ```

2. **Compile the sketch** (ensuring USB CDC On Boot is enabled):
   ```bash
   arduino-cli compile --fqbn esp32:esp32:esp32s3:CDCOnBoot=cdc simppeliTCU
   ```

3. **Upload to the board** (replace `<PORT>` with your COM port, e.g., `COM3` or `/dev/cu.usbserial...`):
   ```bash
   arduino-cli upload -p <PORT> --fqbn esp32:esp32:esp32s3:CDCOnBoot=cdc simppeliTCU
   ```

## Flashing Pre-compiled Binaries

To get the pre-compiled binaries, go to the **Actions** tab of this GitHub repository, click on the latest successful "Build" run, and download the `simppeliTCU-firmware` zip from the Artifacts section. These pre-compiled binaries already have "USB CDC On Boot" enabled.

### Option 1: Easy Offline GUI (Windows)
For the easiest offline flashing experience, use Espressif's official tool:
1. Download the **Flash Download Tools** from [Espressif's website](https://www.espressif.com/en/support/download/other-tools).
2. Extract and run the tool. Select **Chip Type: ESP32-S3** and **WorkMode: Develop**.
3. In the tool:
   - Select the downloaded `simppeliTCU.ino.bin` file and set the address to `0x10000`.
   - Check the box next to the file path to enable it.
   - Select your LilyGo board's COM port at the bottom right.
4. Click **START** to flash the firmware.

### Option 2: Command Line (esptool)
If you prefer using the command line, you can flash it with `esptool` command (the tool automatically detects most of the hardware parameters):

```bash
# Install esptool if you haven't already: pip install esptool
esptool --chip esp32s3 -p <PORT> write_flash 0x10000 simppeliTCU.ino.bin
```
*(Replace `<PORT>` with your COM port, e.g., `COM3` on Windows or `/dev/cu.usbserial...` on Mac/Linux)*

Alternatively, you could use a browser-based tool like [Adafruit ESPTool](https://adafruit.github.io/Adafruit_WebSerial_ESPTool/) to flash the `.bin` to offset `0x10000`.

## The CAN Magic (Discoveries)
This project utilizes the `IT-CAN` bus (often referred to as CAR-CAN by the community). While climate commands are somewhat known, this project also maps the previously undocumented remote charging sequences for the ZE1:

* **Wakeup Ping:** `ID: 0x68C | Len: 1 | Data: 00` TCU seems to be calling this about 65 times to wake others u
* **Wakeup command:** `0x601 | Len: 2 | Data: 83 C0` Seen once and then the wakeup pinging stops after one last message.
* **TCU Command ID:** `0x56E | Len: 4`
  * **Init/Prepare:** `46 08 00 00` First CU message after the wakeup calls
  * **Sleep:** `86 00 00 00` Last 0x56E message when done and shortly after the IT-CAN goes quiet.
  * **Climate ON:** `4E 08 00 00`  
  * **Climate ABORT (Force Stop):** Sequence starting with `96 00 00 00`
  * **Climate OFF:** `56 08 00 00` Explicit deactivation, sent after Climate ABORT in the shutdown sequence. (`0x46` Init with bit 4 set)
  * **Charge ON:** `66 08 00 00` Called instead of Init/Prepare `46` but with one bit (0x20) being set

## Known Issues
* **CAN communication errors:** When the original TCU is disconnected from the CAN bus, other modules raise a set of CAN-related communication errors. The current implementation does not mimic the correct responses to keep other modules happy. However, no dashboard warning lights have illuminated, and no other issues have been noticed while driving.

## Known Limitations / Research
* **Sleep (charging):** `A6 00 00 00` Captured what appears to be variant of 0x86, not sure if this was error related (charging did not start this time) or always used at end of every charge start sequence.

## License & Disclaimer (MIT License)
This project is licensed under the MIT License. 

**DISCLAIMER OF LIABILITY:**
This software and hardware modification interacts with the high-voltage control systems of a vehicle. It is provided "AS IS", without warranty of any kind, express or implied. By using this code, you assume all risks. The authors or contributors will NOT be held liable for any damage to your vehicle, voided warranties, property damage, or personal injury resulting from the use of this software or hardware. **USE AT YOUR OWN RISK.**

## Credits
* Inspired by the incredible work of the [OVMS Project](https://github.com/openvehicles/Open-Vehicle-Monitoring-System-3) and [Dalathegreat](https://github.com/dalathegreat) (Dala's EV Repair) for documenting the Nissan Leaf CAN buses.
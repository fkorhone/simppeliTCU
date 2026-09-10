# Flashing Pre-compiled Binaries

Pre-compiled firmware binaries for each supported board (`LilyGo-T2-Can` and `Waveshare-esp32-rs485-can`) are available in two places:
1. **GitHub Releases (Recommended):** Go to the **Releases** section of this GitHub repository to download a versioned release (e.g., `v0.1.0`) along with its changelog.
2. **GitHub Actions Artifacts (Development builds):** For in-progress work, go to the **Actions** tab, click on the latest successful "Build" run, and download the artifact zip for your board.

All pre-compiled binaries already have "USB CDC On Boot" enabled. Make sure to download the binary matching your hardware (`simppeliTCU-LilyGo-T2-Can-*.bin` or `simppeliTCU-Waveshare-esp32-rs485-can-*.bin`).

> **Re-flashing / Updating in the Car:** Flashing the application binary at `0x10000` preserves your saved Wi-Fi, AP, and MQTT settings in persistent NVS memory. If your board is already installed in the glovebox, see [Updating Firmware in the Car](INSTALLATION.md#updating-firmware-in-the-car) for quick USB-C update steps without touching the harness behind the glovebox.

### Option 1: Easy Offline GUI (Windows)
For the easiest offline flashing experience, use Espressif's official tool:
1. Download the **Flash Download Tools** from [Espressif's website](https://www.espressif.com/en/support/download/other-tools).
2. Extract and run the tool. Select **Chip Type: ESP32-S3** and **WorkMode: Develop**.
3. In the tool:
   - Select the downloaded `.bin` file (e.g., `simppeliTCU-LilyGo-T2-Can.bin` or `simppeliTCU.ino.bin`) and set the address to `0x10000`.
   - Check the box next to the file path to enable it.
   - Select your board's COM port at the bottom right.
4. Click **START** to flash the firmware.

### Option 2: Command Line (esptool)
If you prefer using the command line, you can flash it with `esptool` command (the tool automatically detects most of the hardware parameters):

```bash
# Install esptool if you haven't already: pip install esptool
esptool --chip esp32s3 -p <PORT> write_flash 0x10000 <FIRMWARE_FILE>.bin
```
*(Replace `<PORT>` with your COM port, e.g., `COM3` on Windows or `/dev/cu.usbserial...` on Mac/Linux, and `<FIRMWARE_FILE>.bin` with the downloaded binary)*

Alternatively, you could use a browser-based tool like [Adafruit ESPTool](https://adafruit.github.io/Adafruit_WebSerial_ESPTool/) to flash the `.bin` to offset `0x10000`.
# Flashing Pre-compiled Binaries

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
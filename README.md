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

## Hardware Requirements
1. **Lilygo T-2CAN** (ESP32 with built-in CAN transceiver).
2. **Automotive 12V to 5V Step-Down Converter** (Highly recommended: a flush-mount 12V USB car socket).
3. Connectors/pins to mate with the original TCU harness.

### ⚠️ CRITICAL WIRING WARNING
**DO NOT connect the car's 12V line directly to the Lilygo board!** Automotive 12V systems experience massive voltage spikes (transients) that will most likely damage the ESP32. 
* Unplug the IT-CAN wires from original TCU.
* Route the 12V from the TCU harness through an automotive-grade 12V-to-5V USB adapter (with an inline fuse).
* Power the Lilygo using a standard USB cable from the adapter.
* Connect `CAN-H` and `CAN-L` to the Lilygo's TWAI pins (Port B: TX 7, RX 6 in this code).

## The CAN Magic (Discoveries)
This project utilizes the `IT-CAN` bus (often referred to as CAR-CAN by the community). While climate commands are somewhat known, this project also maps the previously undocumented remote charging sequences for the ZE1:

* **Wakeup Ping:** `ID: 0x68C | Len: 1 | Data: 00`
* **TCU Command ID:** `0x56E | Len: 4`
  * **Idle/Sleep:** `46 08 00 00` or `86 00 00 00`
  * **Climate ON:** `4E 08 00 00`
  * **Climate ABORT (Force Stop):** Sequence starting with `96 00 00 00`
  * **Charge ON:** `66 08 00 00`

## Known Limitations / Research
* 🛑 **Charge Abort (`0xA6...`):** We captured what appears to be the TCU's charge abort sequence. However, seems that the Leaf's Vehicle Control Module (VCM) possibly ignores this command. The sequence is included in the code and documentation for research purposes, but it does NOT stop an active charge. If you want to stop charging, you currently need to rely on a smart EVSE (wallbox) or the car's physical unplug button.

## License & Disclaimer (MIT License)
This project is licensed under the MIT License. 

**DISCLAIMER OF LIABILITY:**
This software and hardware modification interacts with the high-voltage control systems of a vehicle. It is provided "AS IS", without warranty of any kind, express or implied. By using this code, you assume all risks. The authors or contributors will NOT be held liable for any damage to your vehicle, voided warranties, property damage, or personal injury resulting from the use of this software or hardware. **USE AT YOUR OWN RISK.**

## Credits
* Inspired by the incredible work of the [OVMS Project](https://github.com/openvehicles/Open-Vehicle-Monitoring-System-3) and [Dalathegreat](https://github.com/dalathegreat) (Dala's EV Repair) for documenting the Nissan Leaf CAN buses.
# Installation Guide (Nissan Leaf ZE1)

This guide covers wiring and installing `simppeliTCU` in place of the Nissan Leaf ZE1 Telematics Control Unit (TCU) behind the glovebox.

## Overview & Choosing an Installation Approach

To let `simppeliTCU` communicate on the car's `IT-CAN` bus without conflicting with the factory TCU, the two `IT-CAN` wires (**Pin `#6` CAN-H** and **Pin `#7` CAN-L**) must be disconnected from the original 40-pin TCU plug and routed to your ESP32 CAN board.

**Why keep `+12V` (`#1`) and `GND` (`#28`) connected to the original TCU?**
The car's **hands-free phone microphone** routes through the original TCU. If you unplug the TCU completely (or leave its power pins disconnected), `simppeliTCU` works fine, **but the cabin hands-free microphone will stop working**. Looping `+12V` and `GND` back to the factory TCU keeps the microphone working normally while isolating the TCU from `IT-CAN`.

You can choose from three car-side wiring approaches:

| Approach | Factory Wires Cut/Spliced? | Hands-Free Mic Works? | Summary |
| :--- | :---: | :---: | :--- |
| **Option A (Recommended): 16-Pin Sumitomo Adapter** | **No** (100% reversible) | **Yes** | Unpin 4 wires (`#1, #6, #7, #28`) from the 40-pin TCU plug into a 16-pin NH `.025"` Female housing, and loop `+12V` & `GND` back into TCU `#1` & `#28` via pins `9` & `10`. |
| **Option B: CAN-Only Unpin + Power Tap** | Power wires tapped | **Yes** | Unpin only `#6` (`CAN-H`) and `#7` (`CAN-L`) from the 40-pin TCU plug, leave `#1` & `#28` plugged into the TCU, and tap `+12V` (with a 2A fuse) & `GND` using Posi-Tap, Wago 221, or solder. |
| **Option C: Original TCU Unplugged** | **No** | **No** | Leave the 40-pin plug disconnected from the original TCU and route only `#1, #6, #7, #28` to `simppeliTCU` (no power loopback wires needed). |

> **Why is there a 16-pin connector in Option A?**
> The male/female Sumitomo connector behind the glovebox exists so you can adapt the unpinned factory terminals cleanly **without cutting or splicing any original car wires**. It is **not** intended to be unplugged for routine firmware updates (unplugging it also disconnects power to the factory TCU). For firmware updates, route the board or a USB cable into the glovebox and flash directly over USB-C—see [Updating Firmware in the Car](#updating-firmware-in-the-car).

---

## Wiring Diagrams

*All connector pinout grids in the diagrams below are drawn from the **Wire-Side View** (looking into the back of the connector where wires enter, with the locking tab at the top—matching the photo in Step 6).*

### Option A (Recommended): Non-Destructive 16-Pin Adapter — LilyGo T-2CAN
![Option A Wiring Diagram — LilyGo T-2CAN](wiring_lilygo_adapter.svg)

### Alternative Controller: Waveshare ESP32-S3-RS485-CAN (Device Side)
If using the **Waveshare ESP32-S3-RS485-CAN**, its built-in `DC 7–36V` regulator accepts fused nominal `12V` directly (no external 12V-to-5V USB converter needed). The Car Side (16-pin Female connector) is identical to Option A above.
![Waveshare ESP32-S3-RS485-CAN Wiring Diagram](wiring_waveshare_adapter.svg)

### Simplified Car-Side Alternatives (Option B & Option C)
![Alternative Car-Side Wiring Options B and C](wiring_alternatives.svg)

*(Diagrams are generated from [`docs/generate_diagrams.py`](generate_diagrams.py) for easy maintenance.)*

---

## Master Pinout & Wire Reference (Option A)

| Signal | OEM Wire Color | Original 40-Pin TCU Cavity | 16-Pin Female (Car Side) | 16-Pin Male (Device Side) | Device-Side Destination |
| :--- | :--- | :--- | :---: | :---: | :--- |
| **+12V (BAT)** | White (`W`) | Unpinned from **#1** | **Pin 1** *(OEM pin)* | **Pin 1** | Splits (3 ends): **2A Inline Fuse -> 12V-to-USB `+`** *(or Waveshare `DC +`)* AND loops to **Male Pin 9** |
| **GND** | Black (`B`) | Unpinned from **#28** | **Pin 2** *(OEM pin)* | **Pin 2** | Splits (4 ends on LilyGo): **12V-to-USB `-`**, **LilyGo `SGNDB`**, AND loops to **Male Pin 10** *(3 ends on Waveshare -> `DC -` & Pin 10)* |
| **IT-CAN H** | Sky Blue (`SB`) | Unpinned from **#6** *(leave #6 empty)* | **Pin 3** *(OEM pin)* | **Pin 3** | **LilyGo Port B `CANHB`** *(or Waveshare `CAN-H`)* |
| **IT-CAN L** | Violet (`V`) | Unpinned from **#7** *(leave #7 empty)* | **Pin 4** *(OEM pin)* | **Pin 4** | **LilyGo Port B `CANLB`** *(or Waveshare `CAN-L`)* |
| **+12V TCU Return** | Red *(new wire)* | Inserted into **#1** *(new female pin)* | **Pin 9** *(new female pin)* | **Pin 9** | Receives looped `+12V` from **Male Pin 1** (powers original TCU mic) |
| **GND TCU Return** | Black *(new wire)* | Inserted into **#28** *(new female pin)* | **Pin 10** *(new female pin)* | **Pin 10** | Receives looped `GND` from **Male Pin 2** (grounds original TCU mic) |

* **Connector pin numbering flexibility:** Which exact slots (`1, 2, 3, 4, 9, 10`) you use inside the 16-pin Sumitomo connector does not matter electrically, **as long as every pin on the Male side lines up with the matching signal on the Female side** (remembering that Male and Female Wire-Side views are horizontal mirror images).
* **Note on `SGNDB` (Signal Ground):** Connecting `SGNDB` on LilyGo Port B to ground is included by default and recommended for boards with an isolated CAN transceiver so the transceiver reference does not float, although it will most likely function without `SGNDB` connected as well.
* **LilyGo Port B vs. Port A:** Connect `IT-CAN` to **Port B (`CANHB` / `CANLB` / `SGNDB`)**, which uses the ESP32's built-in TWAI controller (`GPIO 7 / 6`). The second screw terminal block (**Port A**, MCP2515) is only for optional `AV-CAN` features (nag screen auto-dismiss and GPS—see [README.md](../README.md#dual-can-bus-support-lilygo-t-2can)).

---

## Required Tools and Parts

### Tools
* Cross / Phillips screwdriver (for removing the glovebox and TCU bracket)
* Terminal unpinning pick or very small jeweler's flat screwdriver (for releasing the 40-pin TCU lock & terminals)
* Open-barrel terminal crimping tool (recommended) or fine needle-nose pliers + soldering iron

### Parts & Accessories
* **Controller & Power Supply (choose one):**
  * **LilyGo T-2CAN** + **Automotive 12V-to-5V USB converter** (e.g., [Motonet 12V USB socket](https://www.motonet.fi/tuote/four-vedenpitava-usb-pistorasia-12-24-v?product=65-01122)) + **USB-A to USB-C cable** *(NEVER connect automotive 12V directly to the LilyGo board!)*
  * *Alternative:* **Waveshare ESP32-S3-RS485-CAN** (connects via fused 12V & GND directly to its `DC 7–36V` screw terminals; no separate 12V-to-5V USB converter needed)
* **Inline fuse holder + 2A automotive fuse** (1A–3A blade or glass fuse on the `+12V` line feeding your converter/Waveshare; note that some flush-mount 12V USB sockets already come with a pre-wired inline fuse)
* **For Option A:** **16-Pin Sumitomo NH `.025"` Male & Female connector kit** (e.g., [Eastern Beaver 16-Pin Sumitomo NH .025" Connector](https://www.easternbeaver.com/product/16-pin-sumitomo-nh-025-connector/))
  * *Terminal compatibility:* The NH `.025"` female terminals in this kit fit directly into the Nissan 40-pin TCU connector. One 16-pin kit includes 16 male + 16 female pins, which is plenty for Option A (**6 male pins** + **4 female pins** used; the 4 unpinned OEM wires reuse their factory terminals).
* Automotive stranded wire (`0.35–0.5 mm²` / `22–20 AWG`, sized for `.025"` terminals) and heat-shrink tubing / electrical tape.

---

## Phase 1: Bench Preparation (Before Going to the Car)

Doing the flashing, initial setup, and crimping indoors at a desk makes the in-car work much faster and easier.

1. **Flash Firmware & Configure Settings on Your Computer:**
   * Connect your board to your computer via USB-C and flash the firmware using either:
     * **[Pre-compiled Binaries Flashing Guide](flashing.md)** *(easiest — download the `.bin` for your board from GitHub Releases)*, or
     * **[Compiling and Uploading Guide](compiling.md)** *(build from source with Arduino IDE or Arduino CLI)*.
   * Open a serial monitor at **115200 baud** (with Newline `\n` or Carriage Return `\r` enabled) and set your Wi-Fi / AP / MQTT configuration (e.g., `set ssid MyWiFi`, `set password MySecret`, `reboot` — see [README Configuration](../README.md#configuration)).

2. **Build the Device-Side Harness (Option A — 16-Pin Male Housing):**
   * **2.1. CAN wires:** Crimp NH `.025"` **male** terminals onto one end of two wires (`CAN-H` and `CAN-L`). Screw the stripped other ends into **LilyGo Port B `CANHB`** and **`CANLB`** *(or Waveshare `CAN-H` and `CAN-L`)*.
   * **2.2. `+12V` Y-split wire (3 ends total):** Join three wire branches (by crimping or soldering + heat-shrink):
     1. NH `.025"` **male** pin -> **Male Pin `1`** (`+12V` input from car)
     2. NH `.025"` **male** pin -> **Male Pin `9`** (`+12V` loopback to original TCU)
     3. **2A inline fuse** -> **12V-to-USB converter `+`** terminal *(or Waveshare `DC +`)*
   * **2.3. `GND` split wire (4 ends for LilyGo; 3 ends for Waveshare):** Join the ground branches:
     1. NH `.025"` **male** pin -> **Male Pin `2`** (`GND` input from car)
     2. NH `.025"` **male** pin -> **Male Pin `10`** (`GND` loopback to original TCU)
     3. **12V-to-USB converter `-`** terminal *(or Waveshare `DC -`)*
     4. *(LilyGo T-2CAN)* **LilyGo Port B `SGNDB`** screw terminal
   * **2.4. Insert & lock Male pins:** Release/open the secondary terminal lock on the **16-pin Male housing**, insert all 6 male terminals (`1, 2, 3, 4, 9, 10`) from the wire side until each clicks (`1: +12V`, `2: GND`, `3: CAN-H`, `4: CAN-L`, `9: +12V loop`, `10: GND loop`), gently tug-test each wire, and **press the Male housing's secondary lock closed**.

3. **Build the Two Short TCU Power-Return Wires (Option A Only):**
   * Cut two short (~10–15 cm) wires (one red for `+12V`, one black for `GND`) and crimp an NH `.025"` **female** terminal onto **both ends** of each wire (4 female terminals total).

---

## Phase 2: In-Car Installation

4. **Put the Car to Sleep & Disconnect the Negative 12V Battery Terminal:**
   * **4.1.** Turn the car power ON and then OFF, close all doors, and **wait at least 5 minutes** for the car's ECUs and CAN buses to go to sleep.
   * **4.2.** Disconnect the **negative (`-`) terminal** of the 12V battery under the hood.

5. **Remove Glovebox & Unplug the Original TCU:**
   * Remove the glovebox (see [LHD ZE1 Leaf video tutorial](https://youtu.be/kyx2A4U-M1M?si=YGewj1TeBEfrpOKV)).
   * Unbolt/remove the TCU bracket behind the glovebox and detach its black **40-pin connector**.

6. **Unpin Wires `#1, #6, #7, #28` from the 40-Pin TCU Connector:**

   ![TCU pins Wire-Side View](tcu_pins.jpg)

   * **6.1.** Unclip and remove the white plastic protective end cap from the mating face of the 40-pin connector (opposite side from the wires):

     ![40-pin TCU connector end cap](end_cap.jpg)

   * **6.2.** Release the secondary terminal locking bar on the side of the 40-pin connector housing. *(Tip: Prying this lock bar up to its unlocked position takes patience and a small flat screwdriver/pick—work gently from each end so the plastic tabs don't crack.)*
   * **6.3.** Using a fine unpinning pick from the front face, lift the internal plastic locking lance inside each cavity while gently pulling the wire out from the back:
     * **Option A / C:** Extract all 4 wires: **`#1`** (White `+12V`), **`#6`** (Sky Blue `CAN-H`), **`#7`** (Violet `CAN-L`), and **`#28`** (Black `GND`).
     * **Option B:** Extract only **`#6`** and **`#7`** (leave `#1` and `#28` in the connector and tap them on the harness, avoiding insulation-displacement clips / *"rosvoliitin"* on CAN data wires).

7. **Populate & Lock the 16-Pin Female Housing (Option A):**
   * Release/open the secondary lock on the new **16-pin Female housing**.
   * Push the 4 unpinned factory wires into the wire side of the Female housing until each clicks:
     * Factory **`#1` (White, `+12V`)** -> **Female Pin `1`**
     * Factory **`#28` (Black, `GND`)** -> **Female Pin `2`**
     * Factory **`#6` (Sky Blue, `CAN-H`)** -> **Female Pin `3`**
     * Factory **`#7` (Violet, `CAN-L`)** -> **Female Pin `4`**
   * Push one end of the short red return wire into **Female Pin `9`**, and one end of the short black return wire into **Female Pin `10`**.
   * Tug-test all 6 wires, then **press the 16-pin Female housing's secondary lock closed**.

8. **Insert Return Wires & Re-Lock the 40-Pin TCU Connector (Option A):**
   * Push the free end of the short red return wire (from Female Pin `9`) into **40-pin TCU cavity `#1`**, and the free end of the short black return wire (from Female Pin `10`) into **40-pin TCU cavity `#28`** until they click. Leave cavities **`#6`** and **`#7`** empty.
   * **Press the 40-pin connector's secondary locking bar back down into its locked position** and **snap the white protective end cap back onto the front**.
   * **Plug the 40-pin connector back into the original TCU** until its main latch clicks, then bolt the TCU back into its mounting bracket.

9. **Connect Adapter, Position Board & Reassemble:**
   * Mate the **16-pin Male and Female Sumitomo connectors** together until they click.
   * Secure the 12V-to-USB converter and 2A inline fuse behind the glovebox area so nothing rattles.
   * Place the `simppeliTCU` board (in an enclosure) in a convenient spot. **Tip:** Route the board or USB cable into the **glovebox through the small access hatch on the side of the glovebox**—this makes future USB-C firmware updates effortless without removing the glovebox again.
   * Double-check all connections (`+12V` vs `GND` polarity, `2A` inline fuse installed, `CAN-H` / `CAN-L` order).
   * Reinstall the glovebox, **reconnect the negative (`-`) 12V battery terminal**, and plug the USB power cable into the 12V USB converter to power up `simppeliTCU`.

---

## Updating Firmware in the Car

When a new `simppeliTCU` version is released, **you do not need to access the wiring behind the glovebox or unplug the 16-pin Sumitomo connector**:

* **LilyGo T-2CAN:** Open the glovebox (with the car parked/off). Unplug the USB cable from the car's 12V USB socket (or unplug the USB-C end from the LilyGo), connect your laptop directly to the LilyGo's USB-C port, flash the update using the [Pre-compiled Binaries Guide](flashing.md) or [Compiling Guide](compiling.md), and plug the car's 5V USB power cable back in. *(The CAN screw terminals can stay connected during flashing, and your Wi-Fi/MQTT settings in persistent memory are preserved.)*
* **Waveshare ESP32-S3-RS485-CAN:** Pull the inline 2A fuse (or disconnect the `DC 7–36V` terminal block) before connecting your laptop to the board's USB-C port so the board is only powered from one source during flashing, flash the update, and reconnect the 2A fuse / DC terminal.

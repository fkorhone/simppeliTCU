# The CAN communication
This project utilizes the `IT-CAN` bus (often referred to as CAR-CAN by the community). Climate commands are somewhat known, this project utilises some remote charging sequences for the ZE1:

* **Wakeup Ping:** `ID: 0x68C | Len: 1 | Data: 00` TCU seems to be calling this about 65 times to wake others up
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
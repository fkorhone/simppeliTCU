#include "canLeafZE1.h"

// Can message reception handler, to be called when a CAN message is received
void handleReceivedMessage(uint32_t identifier, uint8_t* data, uint8_t len) {
    if (receivedMessageIs(identifier, len, raw_soc_readout)) {
      uint16_t soc_raw = (data[0] << 2) | (data[1] >> 6);
      handleRawSOC(soc_raw / 10.0);
    }
    else if (receivedMessageIs(identifier, len, cabin_temp_readout)) {
      handleCabinTemp((data[0] / 2.0) - 40.0);
    }
    else if (receivedMessageIs(identifier, len, car_awake_readout)) {
      handleCarAwake();
    }
}

// Control Sequences
void wake() {
  sendCAN(wakeup_data);
  waitCAN(15);
}

void refreshSequence() {
  sendCAN(heating_init, 20, 100);
  waitCAN(1000);
  sendCAN(idle_data, 20, 100);
}

void heatOnSequence() {
  sendCAN(heating_init, 20, 100);
  sendCAN(heat_on_data, 20, 100);
  sendCAN(idle_data, 20, 100);
  // After this the bus goes quiet, but the heating is on. 
}

void heatOffSequence() {
  sendCAN(heating_init, 10, 100); 
  sendCAN(interrupt_data, 9, 100);
  sendCAN(heat_off_data, 8, 100);
  sendCAN(heating_init, 4, 100);
  sendCAN(idle_data, 8, 100);
  // After this the bus goes quiet, and the heating is off.
}

void chargeOnSequence() {
  sendCAN(start_charge_data, 20, 100);
  sendCAN(idle_data, 8, 100);
}

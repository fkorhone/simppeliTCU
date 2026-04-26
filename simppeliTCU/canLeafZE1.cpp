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
    else if (receivedMessageIs(identifier, len, charger_status_readout)) {
      uint8_t cStatus = (data[5] >> 1) & 0x3f;
      bool qc_state = (data[4] & 0x40) == 0x40;
      bool vg_state = (data[0] >> 4) == 0x09;
      float chargeVoltage = ((data[3] >> 3) & 0x03) * 110.0f;
      
      bool isCharging = false;
      ChargerState state = ChargerState::IDLE;

      if (cStatus == 0x01) {
        if (qc_state && !vg_state) {
          state = ChargerState::CHARGING;
          isCharging = true;
        } else {
          state = ChargerState::IDLE;
        }
      } else if (cStatus == 0x04) {
        if (chargeVoltage > 0) {
          state = ChargerState::CHARGING;
          isCharging = true;
        } else {
          state = ChargerState::INTERRUPTED;
        }
      } else if (cStatus == 0x02) {
        state = ChargerState::FINISHED;
      } else if (cStatus == 0x0c) {
        state = ChargerState::WAITING;
      }
      
      handleChargerStatus(isCharging, state);
    }
    else if (receivedMessageIs(identifier, len, hvac_status_readout)) {
      bool isOn = (data[1] & 0x01) || (data[1] & 0x30);
      handleHVACStatus(isOn);
    }
}

// Control Sequences
void wake() {
  sendCAN(wakeup_data);
  waitCAN(15);
}

void refreshSequence() {
  sendCAN(hvac_init, 20, 100);
  waitCAN(1000);
  sendCAN(idle_data, 20, 100);
}

void hvacOnSequence() {
  sendCAN(hvac_init, 20, 100);
  sendCAN(hvac_on_data, 20, 100);
  sendCAN(idle_data, 20, 100);
  // After this the bus goes quiet, but the HVAC is on. 
}

void hvacOffSequence() {
  sendCAN(hvac_init, 10, 100); 
  sendCAN(interrupt_data, 9, 100);
  sendCAN(hvac_off_data, 8, 100);
  sendCAN(hvac_init, 4, 100);
  sendCAN(idle_data, 8, 100);
  // After this the bus goes quiet, and the HVAC is off.
}

void chargeOnSequence() {
  sendCAN(start_charge_data, 20, 100);
  sendCAN(idle_data, 8, 100);
}

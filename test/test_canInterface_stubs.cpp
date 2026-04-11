#include <stdint.h>
#include <stddef.h>

#include "../simppeliTCU/vehicleTypes.h"

bool carIsAwake = false;

float lastSOC_value = -1.0f;
float lastTemp_value = -100.0f;
bool lastCharging_value = false;
ChargerState lastChargerState_value = ChargerState::IDLE;
bool lastHVAC_value = false;
bool lastDoor_fl = false;
bool lastDoor_fr = false;
bool lastDoor_rl = false;
bool lastDoor_rr = false;
bool lastDoor_trunk = false;
bool lastLockStatus = false;

void handleRawSOC(float soc) {
    lastSOC_value = soc;
}

void handleCabinTemp(float temp) {
    lastTemp_value = temp;
}

void handleCarAwake() {
    carIsAwake = true;
}

void handleChargerStatus(bool isCharging, ChargerState state) {
    lastCharging_value = isCharging;
    lastChargerState_value = state;
}

void handleHVACStatus(bool isOn) {
    lastHVAC_value = isOn;
}

void handleDoorStatus(bool fl, bool fr, bool rl, bool rr, bool trunk) {
    lastDoor_fl = fl;
    lastDoor_fr = fr;
    lastDoor_rl = rl;
    lastDoor_rr = rr;
    lastDoor_trunk = trunk;
}

void handleLockStatus(bool locked) {
    lastLockStatus = locked;
}

void sendCAN(uint32_t id, const uint8_t* data, uint8_t len) {
    (void)id; (void)data; (void)len;
}

void resetCanLogTimestamps() {
}

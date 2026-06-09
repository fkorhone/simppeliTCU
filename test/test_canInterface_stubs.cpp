#include <stdint.h>
#include <stddef.h>

#include "../simppeliTCU/vehicleTypes.h"

bool carIsAwake = false;

float lastSOC_value = -1.0f;
float lastTemp_value = -100.0f;
bool lastCharging_value = false;
ChargerState lastChargerState_value = ChargerState::IDLE;
bool lastHVAC_value = false;

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

void sendCAN(uint32_t id, const uint8_t* data, uint8_t len) {
    (void)id; (void)data; (void)len;
}

void resetCanLogTimestamps() {
}

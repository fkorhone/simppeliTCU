#ifndef MQTT_INTERFACE_H
#define MQTT_INTERFACE_H

#include <Arduino.h>
#include "vehicleTypes.h"

// Initialization and Loop
void setupMQTT();
void manageMQTT();

// Status updates
void mqttUpdateSOC(float soc);
void mqttUpdateCabinTemp(float temp);
void mqttUpdateCharging(bool isCharging, ChargerState state);
void mqttUpdateHVAC(bool isOn);
void mqttPublishStatus(const char* msg);

// Optional callbacks implemented in simppeliTCU.ino
extern void handleMqttHvacOn();
extern void handleMqttHvacOff();
extern void handleMqttChargeOn();
extern void handleMqttRefresh();

#endif

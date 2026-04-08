#ifndef CAN_LEAF_ZE1_H
#define CAN_LEAF_ZE1_H

#include "canInterface.h"

// Leaf readout messages
inline constexpr CANMessage<2> raw_soc_readout = {0x55B};
inline constexpr CANMessage<1> cabin_temp_readout = {0x54F};
inline constexpr CANMessage<0> car_awake_readout = {0x601};

// Leaf can messages
inline constexpr CANMessage<1> wakeup_data       = {0x68C, {0x00}};                  // Wakeup ping
inline constexpr CANMessage<4> heating_init      = {0x56E, {0x46, 0x08, 0x00, 0x00}}; // Heating init
inline constexpr CANMessage<4> idle_data         = {0x56E, {0x86, 0x00, 0x00, 0x00}}; // Go to sleep
inline constexpr CANMessage<4> heat_on_data      = {0x56E, {0x4E, 0x08, 0x00, 0x00}}; // Heating ON
inline constexpr CANMessage<4> start_charge_data = {0x56E, {0x66, 0x08, 0x00, 0x00}}; // Charging ON
inline constexpr CANMessage<4> interrupt_data    = {0x56E, {0x96, 0x00, 0x00, 0x00}}; // Interrupt action
inline constexpr CANMessage<4> heat_off_data     = {0x56E, {0x56, 0x08, 0x00, 0x00}}; // Heating OFF

// Readout callbacks (to be called when a message is received, implemented at application side):
void handleRawSOC(float soc);
void handleCabinTemp(float temp);
void handleCarAwake();

// Control Sequences
void wake();
void refreshSequence();
void heatOnSequence();
void heatOffSequence();
void chargeOnSequence();


#endif
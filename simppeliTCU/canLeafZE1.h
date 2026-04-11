#ifndef CAN_LEAF_ZE1_H
#define CAN_LEAF_ZE1_H

#include "canInterface.h"
#include "vehicleTypes.h"

// Leaf readout messages
inline constexpr CANMessage<2> raw_soc_readout = {0x55B};
inline constexpr CANMessage<1> cabin_temp_readout = {0x54F};
inline constexpr CANMessage<0> car_awake_readout = {0x601};
inline constexpr CANMessage<6> charger_status_readout = {0x390};
inline constexpr CANMessage<2> hvac_status_readout = {0x54B};
inline constexpr CANMessage<3> doors_and_locks_readout = {0x60D};

// Leaf can messages
inline constexpr CANMessage<1> wakeup_data       = {0x68C, {0x00}};                  // Wakeup ping
inline constexpr CANMessage<4> hvac_init         = {0x56E, {0x46, 0x08, 0x00, 0x00}}; // HVAC init
inline constexpr CANMessage<4> idle_data         = {0x56E, {0x86, 0x00, 0x00, 0x00}}; // Go to sleep
inline constexpr CANMessage<4> hvac_on_data      = {0x56E, {0x4E, 0x08, 0x00, 0x00}}; // HVAC ON
inline constexpr CANMessage<4> start_charge_data = {0x56E, {0x66, 0x08, 0x00, 0x00}}; // Charging ON
inline constexpr CANMessage<4> interrupt_data    = {0x56E, {0x96, 0x00, 0x00, 0x00}}; // Interrupt action
inline constexpr CANMessage<4> hvac_off_data     = {0x56E, {0x56, 0x08, 0x00, 0x00}}; // HVAC OFF
inline constexpr CANMessage<4> unlock_doors_data = {0x56E, {0x11, 0x00, 0x00, 0x00}}; // Unlock doors
inline constexpr CANMessage<4> lock_doors_data   = {0x56E, {0x60, 0x80, 0x00, 0x00}}; // Lock doors

// ===== CAN Field Definitions (bit 0 = MSB of byte 0) =====
inline constexpr CANField soc_field = { 0, 10 };    
inline constexpr CANField cabin_temp_field = { 0, 8 }; 
inline constexpr CANField cStatus_field   = { 41, 6 }; 
inline constexpr CANField qc_state_field  = { 33, 1 }; 
inline constexpr CANField vg_state_field   = { 0, 4 };  
inline constexpr CANField chargeVolt_field = { 27, 2 }; 
inline constexpr CANField hvac_bit0_field  = { 15, 1 };
inline constexpr CANField hvac_bit4_field  = { 11, 1 }; 
inline constexpr CANField hvac_bit5_field  = { 10, 1 }; 
inline constexpr CANField door_trunk_field = { 0, 1 };
inline constexpr CANField door_rl_field    = { 1, 1 };
inline constexpr CANField door_rr_field    = { 2, 1 };
inline constexpr CANField door_fr_field    = { 3, 1 };
inline constexpr CANField door_fl_field    = { 4, 1 };
inline constexpr CANField locked_field     = { 19, 1 };

// Scaling definitions
inline constexpr FieldScaling soc_scaling        = {false, 0.1f,   0.0f};
inline constexpr FieldScaling cabin_temp_scaling = {false, 0.5f,  -40.0f};
inline constexpr FieldScaling voltage_scaling    = {false, 110.0f, 0.0f};

// Readout callbacks (to be called when a message is received, implemented at application side):
void handleRawSOC(float soc);
void handleCabinTemp(float temp);
void handleCarAwake();
void handleChargerStatus(bool isCharging, ChargerState state);
void handleHVACStatus(bool isOn);
void handleDoorStatus(bool fl, bool fr, bool rl, bool rr, bool trunk);
void handleLockStatus(bool locked);

// Sequence State Machine
enum class CanSequence {
    NONE,
    REFRESH,
    HVAC_ON,
    HVAC_OFF,
    CHARGE_ON,
    UNLOCK_DOORS,
    LOCK_DOORS
};

enum class CanSeqResult { IDLE, PROCESSING, WAKE_SUCCESS, WAKE_TIMEOUT, SEQUENCE_FINISHED };

extern CanSequence activeSequence;

void startSequence(CanSequence seq, unsigned long currentTimeMs);
CanSeqResult manageCANSequence(unsigned long currentTimeMs);

#endif

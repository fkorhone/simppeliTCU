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
inline constexpr CANMessage<5> hvac_setpoint_readout = {0x54A};
inline constexpr CANMessage<3> doors_and_locks_readout = {0x60D};

// Leaf can messages
inline constexpr CANMessage<1> wakeup_data       = {0x68C, {0x00}};                  // Wakeup ping
inline constexpr CANMessage<4> hvac_init         = {0x56E, {0x46, 0x08, 0x00, 0x00}}; // HVAC init
inline constexpr CANMessage<4> idle_data         = {0x56E, {0x86, 0x00, 0x00, 0x00}}; // Go to sleep
inline constexpr CANMessage<4> hvac_on_data      = {0x56E, {0x4E, 0x08, 0x00, 0x00}}; // HVAC ON
inline constexpr CANMessage<4> hvac_on_idle      = {0x56E, {0x8E, 0x00, 0x00, 0x00}}; // Stop HVAC ON
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
inline constexpr CANField hvac_setpoint_field = { 32, 8 };
inline constexpr CANField fan_speed_field = { 34, 3 }; // Bits 5, 4, 3 (MSB bit 7 is 32)
inline constexpr CANField ventilation_mode_field = { 16, 8 }; // Byte 2 contains ventilation mode
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
inline constexpr FieldScaling setpoint_scaling   = {false, 0.5f,   0.0f};
inline constexpr FieldScaling fan_speed_scaling  = {false, 100.0f/7, 0.0f};

// Readout callbacks (to be called when a message is received, implemented at application side):
void handleRawSOC(float soc);
void handleCabinTemp(float temp);
void handleCarAwake();
void handleChargerStatus(bool isCharging, ChargerState state);
void handleHVACStatus(bool isOn);
void handleHVACSetpoint(float setpoint);
void handleFanSpeed(float speed);
void handleHeatingMode(bool heating, bool cooling);
void handleVentilationMode(VentilationMode mode);
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

// Sequence Payload Modifier Callback Type
typedef void (*SequencePayloadModifier)(uint8_t* data, uint8_t len);

// Application layer payload modifiers
void setHVACTargetTemperature(float setpoint);

void startSequence(CanSequence seq, unsigned long currentTimeMs);
CanSeqResult manageCANSequence(unsigned long currentTimeMs);

#endif

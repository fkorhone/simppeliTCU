#ifndef CAN_LEAF_ZE1_H
#define CAN_LEAF_ZE1_H

// Leaf can messages
static uint8_t wakeup_data[1]       = {0x00};                   // Wakeup ping (0x68C)
static uint8_t heating_init[4]      = {0x46, 0x08, 0x00, 0x00}; // Heating init (0x56E)
static uint8_t idle_data[4]         = {0x86, 0x00, 0x00, 0x00}; // Go to sleep (0x56E) Needs verification!

static uint8_t heat_on_data[4]      = {0x4E, 0x08, 0x00, 0x00}; // Heating ON (0x56E)
static uint8_t start_charge_data[4] = {0x66, 0x08, 0x00, 0x00}; // Charging ON (0x56E)

static uint8_t interrupt_data[4]    = {0x96, 0x00, 0x00, 0x00}; // Interrupt action (0x56E)
static uint8_t heat_off_data[4]     = {0x56, 0x08, 0x00, 0x00}; // Heating OFF (0x56E)

typedef struct {
  uint32_t identifier;
  uint8_t data[4];
} CANMessage8;

typedef struct {
  uint32_t identifier;
  size_t length;
} CANMessageReadout;

// Leaf readout messages
static CANMessageReadout raw_soc_readout = {0x55B,2};
static CANMessageReadout cabin_temp_readout = {0x54F,1};
static CANMessageReadout car_awake_readout = {0x601,0};


#endif
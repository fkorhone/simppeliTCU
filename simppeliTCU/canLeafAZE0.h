#ifndef CAN_LEAF_AZE0_H
#define CAN_LEAF_AZE0_H

#include "canInterface.h"

// Leaf AZE0 readout messages
inline constexpr CANMessage<8> aze0_dashboard_soc_readout = {0x1DB}; // AZE0 dashboard SOC

// AZE0 SOC field: byte 4, bits 0-6 (7 bits)
inline constexpr CANField aze0_soc_field = { 33, 7 };
inline constexpr FieldScaling aze0_soc_scaling = {false, 1.0f, 0.0f};
inline constexpr uint8_t aze0_dashboard_soc_sentinel = 0x7F; // 127

inline constexpr FieldScaling aze0_cabin_temp_scaling = {false, 5.0f/9.0f, -32.0f * (5.0f/9.0f)};
inline constexpr uint8_t aze0_cabin_temp_sentinel = 20; // 0x14

#endif // CAN_LEAF_AZE0_H

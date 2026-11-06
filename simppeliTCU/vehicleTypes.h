#ifndef VEHICLE_TYPES_H
#define VEHICLE_TYPES_H

#include <stdint.h>

// Common vehicle states shared across components
enum class ChargerState { 
    IDLE, 
    CHARGING, 
    FINISHED, 
    INTERRUPTED, 
    WAITING 
};

// HVAC Ventilation modes (from OVMS implementation)
enum class VentilationMode {
    OFF,
    FACE,
    FACE_FEET,
    FEET,
    WINDSCREEN_FEET,
    WINDSCREEN,
    UNKNOWN
};

// GPS data parsed from AV-CAN (0x62C) multi-frame message
struct LeafGpsData {
    double latitude;
    double longitude;
    uint8_t lat_deg;
    uint8_t lat_min;
    uint16_t lat_ms;
    uint8_t lon_deg;
    uint8_t lon_min;
    uint16_t lon_ms;
};

#endif

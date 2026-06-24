#ifndef VEHICLE_TYPES_H
#define VEHICLE_TYPES_H

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

#endif

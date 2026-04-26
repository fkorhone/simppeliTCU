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

#endif

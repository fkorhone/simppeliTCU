#include "canLeafZE1.h"
#include <cstring>

// Message parser dispatch
struct MessageParser {
    uint32_t id;
    uint8_t expected_len;
    void (*handler)(const uint8_t* data, uint8_t len);
};

// Helper macro to create parser entries from CANMessage objects
#define CAN_PARSER(msg, handler) { (msg).identifier, decltype(msg)::data_size, handler }

// Message-specific handlers
void handleSOCMessage(const uint8_t* data, uint8_t len) {
    (void)len;
    float soc = extractScaledValue(data, soc_field, soc_scaling);
    handleRawSOC(soc);
}

void handleTempMessage(const uint8_t* data, uint8_t len) {
    float temp = extractScaledValue(data, cabin_temp_field, cabin_temp_scaling);
    handleCabinTemp(temp);
}

void handleCarAwakeMessage(const uint8_t* data, uint8_t len) {
    handleCarAwake();
}

void handleChargerMessage(const uint8_t* data, uint8_t len) {
    uint8_t cStatus = static_cast<uint8_t>(extractBits(data, cStatus_field));
    bool qc_state = extractBool(data, qc_state_field);
    bool vg_state = (extractBits(data, vg_state_field) == 0x09);
    float chargeVoltage = extractScaledValue(data, chargeVolt_field, voltage_scaling);

    bool isCharging = false;
    ChargerState state = ChargerState::IDLE;

    switch (cStatus) {
        case 0x01:
            if (qc_state && !vg_state) { state = ChargerState::CHARGING; isCharging = true; }
            break;
        case 0x04:
            if (chargeVoltage > 0) { state = ChargerState::CHARGING; isCharging = true; }
            else { state = ChargerState::INTERRUPTED; }
            break;
        case 0x02:
            state = ChargerState::FINISHED;
            break;
        case 0x0c:
            state = ChargerState::WAITING;
            break;
        default:
            break;
    }

    handleChargerStatus(isCharging, state);
}

void handleHVACSetpointMessage(const uint8_t* data, uint8_t len) {
    (void)len;
    float setpoint = extractScaledValue(data, hvac_setpoint_field, setpoint_scaling);
    handleHVACSetpoint(setpoint);
}

VentilationMode decodeVentilationMode(uint8_t byte2) {
    // OVMS implementation: byte 2 of 0x54B message contains ventilation mode
    switch (byte2) {
        case 0x80: return VentilationMode::OFF;
        case 0x88: return VentilationMode::FACE;
        case 0x90: return VentilationMode::FACE_FEET;
        case 0x98: return VentilationMode::FEET;
        case 0xA0: return VentilationMode::WINDSCREEN_FEET;
        case 0xA8: return VentilationMode::WINDSCREEN;
        default: return VentilationMode::UNKNOWN;
    }
}

void handleHVACMessage(const uint8_t* data, uint8_t len) {
    (void)len;
    // Extract new metrics
    bool heating = extractBool(data, hvac_bit0_field);
    bool cooling = extractBool(data, hvac_bit4_field) || extractBool(data, hvac_bit5_field);
    float fanSpeed = extractScaledValue(data, fan_speed_field, fan_speed_scaling);
    
    // Extract ventilation mode from byte 2
    VentilationMode ventMode = decodeVentilationMode(data[2]);
    
    // HVAC enabled logic from OVMS
    bool isOn = (data[1] != 0x08 && data[1] != 0x04);
    
    handleHVACStatus(isOn);
    handleFanSpeed(fanSpeed);
    handleHeatingMode(heating, cooling);
    handleVentilationMode(ventMode);
}



void handleDoorsAndLocksMessage(const uint8_t* data, uint8_t len) {
    bool trunk = extractBool(data, door_trunk_field);
    bool rr = extractBool(data, door_rr_field);
    bool rl = extractBool(data, door_rl_field);
    bool fr = extractBool(data, door_fr_field);
    bool fl = extractBool(data, door_fl_field);
    bool locked = extractBool(data, locked_field);

    handleDoorStatus(fl, fr, rl, rr, trunk);
    handleLockStatus(locked);
}

// Message parser dispatch table
static const MessageParser parsers[] = {
    CAN_PARSER(raw_soc_readout, handleSOCMessage),
    CAN_PARSER(cabin_temp_readout, handleTempMessage),
    CAN_PARSER(car_awake_readout, handleCarAwakeMessage),
    CAN_PARSER(charger_status_readout, handleChargerMessage),
    CAN_PARSER(hvac_status_readout, handleHVACMessage),
    CAN_PARSER(hvac_setpoint_readout, handleHVACSetpointMessage),
    CAN_PARSER(doors_and_locks_readout, handleDoorsAndLocksMessage),
};

// Can message reception handler, to be called when a CAN message is received
void handleReceivedMessage(uint32_t identifier, uint8_t* data, uint8_t len) {
    for (const auto& p : parsers) {
        if (p.id == identifier && len >= p.expected_len) {
            p.handler(data, len);
            return;
        }
    }
}

// Sequence State Machine
extern bool carIsAwake;
CanSequence activeSequence = CanSequence::NONE;

enum class SeqPhase {
    IDLE,
    WAKING,
    WAKE_WAIT,
    RUN_STEP
};

static SeqPhase currentPhase = SeqPhase::IDLE;
static int wakeAttempts = 0;
static int stepIndex = 0;
static int repeatCount = 0;
static unsigned long nextCANEventTime = 0;

struct SequenceStep {
    uint32_t id;
    const uint8_t* data;
    uint8_t len;
    int repeats;
    int intervalMs;
    int delayAfterMs;
    SequencePayloadModifier modifier;
};

#define MSG_STEP(msg, reps, interv) { msg.identifier, msg.data, sizeof(msg.data), reps, interv, 0, nullptr }
#define MSG_DLY_STEP(msg, reps, interv, delay) { msg.identifier, msg.data, sizeof(msg.data), reps, interv, delay, nullptr }
#define MSG_MOD_STEP(msg, reps, interv, mod_fn) { msg.identifier, msg.data, sizeof(msg.data), reps, interv, 0, mod_fn }

static uint8_t internal_hvac_setpoint_byte = 0;

void setHVACTargetTemperature(float setpoint) {
    internal_hvac_setpoint_byte = setpoint > 0.0f ? static_cast<uint8_t>(setpoint * 2.0f) : 0;
}

static void hvacPayloadModifier(uint8_t* data, uint8_t len) {
    if (len >= 3) {
        data[2] = internal_hvac_setpoint_byte;
    }
}

static const SequenceStep seq_refresh[] = {
    MSG_DLY_STEP(hvac_init, 20, 100, 1000),
    MSG_STEP(idle_data, 20, 100)
};

static const SequenceStep seq_hvac_on[] = {
    MSG_STEP(hvac_init, 20, 100),
    MSG_MOD_STEP(hvac_on_data, 25, 100, hvacPayloadModifier),
    MSG_MOD_STEP(hvac_on_idle, 40, 100, hvacPayloadModifier),
    MSG_MOD_STEP(idle_data, 50, 100, hvacPayloadModifier)
};

static const SequenceStep seq_hvac_off[] = {
    MSG_STEP(hvac_init, 10, 100),
    MSG_STEP(interrupt_data, 9, 100),
    MSG_STEP(hvac_off_data, 8, 100),
    MSG_STEP(hvac_init, 4, 100),
    MSG_STEP(idle_data, 8, 100)
};

static const SequenceStep seq_charge_on[] = {
    MSG_STEP(start_charge_data, 20, 100),
    MSG_STEP(idle_data, 8, 100)
};

static const SequenceStep seq_unlock_doors[] = {
    MSG_STEP(unlock_doors_data, 1, 100),
    MSG_STEP(idle_data, 8, 100)
};

static const SequenceStep seq_lock_doors[] = {
    MSG_STEP(lock_doors_data, 1, 100),
    MSG_STEP(idle_data, 8, 100)
};

struct SequenceDef {
    CanSequence seq;
    const SequenceStep* steps;
    int length;
};

static const SequenceDef sequence_map[] = {
    { CanSequence::REFRESH, seq_refresh, sizeof(seq_refresh)/sizeof(seq_refresh[0]) },
    { CanSequence::HVAC_ON, seq_hvac_on, sizeof(seq_hvac_on)/sizeof(seq_hvac_on[0]) },
    { CanSequence::HVAC_OFF, seq_hvac_off, sizeof(seq_hvac_off)/sizeof(seq_hvac_off[0]) },
    { CanSequence::CHARGE_ON, seq_charge_on, sizeof(seq_charge_on)/sizeof(seq_charge_on[0]) },
    { CanSequence::UNLOCK_DOORS, seq_unlock_doors, sizeof(seq_unlock_doors)/sizeof(seq_unlock_doors[0]) },
    { CanSequence::LOCK_DOORS, seq_lock_doors, sizeof(seq_lock_doors)/sizeof(seq_lock_doors[0]) }
};

void startSequence(CanSequence seq, unsigned long currentTimeMs) {
    if (seq == CanSequence::NONE) return;
    if (activeSequence != CanSequence::NONE) return; // Prevent overwriting an active sequence
    resetCanLogTimestamps();
    
    activeSequence = seq;
    currentPhase = SeqPhase::WAKING;
    wakeAttempts = 0;
    stepIndex = 0;
    repeatCount = 0;
    carIsAwake = false;
    nextCANEventTime = currentTimeMs;
}

CanSeqResult manageCANSequence(unsigned long currentTimeMs) {
    if (activeSequence == CanSequence::NONE) return CanSeqResult::IDLE;
    if ((long)(currentTimeMs - nextCANEventTime) < 0) return CanSeqResult::PROCESSING;

    switch (currentPhase) {
        case SeqPhase::WAKING:
            if (carIsAwake || wakeAttempts >= 130) {
                CanSeqResult res;
                if (carIsAwake) res = CanSeqResult::WAKE_SUCCESS;
                else res = CanSeqResult::WAKE_TIMEOUT;
                currentPhase = SeqPhase::WAKE_WAIT;
                nextCANEventTime = currentTimeMs + 50; 
                return res;
            } else {
                sendCAN(wakeup_data);
                wakeAttempts++;
                nextCANEventTime = currentTimeMs + 15;
            }
            break;
            
        case SeqPhase::WAKE_WAIT:
            currentPhase = SeqPhase::RUN_STEP;
            stepIndex = 0;
            repeatCount = 0;
            nextCANEventTime = currentTimeMs;
            break;
            
        case SeqPhase::RUN_STEP: {
            bool done = false;
            const SequenceStep* currentSeqArray = nullptr;
            int currentSeqLen = 0;

            for (const auto& def : sequence_map) {
                if (def.seq == activeSequence) {
                    currentSeqArray = def.steps;
                    currentSeqLen = def.length;
                    break;
                }
            }

            if (!currentSeqArray) {
                done = true;
            }

            if (!done && stepIndex < currentSeqLen && currentSeqArray != nullptr) {
                const SequenceStep& step = currentSeqArray[stepIndex];
                
                if (step.id != 0 && step.data != nullptr) {
                    uint8_t buffer[8];
                    uint8_t safeLen = step.len > sizeof(buffer) ? sizeof(buffer) : step.len;
                    memcpy(buffer, step.data, safeLen);
                    if (step.modifier) {
                        step.modifier(buffer, safeLen);
                    }
                    sendCAN(step.id, buffer, safeLen);
                }
                
                if (++repeatCount >= step.repeats) {
                    stepIndex++;
                    repeatCount = 0;
                    const uint32_t nextDelayMs =
                        (step.delayAfterMs > step.intervalMs) ? step.delayAfterMs : step.intervalMs;
                    nextCANEventTime = currentTimeMs + nextDelayMs;
                } else {
                    nextCANEventTime = currentTimeMs + step.intervalMs;
                }
            } else {
                done = true;
            }

            if (done) {
                activeSequence = CanSequence::NONE;
                currentPhase = SeqPhase::IDLE;
                return CanSeqResult::SEQUENCE_FINISHED;
            }
            break;
        }
    }
    return CanSeqResult::PROCESSING;
}

void manageTCUDTCPrevention(unsigned long currentTimeMs) {
    static bool wasCanActive = false;
    static unsigned long offStartTime = 0;

    // Consider CAN bus active if we received a message (other than our own 0x56E) in the last 2 seconds
    bool canActive = (currentTimeMs - lastCanMessageMillis < 2000); 

    if (canActive) {
        if (!wasCanActive) {
            wasCanActive = true;
        }
        // If no 0x56E message has been sent in the last 120ms, act as a watchdog and send heartbeat
        if (currentTimeMs - last56EMessageSentMillis >= 120) {
            sendCAN(hvac_init);
        }
    } else {
        if (wasCanActive) {
            offStartTime = currentTimeMs;
            wasCanActive = false;
        }
        
        // "When vehicle is turning OFF, last few seconds of CAN bus activity"
        // If the CAN bus just went inactive, send 0x86 a few times to emulate turning OFF
        if (offStartTime != 0 && (currentTimeMs - offStartTime < 2000)) {
            if (currentTimeMs - last56EMessageSentMillis >= 120) {
                sendCAN(idle_data);
            }
        }
    }
}

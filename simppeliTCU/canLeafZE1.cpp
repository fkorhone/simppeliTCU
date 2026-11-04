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
    (void)len;
    uint8_t cStatus = static_cast<uint8_t>(extractBits(data, cStatus_field));
    bool qc_state = extractBool(data, qc_state_field);
    bool vg_state = (extractBits(data, vg_state_field) == CHARGER_VG_STATE_ACTIVE);
    float chargeVoltage = extractScaledValue(data, chargeVolt_field, voltage_scaling);

    bool isCharging = false;
    ChargerState state = ChargerState::IDLE;

    switch (cStatus) {
        case CHARGER_STATUS_QC:
            if (qc_state && !vg_state) { state = ChargerState::CHARGING; isCharging = true; }
            break;
        case CHARGER_STATUS_AC:
            if (chargeVoltage > 0) { state = ChargerState::CHARGING; isCharging = true; }
            else { state = ChargerState::INTERRUPTED; }
            break;
        case CHARGER_STATUS_FINISHED:
            state = ChargerState::FINISHED;
            break;
        case CHARGER_STATUS_WAITING:
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
        case VENT_MODE_OFF:             return VentilationMode::OFF;
        case VENT_MODE_FACE:            return VentilationMode::FACE;
        case VENT_MODE_FACE_FEET:       return VentilationMode::FACE_FEET;
        case VENT_MODE_FEET:            return VentilationMode::FEET;
        case VENT_MODE_WINDSCREEN_FEET: return VentilationMode::WINDSCREEN_FEET;
        case VENT_MODE_WINDSCREEN:      return VentilationMode::WINDSCREEN;
        default:                        return VentilationMode::UNKNOWN;
    }
}

void handleHVACMessage(const uint8_t* data, uint8_t len) {
    (void)len;
    // Extract new metrics
    bool heating = extractBool(data, hvac_bit0_field);
    bool cooling = extractBool(data, hvac_bit4_field) || extractBool(data, hvac_bit5_field);
    float fanSpeed = extractScaledValue(data, fan_speed_field, fan_speed_scaling);
    
    // Extract ventilation mode from byte 2 when present
    VentilationMode ventMode = (len >= 3)
        ? decodeVentilationMode(static_cast<uint8_t>(extractBits(data, ventilation_mode_field)))
        : VentilationMode::UNKNOWN;
    
    // HVAC enabled logic from OVMS
    uint8_t hvacState = static_cast<uint8_t>(extractBits(data, hvac_state_field));
    bool isOn = (hvacState != HVAC_STATE_OFF_2 && hvacState != HVAC_STATE_OFF_1);
    
    handleHVACStatus(isOn);
    handleFanSpeed(fanSpeed);
    handleHeatingMode(heating, cooling);
    handleVentilationMode(ventMode);
}

void handleDoorsAndLocksMessage(const uint8_t* data, uint8_t len) {
    (void)len;
    bool trunk = extractBool(data, door_trunk_field);
    bool rr = extractBool(data, door_rr_field);
    bool rl = extractBool(data, door_rl_field);
    bool fr = extractBool(data, door_fr_field);
    bool fl = extractBool(data, door_fl_field);
    bool locked = extractBool(data, locked_field);

    handleDoorStatus(fl, fr, rl, rr, trunk);
    handleLockStatus(locked);
}

enum class SeqPhase {
    IDLE,
    WAKING,
    WAKE_WAIT,
    RUN_STEP
};

CanSequence activeSequence = CanSequence::NONE;
static SeqPhase currentPhase = SeqPhase::IDLE;

static uint8_t internal_av_seq_nibble = AV_SEQ_MIN;
static bool nag_popup_armed = false;
static bool nag_popup_pending = false;
static bool car_in_reverse = false;

static bool isAVBootInitFrame(const uint8_t* data) {
    return extractBits(data, av_frame_type_field) == AV_FRAME_TYPE_SINGLE &&
           extractBits(data, av_seq_nibble_field) == AV_SEQ_BOOT_INIT;
}

static void triggerNagScreenPopup() {
    nag_popup_armed = false;
    if (car_in_reverse) {
        nag_popup_pending = true;
        return;
    }
    nag_popup_pending = false;
    handleNagScreenPopup();
}

void handleShifterMessage(const uint8_t* data, uint8_t len) {
    (void)len;
    uint8_t shifterState = static_cast<uint8_t>(extractBits(data, shifter_state_field));
    if (shifterState == SHIFTER_STATE_TRANSITION) {
        return;
    }
    if (shifterState == SHIFTER_STATE_REVERSE) {
        car_in_reverse = true;
        if (activeSequence == CanSequence::DISMISS_NAG) {
            activeSequence = CanSequence::NONE;
            currentPhase = SeqPhase::IDLE;
            nag_popup_pending = true;
        }
    } else if (car_in_reverse) {
        car_in_reverse = false;
        if (nag_popup_pending) {
            triggerNagScreenPopup();
        }
    }
}

void handleSteeringWheelButtonMessage(const uint8_t* data, uint8_t len) {
    (void)len;
    // Track rolling sequence counter (1..7) from single-frame 0x683 messages (ignore 0xE0 ACK and 0xF0 keep-alive)
    if (extractBits(data, av_frame_type_field) != AV_FRAME_TYPE_SINGLE) {
        return;
    }
    uint8_t seq = static_cast<uint8_t>(extractBits(data, av_seq_nibble_field));
    if (seq == AV_SEQ_BOOT_INIT) {
        internal_av_seq_nibble = AV_SEQ_MIN;
        nag_popup_armed = true;
        nag_popup_pending = false;
        car_in_reverse = false;
    } else if (seq <= AV_SEQ_MAX) {
        internal_av_seq_nibble = seq;
    }
}

void handleAVDisplayStatusMessage(const uint8_t* data, uint8_t len) {
    (void)len;
    if (isAVBootInitFrame(data)) {
        nag_popup_armed = true;
        return;
    }
    if (nag_popup_armed &&
        extractBits(data, av_header_byte_field) == AV_FIRST_FRAME_HEADER &&
        extractBits(data, av_ff_seq_byte_field) == AV_DISPLAY_NAG_READY_SEQ &&
        extractBits(data, av_ff_cmd_field) == AV_DISPLAY_STATUS_CMD) {
        triggerNagScreenPopup();
    }
}

void handleAVPopupStatusMessage(const uint8_t* data, uint8_t len) {
    (void)len;
    if (isAVBootInitFrame(data)) {
        nag_popup_armed = true;
        return;
    }
    if (nag_popup_armed &&
        extractBits(data, av_header_byte_field) == AV_FIRST_FRAME_HEADER &&
        extractBits(data, av_ff_len_byte_field) == AV_POPUP_STATUS_LEN &&
        extractBits(data, av_ff_cmd_field) == AV_DISPLAY_STATUS_CMD) {
        triggerNagScreenPopup();
    }
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
    CAN_PARSER(shifter_status_readout, handleShifterMessage),
    CAN_PARSER(steering_wheel_button_readout, handleSteeringWheelButtonMessage),
    CAN_PARSER(av_display_status_readout, handleAVDisplayStatusMessage),
    CAN_PARSER(av_popup_status_readout, handleAVPopupStatusMessage),
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
#define WAIT_STEP(delay) { 0, nullptr, 0, 1, delay, 0, nullptr }

static uint8_t internal_hvac_setpoint_byte = 0;

void setHVACTargetTemperature(float setpoint) {
    internal_hvac_setpoint_byte = setpoint > 0.0f ? static_cast<uint8_t>(setpoint * HVAC_CMD_SETPOINT_SCALE) : 0;
}

static void hvacPayloadModifier(uint8_t* data, uint8_t len) {
    if (len > HVAC_CMD_SETPOINT_BYTE) {
        data[HVAC_CMD_SETPOINT_BYTE] = internal_hvac_setpoint_byte;
    }
}

static void avSeqPayloadModifier(uint8_t* data, uint8_t len) {
    if (len > AV_CMD_SEQ_BYTE) {
        internal_av_seq_nibble = (internal_av_seq_nibble % AV_SEQ_MAX) + AV_SEQ_MIN;
        data[AV_CMD_SEQ_BYTE] = static_cast<uint8_t>(internal_av_seq_nibble << AV_SEQ_NIBBLE_SHIFT);
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

static const SequenceStep seq_dismiss_nag[] = {
    WAIT_STEP(1500),
    MSG_MOD_STEP(steering_wheel_button_phone_press, 1, 100, avSeqPayloadModifier),
    MSG_MOD_STEP(steering_wheel_button_phone_press, 1, 50, avSeqPayloadModifier),
    MSG_MOD_STEP(steering_wheel_button_phone_release, 1, 50, avSeqPayloadModifier)
};

struct SequenceDef {
    CanSequence seq;
    const SequenceStep* steps;
    int length;
    bool wakeUpCar;
};

static const SequenceDef sequence_map[] = {
    { CanSequence::REFRESH, seq_refresh, sizeof(seq_refresh)/sizeof(seq_refresh[0]), true },
    { CanSequence::HVAC_ON, seq_hvac_on, sizeof(seq_hvac_on)/sizeof(seq_hvac_on[0]), true },
    { CanSequence::HVAC_OFF, seq_hvac_off, sizeof(seq_hvac_off)/sizeof(seq_hvac_off[0]), true },
    { CanSequence::CHARGE_ON, seq_charge_on, sizeof(seq_charge_on)/sizeof(seq_charge_on[0]), true },
    { CanSequence::UNLOCK_DOORS, seq_unlock_doors, sizeof(seq_unlock_doors)/sizeof(seq_unlock_doors[0]), true },
    { CanSequence::LOCK_DOORS, seq_lock_doors, sizeof(seq_lock_doors)/sizeof(seq_lock_doors[0]), true },
    { CanSequence::DISMISS_NAG, seq_dismiss_nag, sizeof(seq_dismiss_nag)/sizeof(seq_dismiss_nag[0]), false }
};

static const SequenceDef* findSequenceDef(CanSequence seq) {
    for (const auto& def : sequence_map) {
        if (def.seq == seq) {
            return &def;
        }
    }
    return nullptr;
}

void startSequence(CanSequence seq, unsigned long currentTimeMs) {
    if (activeSequence != CanSequence::NONE) return; // Prevent overwriting an active sequence
    const SequenceDef* def = findSequenceDef(seq);
    if (!def) return;

    resetCanLogTimestamps();
    activeSequence = seq;
    currentPhase = def->wakeUpCar ? SeqPhase::WAKING : SeqPhase::RUN_STEP;
    wakeAttempts = 0;
    stepIndex = 0;
    repeatCount = 0;
    if (def->wakeUpCar) {
        carIsAwake = false;
    }
    nextCANEventTime = currentTimeMs;
}

CanSeqResult manageCANSequence(unsigned long currentTimeMs) {
    if (activeSequence == CanSequence::NONE) return CanSeqResult::IDLE;
    if ((long)(currentTimeMs - nextCANEventTime) < 0) return CanSeqResult::PROCESSING;

    switch (currentPhase) {
        case SeqPhase::WAKING:
            if (carIsAwake || wakeAttempts >= 130) {
                CanSeqResult res = carIsAwake ? CanSeqResult::WAKE_SUCCESS : CanSeqResult::WAKE_TIMEOUT;
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
            const SequenceDef* def = findSequenceDef(activeSequence);
            if (!def || stepIndex >= def->length) {
                activeSequence = CanSequence::NONE;
                currentPhase = SeqPhase::IDLE;
                return CanSeqResult::SEQUENCE_FINISHED;
            }

            const SequenceStep& step = def->steps[stepIndex];
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
            break;
        }
    }
    return CanSeqResult::PROCESSING;
}

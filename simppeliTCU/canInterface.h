#ifndef CAN_INTERFACE_H
#define CAN_INTERFACE_H

#include <stddef.h>
#include <inttypes.h>

// Field definition: bit_offset = byte_idx * 8 + bit_in_byte (0-63, bit 0 = MSB of byte)
struct CANField {
    uint16_t bit_offset;
    uint8_t bit_size;
};

struct FieldScaling {
    bool is_signed;
    float scale;
    float offset;
};

// Extract raw bits from data buffer (MSB-first within each byte)
inline uint64_t extractBits(const uint8_t* data, const CANField& field) {
    uint64_t value = 0;
    uint16_t start_bit = field.bit_offset;
    for (uint8_t i = 0; i < field.bit_size; i++) {
        uint16_t bit_pos = start_bit + i;
        uint16_t byte_idx = bit_pos / 8;
        uint8_t bit_in_byte = bit_pos % 8;
        uint8_t bit_val = (data[byte_idx] >> (7 - bit_in_byte)) & 0x01;
        value = (value << 1) | bit_val;
    }
    return value;
}

// Extract scaled float value
inline float extractScaledValue(const uint8_t* data, const CANField& field, const FieldScaling& scaling) {
    uint64_t raw = extractBits(data, field);
    if (scaling.is_signed) {
        int64_t signed_val = static_cast<int64_t>(raw << (64 - field.bit_size)) >> (64 - field.bit_size);
        return static_cast<float>(signed_val) * scaling.scale + scaling.offset;
    }
    return static_cast<float>(raw) * scaling.scale + scaling.offset;
}

// Extract boolean (single bit)
inline bool extractBool(const uint8_t* data, const CANField& field) {
    return extractBits(data, field) != 0;
}

template<size_t N>
struct CANMessage {
  uint32_t identifier;
  static constexpr uint8_t data_size = N;
  uint8_t data[N > 0 ? N : 1]; // To avoid illegal zero-length arrays when N=0
};

// Can implementation interface (definitions platform specific):
void sendCAN(uint32_t id, const uint8_t* data, uint8_t len);

template <size_t N>
void sendCAN(const CANMessage<N> &message) {
  sendCAN(message.identifier, message.data, N);
}

template<size_t N>
bool receivedMessageIs(uint32_t identifier, unsigned int len, const CANMessage<N> &message) {
  return message.identifier == identifier && len >= N;
}

// Application specific message handler:
void handleReceivedMessage(uint32_t identifier, uint8_t* data, uint8_t len);

void setupCAN();
void readAndHandleCANMessage();

void resetCanLogTimestamps();

#endif
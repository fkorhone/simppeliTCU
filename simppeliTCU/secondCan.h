#ifndef SECOND_CAN_H
#define SECOND_CAN_H

#include <stddef.h>
#include <inttypes.h>

// Pin configuration for the second CAN interface (MCP2515 over SPI).
// Values are defined in hardware.h and passed to setupSecondCAN() at runtime.
struct SecondCanPins {
    uint8_t cs;
    uint8_t sclk;
    uint8_t mosi;
    uint8_t miso;
    uint8_t rst;
    uint8_t intPin;
};

// Initializes SPI and the MCP2515. Returns true on success.
bool setupSecondCAN(const SecondCanPins& pins);

// Sends a standard 11-bit CAN frame. Returns true on success.
bool sendSecondCAN(uint32_t id, const uint8_t* data, uint8_t len);

// Reads one pending frame from the MCP2515 RX buffers.
// Returns true if a frame was read into id/data/len.
bool readSecondCANMessage(uint32_t* id, uint8_t* data, uint8_t* len);

#endif // SECOND_CAN_H

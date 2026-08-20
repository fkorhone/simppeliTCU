#include "secondCan.h"
#include "featureFlags.h"

#if USE_SECOND_CAN

#include <Arduino.h>
#include <SPI.h>
#include <mcp2515.h>
#include "hardware.h"

#define CAN_FRAME_MAX_DLEN 8

// Constructed in setupSecondCAN() once the runtime pin configuration is known
static MCP2515* mcp2515 = nullptr;

bool setupSecondCAN(const SecondCanPins& pins) {
  pinMode(pins.rst, OUTPUT);
  digitalWrite(pins.rst, HIGH);
  delay(10);
  digitalWrite(pins.rst, LOW);
  delay(10);
  digitalWrite(pins.rst, HIGH);
  delay(10);

  SPI.begin(pins.sclk, pins.miso, pins.mosi, pins.cs);

  mcp2515 = new MCP2515(pins.cs, MCP2515_SPI_CLOCK, &SPI);
  mcp2515->reset();
  if (mcp2515->setBitrate(CAN_500KBPS) == MCP2515::ERROR_OK) {
    mcp2515->setNormalMode();
    return true;
  }
  return false;
}

bool sendSecondCAN(uint32_t id, const uint8_t* data, uint8_t len) {
  if (mcp2515 == nullptr) return false;
  if (len > CAN_FRAME_MAX_DLEN) len = CAN_FRAME_MAX_DLEN;

  struct can_frame msg;
  msg.can_id = id;
  msg.can_dlc = len;
  for (int i = 0; i < len; i++) {
    msg.data[i] = data[i];
  }
  return mcp2515->sendMessage(&msg) == MCP2515::ERROR_OK;
}

bool readSecondCANMessage(uint32_t* id, uint8_t* data, uint8_t* len) {
  if (mcp2515 == nullptr) return false;

  struct can_frame msg;
  if (mcp2515->readMessage(&msg) != MCP2515::ERROR_OK) return false;

  *id = msg.can_id;
  *len = msg.can_dlc > CAN_FRAME_MAX_DLEN ? CAN_FRAME_MAX_DLEN : msg.can_dlc;
  for (int i = 0; i < *len; i++) {
    data[i] = msg.data[i];
  }
  return true;
}

#endif // USE_SECOND_CAN

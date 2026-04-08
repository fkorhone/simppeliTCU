#ifndef CAN_INTERFACE_H
#define CAN_INTERFACE_H

#include <stddef.h>
#include <inttypes.h>

template<size_t N>
struct CANMessage {
  uint32_t identifier;
  uint8_t data[N > 0 ? N : 1]; // To avoid illegal zero-length arrays when N=0
};

// Can implementation interface (definitions platform specific):
void waitCAN(int delayMs);

void sendCAN(uint32_t id, const uint8_t* data, uint8_t len);

template<size_t N>
void sendCAN(const CANMessage<N> &message) {
  sendCAN(message.identifier, message.data, N);
}

template<size_t N>
void sendCAN(const CANMessage<N> &message, int repeat, int delayMs) {
  for (int i = 0; i < repeat; i++) {
    sendCAN(message.identifier, message.data, N);
    waitCAN(delayMs);
  }
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
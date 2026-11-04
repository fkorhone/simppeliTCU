#include <Arduino.h>
#include <cstdio>
#include "driver/twai.h"
#include "canInterface.h"
#include "featureFlags.h"
#include "hardware.h"

#if USE_SECOND_CAN
#include "secondCan.h"

// Second CAN (MCP2515) pin configuration from hardware.h
static const SecondCanPins secondCanPins = {
  MCP2515_CS, MCP2515_SCLK, MCP2515_MOSI, MCP2515_MISO, MCP2515_RST, MCP2515_INT
};
#endif

static constexpr int MAX_CAN_FRAMES_PER_POLL = 4;
static constexpr uint32_t TWAI_RX_QUEUE_LEN = 32;

unsigned long logStartTime = 0;

void print_can_message(const char* prefix, const twai_message_t &message) {
  unsigned long message_time = millis() - logStartTime;
  char buf[80];
  int pos = snprintf(buf, sizeof(buf), "%s%lu.%03lu 0x%lX ",
                     prefix,
                     message_time / 1000UL,
                     message_time % 1000UL,
                     (unsigned long)message.identifier);
  if (pos < 0 || pos >= (int)sizeof(buf)) return;

  uint8_t dlc = message.data_length_code > 8 ? 8 : message.data_length_code;
  for (uint8_t d = 0; d < dlc; d++) {
    int n = snprintf(buf + pos, sizeof(buf) - pos, " 0x%02X ", message.data[d]);
    if (n < 0 || pos + n >= (int)sizeof(buf)) break;
    pos += n;
  }
  if (pos + 2 < (int)sizeof(buf)) {
    buf[pos++] = '\r';
    buf[pos++] = '\n';
    buf[pos] = '\0';
  }

  if (Serial.availableForWrite() >= pos) {
    Serial.write((const uint8_t*)buf, pos);
  }
}

// Helper function to send CAN message
void sendCAN(uint32_t id,const uint8_t* data, uint8_t len) {
  twai_message_t message;
  message.identifier = id;
  message.extd = 0;
  message.rtr = 0;
  message.data_length_code = len;
  for (int i = 0; i < len; i++) {
    message.data[i] = data[i];
  }

  const char* prefix = (twai_transmit(&message, pdMS_TO_TICKS(10)) == ESP_OK) ? "> " : "! ";
  print_can_message(prefix, message);

#if USE_SECOND_CAN
  if (!sendSecondCAN(id, data, len)) {
    static const char errMsg[] = "! Failed to send message on second CAN\r\n";
    if (Serial.availableForWrite() >= (int)(sizeof(errMsg) - 1)) {
      Serial.write((const uint8_t*)errMsg, sizeof(errMsg) - 1);
    }
  }
#endif
}

void setupCAN() {
  twai_timing_config_t t_config = TWAI_TIMING_CONFIG_500KBITS();
  twai_general_config_t g_config = TWAI_GENERAL_CONFIG_DEFAULT((gpio_num_t)CAN_TX_PIN, (gpio_num_t)CAN_RX_PIN, TWAI_MODE_NORMAL);
  g_config.rx_queue_len = TWAI_RX_QUEUE_LEN;
  twai_filter_config_t f_config = TWAI_FILTER_CONFIG_ACCEPT_ALL();

  if (twai_driver_install(&g_config, &t_config, &f_config) == ESP_OK) {
    Serial.println("CAN driver installed");
  } else {
    Serial.println("Failed to install CAN driver");
  }

  if (twai_start() == ESP_OK) {
    Serial.println("CAN driver started");
  } else {
    Serial.println("Failed to start CAN driver");
  }

#if USE_SECOND_CAN
  if (setupSecondCAN(secondCanPins)) {
    Serial.println("Second CAN initialized");
  } else {
    Serial.println("Failed to initialize second CAN");
  }
#else
  Serial.println("Second CAN disabled by configuration");
#endif
}

void readAndHandleCANMessage() {
  twai_message_t message;
  for (int i = 0; i < MAX_CAN_FRAMES_PER_POLL && twai_receive(&message, 0) == ESP_OK; i++) {
    handleReceivedMessage(message.identifier, message.data, message.data_length_code);
    print_can_message("< ", message);
  }

#if USE_SECOND_CAN
  uint32_t mcp_id;
  uint8_t mcp_data[8];
  uint8_t mcp_len;
  // Drain pending frames from the MCP2515 RX buffers (bounded so high bus load cannot starve loop)
  for (int i = 0; i < MAX_CAN_FRAMES_PER_POLL && readSecondCANMessage(&mcp_id, mcp_data, &mcp_len); i++) {
    handleReceivedMessage(mcp_id, mcp_data, mcp_len);

    // Print to serial mimicking TWAI print
    twai_message_t mock_msg;
    mock_msg.identifier = mcp_id;
    mock_msg.data_length_code = mcp_len;
    for (int j = 0; j < mcp_len; j++) {
      mock_msg.data[j] = mcp_data[j];
    }
    print_can_message("2< ", mock_msg);
  }
#endif
}

void resetCanLogTimestamps() {
  logStartTime = millis();
}

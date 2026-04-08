#include <Arduino.h>
#include "driver/twai.h"
#include "canInterface.h"

// --- CAN SETTINGS (Port B) ---
#define CAN_TX_PIN 7 
#define CAN_RX_PIN 6

unsigned long logStartTime = 0;

void waitCAN(int delayMs) {
    unsigned long startTime = millis();
    while (millis() - startTime < delayMs) {
      readAndHandleCANMessage();
    }
    delay(1);
}

void print_can_message(twai_message_t &message) {
      unsigned long message_time = millis() - logStartTime;
      Serial.print(message_time/1000.0, 3);
      Serial.print(" 0x");
      Serial.print(message.identifier, HEX);
      Serial.print(" ");
      for (int d = 0; d < message.data_length_code; d++) {
        Serial.print(" 0x");
        if (message.data[d] < 0x10) Serial.print("0");
        Serial.print(message.data[d], HEX);
        Serial.print(" ");
      }
      Serial.println();
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

  if(twai_transmit(&message, pdMS_TO_TICKS(10)) == ESP_OK) {
    Serial.print("> ");
  } else {
    Serial.print("! ");
  }
  print_can_message(message);
}

void setupCAN() {
  twai_timing_config_t t_config = TWAI_TIMING_CONFIG_500KBITS();
  twai_general_config_t g_config = TWAI_GENERAL_CONFIG_DEFAULT((gpio_num_t)CAN_TX_PIN, (gpio_num_t)CAN_RX_PIN, TWAI_MODE_NORMAL);
  twai_filter_config_t f_config = TWAI_FILTER_CONFIG_ACCEPT_ALL();

  if (twai_driver_install(&g_config, &t_config, &f_config) == ESP_OK) {
    Serial.println("CAN driver installed");
  } else {
    Serial.println("Failed to install CAN driver");
    return;
  }

  if (twai_start() == ESP_OK) {
    Serial.println("CAN driver started");
  } else {
    Serial.println("Failed to start CAN driver");
  }
}

void readAndHandleCANMessage() {
  twai_message_t message;
  if (twai_receive(&message, 0) == ESP_OK) { // 0 = do not wait, read only if data is available
    handleReceivedMessage(message.identifier, message.data, message.data_length_code);
    Serial.print("< ");
    print_can_message(message);
  }
}

void resetCanLogTimestamps() {
  logStartTime = millis();
}

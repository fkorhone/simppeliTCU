#include "driver/twai.h"

// Lilygo T-2CAN Portti B (Native TWAI)
// HUOM: Jos et saa dataa, nämä voivat olla toisinpäin (TX=7, RX=6). 
// Kokeile vaihtaa päikseen, kumpikaan järjestys ei riko mitään!
#define CAN_TX_PIN 7 
#define CAN_RX_PIN 6



void setup() {
  Serial.begin(115200);
  Serial.println("Käynnistetään Portin B TWAI-kuunteluoppilas (Listen Only)...");

  // Asetetaan väylänopeus 500 kbps (Leaf)
  twai_timing_config_t t_config = TWAI_TIMING_CONFIG_500KBITS();
  
  // Asetetaan pinnit 6 ja 7, ja LISTEN ONLY -tila
  twai_general_config_t g_config = TWAI_GENERAL_CONFIG_DEFAULT(
    (gpio_num_t)CAN_TX_PIN, 
    (gpio_num_t)CAN_RX_PIN, 
    TWAI_MODE_LISTEN_ONLY
  );
  
  twai_filter_config_t f_config = TWAI_FILTER_CONFIG_ACCEPT_ALL();

  // Asennetaan ja käynnistetään ajuri
  if (twai_driver_install(&g_config, &t_config, &f_config) == ESP_OK) {
    if (twai_start() == ESP_OK) {
      Serial.println("TWAI käynnistetty Portissa B. Odotetaan viestejä...");
    }
  } else {
    Serial.println("Virhe ajurin asennuksessa!");
  }
}

void loop() {
  twai_message_t message;
  
  if (twai_receive(&message, pdMS_TO_TICKS(100)) == ESP_OK) {
    //if (message.identifier == 0x56E) {
      Serial.printf("ID: %X | Pituus: ", message.identifier);
      Serial.print(message.data_length_code);
      Serial.print(" | Data: ");
      
      for (int i = 0; i < message.data_length_code; i++) {
        Serial.printf("%02X ", message.data[i]);
      }
      Serial.println();
    //}
  }
}
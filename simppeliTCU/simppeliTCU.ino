#include "driver/twai.h"

// Lilygo T-2CAN Portti B (Native TWAI)
#define CAN_TX_PIN 7 
#define CAN_RX_PIN 6

// Määritellään Nissan Leafin maagiset tavut
uint8_t heat_on_data[4]  = {0x4E, 0x08, 0x00, 0x00};
uint8_t heat_off_data[4] = {0x86, 0x00, 0x00, 0x00};

void setup() {
  Serial.begin(115200);
  Serial.println("Käynnistetään TCU-simulaattori...");

  twai_timing_config_t t_config = TWAI_TIMING_CONFIG_500KBITS();
  
  // HUOM! Tila on nyt TWAI_MODE_NORMAL (ei enää Listen Only!)
  twai_general_config_t g_config = TWAI_GENERAL_CONFIG_DEFAULT(
    (gpio_num_t)CAN_TX_PIN, 
    (gpio_num_t)CAN_RX_PIN, 
    TWAI_MODE_NORMAL
  );
  
  twai_filter_config_t f_config = TWAI_FILTER_CONFIG_ACCEPT_ALL();

  if (twai_driver_install(&g_config, &t_config, &f_config) == ESP_OK) {
    if (twai_start() == ESP_OK) {
      Serial.println("TWAI käynnistetty NORMALI-tilassa.");
      Serial.println("Odotetaan 5 sekuntia ennen testin aloitusta...");
    }
  } else {
    Serial.println("Virhe ajurin asennuksessa!");
    while(1);
  }
  
  delay(5000); // 5 sekunnin varoaika ennen kuin aletaan pommittaa väylää
}

// Apufunktio viestin lähettämiseen
void send_tcu_command(uint8_t* data) {
  twai_message_t message;
  message.identifier = 0x56E;
  message.extd = 0;           // Käytetään standardia 11-bittistä ID:tä
  message.rtr = 0;            // Ei ole Remote Transmission Request
  message.data_length_code = 4; // ZE1-mallin 4 tavun pituus!
  
  for (int i = 0; i < 4; i++) {
    message.data[i] = data[i];
  }
  
  // Lähetetään viesti, timeout 10ms
  twai_transmit(&message, pdMS_TO_TICKS(10));
}

// Apufunktio herätysviestin lähettämiseen
void send_wakeup_ping() {
  twai_message_t message;
  message.identifier = 0x68C;
  message.extd = 0;
  message.rtr = 0;
  message.data_length_code = 1; 
  message.data[0] = 0x00;
  
  twai_transmit(&message, pdMS_TO_TICKS(10));
}

void loop() {
    delay(100); 
    send_wakeup_ping();
    delay(100); 
    send_wakeup_ping();
    delay(100);

  // 1. VAIHE: Laitetaan lämmitys päälle 10 sekunniksi
  Serial.println("LÄMMITYS PÄÄLLE! (Lähetetään komentoa 10 sekunnin ajan...)");
  unsigned long start_time = millis();
  
  while (millis() - start_time < 10000) {
    send_tcu_command(heat_on_data);
    delay(100); // Lähetetään viesti 100 ms välein (kuten aito TCU tekee)
  }

  // 2. VAIHE: Sammutetaan lämmitys ja annetaan auton nukahtaa
  Serial.println("LÄMMITYS POIS. (Lähetetään lepotila-komentoa...)");
  for (int i = 0; i < 20; i++) { 
    send_tcu_command(heat_off_data);
    delay(100); // Lähetetään 2 sekunnin ajan sammutusviestiä
  }

  // 3. VAIHE: Turvapysäytys
  Serial.println("Testi ohi! Releet pitäisi nyt naksua kiinni ja auton nukahtaa.");
  Serial.println("Pysäytetään ohjelma turvallisuussyistä.");
  
  while(1) {
    // Ikuinen luuppi. Resetoi ESP32, jos haluat ajaa testin uudestaan.
    delay(1000); 
  }
}

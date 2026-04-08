#include <WiFi.h>
#include <WebServer.h>
#include "driver/twai.h"
#include "configuration.h"
#include "canLeafZE1.h"
#include "ui.h"

// --- CAN SETTINGS (Port B) ---
#define CAN_TX_PIN 7 
#define CAN_RX_PIN 6


// State variables
bool wifiEnabled = false;
bool isHeating = false;
bool carIsAwake = false;
unsigned long wakeTime = 0;
// Values read from car
float currentSOC = -1.0;
float cabinTemp = -99.0;

void resetData() {
    currentSOC = -1.0;
    cabinTemp = -99.0;
}

void print_can_message(twai_message_t &message) {
      unsigned long message_time = millis() - wakeTime;
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
void sendCAN(uint32_t id, uint8_t* data, uint8_t len) {
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

void waitCan(int delayMs) {
    unsigned long startTime = millis();
    while (millis() - startTime < delayMs) {
      readAndHandleCANMessage();
    }
    delay(1);
}

void sendCAN(uint32_t id, uint8_t* data, uint8_t len, int repeat, int delayMs) {
  for (int i = 0; i < repeat; i++) {
    sendCAN(id, data, len);
    waitCan(delayMs);
  }
}

bool receivedMessageIs(twai_message_t &message, CANMessageReadout &readout) {
  return message.identifier == readout.identifier && message.data_length_code >= readout.length;
}

void readAndHandleCANMessage() {
  twai_message_t message;
  if (twai_receive(&message, 0) == ESP_OK) { // 0 = do not wait, read only if data is available
    if (receivedMessageIs(message, raw_soc_readout)) {
      uint16_t soc_raw = (message.data[0] << 2) | (message.data[1] >> 6);
      currentSOC = soc_raw / 10.0;
    }
    else if (receivedMessageIs(message, cabin_temp_readout)) {
      cabinTemp = (message.data[0] / 2.0) - 40.0;
    }
    else if (receivedMessageIs(message, car_awake_readout)) {
      carIsAwake = true;
    }
    Serial.print("< ");
    print_can_message(message);
  }
}

void wakeup() {
  Serial.print("### Wakeup! ###");
  Serial.println();
  Serial.print("# type(<,>,!) time(s) identifier data...");
  Serial.println();
  wakeTime = millis();
  carIsAwake = false;
  for(int i=0; i<130; i++) { 
    sendCAN(0x68C, wakeup_data, 1);
    waitCan(15);
    if (carIsAwake) {
      Serial.println("### Car is awake! ###");
      break;
    }
  }
  waitCan(50);
}

// Update data (Wake up)
void handleRefresh() {
  Serial.println("### Refresh! ###");
  resetData();
  wakeup();

  sendCAN(0x56E, heating_init, 4, 20, 100);
  
  // listen for a second so car has time to send new values:
  waitCan(1000);

  // and then go to sleep:
  sendCAN(0x56E, idle_data, 4, 20, 100);
  
  server.sendHeader("Location", "/");
  server.send(303);
}

void handleHeatOn() {
  Serial.println("### Heat ON! ###");
  isHeating = true;

  wakeup();
  sendCAN(0x56E, heating_init, 4,20,100);
  sendCAN(0x56E, heat_on_data, 4,20,100);
  sendCAN(0x56E, idle_data, 4,20,100);
  
  // After this the bus goes quiet, but the heating is on. 
  server.sendHeader("Location", "/"); server.send(303);
}

void handleHeatOff() {
  Serial.println("### Heat OFF! ###");
  isHeating = false;

  wakeup();
  sendCAN(0x56E, heating_init, 4, 10, 100); 
  sendCAN(0x56E, interrupt_data, 4, 9, 100);
  sendCAN(0x56E, heat_off_data, 4, 8, 100);
  sendCAN(0x56E, heating_init, 4, 4, 100);
  sendCAN(0x56E, idle_data, 4, 8, 100);
  
  // After this the bus goes quiet, and the heating is off.
  server.sendHeader("Location", "/"); server.send(303);
}

void handleChargeOn() {
  Serial.println("### Charge ON! ###");
  wakeup();
  sendCAN(0x56E, start_charge_data, 4, 20, 100);
  sendCAN(0x56E, idle_data, 4, 8, 100);
  
  server.sendHeader("Location", "/"); server.send(303);
}

void manageWiFi() {
  if(!wifiEnabled) return;
  static unsigned long previousAttemptTime = 0;
  static bool wifiAvailable = false;
  const unsigned long retryInterval = 10000;
  unsigned long currentTime = millis();

  if ((WiFi.status() != WL_CONNECTED) && (currentTime - previousAttemptTime >= retryInterval))
  {
    Serial.println("WiFi-yhteys puuttuu, yritetään yhdistää...");
    WiFi.disconnect();
    wifiAvailable = false;
    WiFi.begin(ssid, password);
    previousAttemptTime = currentTime;
  }
  else if (WiFi.status() == WL_CONNECTED && !wifiAvailable) {
    Serial.println("\nWi-Fi yhdistetty!");
    Serial.print("Löydät käyttöliittymän selaimella osoitteesta: http://");
    Serial.println(WiFi.localIP());
    wifiAvailable = true;
  }
}

void setup() {
  Serial.begin(115200);
  if (strlen(ssid) > 0) {
      WiFi.begin(ssid, password);
      wifiEnabled = true;
  }
  else {
    Serial.println("Wifi pois");
    wifiEnabled = false;
  }

  twai_timing_config_t t_config = TWAI_TIMING_CONFIG_500KBITS();
  twai_general_config_t g_config = TWAI_GENERAL_CONFIG_DEFAULT((gpio_num_t)CAN_TX_PIN, (gpio_num_t)CAN_RX_PIN, TWAI_MODE_NORMAL);
  twai_filter_config_t f_config = TWAI_FILTER_CONFIG_ACCEPT_ALL();

  twai_driver_install(&g_config, &t_config, &f_config);
  twai_start();

  if (wifiEnabled) {
      server.on("/", handleRoot);
      server.on("/refresh", handleRefresh);
      server.on("/heat_on", handleHeatOn);
      server.on("/heat_off", handleHeatOff);
      server.on("/charge_on", handleChargeOn);
      server.begin();
  }
}

void loop() {
  server.handleClient();
  readAndHandleCANMessage();
  manageWiFi();
}
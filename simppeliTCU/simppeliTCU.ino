#include <WiFi.h>
#include <WebServer.h>
#include "driver/twai.h"

// --- WI-FI ASETUKSET ---
const char* ssid = "SINUN_WIFI_NIMI";
const char* password = "SINUN_WIFI_SALASANA";

WebServer server(80);

// --- CAN ASETUKSET (Portti B) ---
#define CAN_TX_PIN 7 
#define CAN_RX_PIN 6

// Leafin taikatavut
uint8_t wakeup_data[1]   = {0x00};                   // 0x68C: Herätyspingi
uint8_t heat_on_data[4]  = {0x4E, 0x08, 0x00, 0x00}; // 0x56E: Lämmitys PÄÄLLE
uint8_t heat_off_data[4] = {0x86, 0x00, 0x00, 0x00}; // 0x56E: Lämmitys POIS
uint8_t charge_on_data[4]    = {0x66, 0x08, 0x00, 0x00}; // Lataus PÄÄLLE
uint8_t charge_abort_data[4] = {0xA6, 0x00, 0x00, 0x00}; // Lataus POIS (Abort)
uint8_t charge_trans_data[4] = {0x66, 0x08, 0x00, 0x00}; // Siirtymä (tässä 66, voi olla myös 46)

bool isCharging = false;
unsigned long lastChargeMsgTime = 0;
// Tilamuuttujat
bool isHeating = false;
unsigned long lastMsgTime = 0;

// Apufunktio CAN-viestin lähettämiseen
void sendCAN(uint32_t id, uint8_t* data, uint8_t len) {
  twai_message_t message;
  message.identifier = id;
  message.extd = 0;
  message.rtr = 0;
  message.data_length_code = len;
  for (int i = 0; i < len; i++) {
    message.data[i] = data[i];
  }
  twai_transmit(&message, pdMS_TO_TICKS(10));
}

void handleRoot() {
  String html = "<html><head><meta name='viewport' content='width=device-width, initial-scale=1'><style>";
  html += "body { font-family: Arial; text-align: center; margin-top: 30px; }";
  html += ".btn { padding: 15px 30px; font-size: 20px; margin: 10px; border-radius: 8px; text-decoration: none; color: white; display: inline-block; width: 80%; max-width: 300px;}";
  html += ".btn-heat-on { background-color: #ff5722; }";
  html += ".btn-heat-off { background-color: #795548; }";
  html += ".btn-charge-on { background-color: #4CAF50; }";
  html += ".btn-charge-off { background-color: #607d8b; }";
  html += "</style></head><body>";
  
  html += "<h1>Nissan Leaf (ZE1) Et&auml;ohjaus</h1>";
  
  // Tilanäyttö
  if (isHeating) html += "<h3 style='color: #ff5722;'>Tila: L&Auml;MMITYS P&Auml;&Auml;LL&Auml;</h3>";
  else if (isCharging) html += "<h3 style='color: #4CAF50;'>Tila: LATAUS P&Auml;&Auml;LL&Auml;</h3>";
  else html += "<h3 style='color: #555;'>Tila: LEPOTILA</h3>";

  // Napit
  html += "<a href='/heat_on' class='btn btn-heat-on'>K&auml;ynnist&auml; L&auml;mmitys</a><br>";
  html += "<a href='/heat_off' class='btn btn-heat-off'>Sammuta L&auml;mmitys</a><br><br>";
  html += "<a href='/charge_on' class='btn btn-charge-on'>K&auml;ynnist&auml; Lataus</a><br>";
  html += "<a href='/charge_off' class='btn btn-charge-off'>Keskeyt&auml; Lataus</a>";
  
  html += "</body></html>";
  server.send(200, "text/html", html);
}

// 2. Kun painetaan "Käynnistä"
void handleOn() {
  isHeating = true;
  // Lähetetään herätysviesti heti ensimmäisenä
  sendCAN(0x68C, wakeup_data, 1);
  delay(100);
  sendCAN(0x68C, wakeup_data, 1);
  delay(100);
  Serial.println("Käsky vastaanotettu: LÄMMITYS PÄÄLLE");
  
  // Ohjataan selain takaisin pääsivulle
  server.sendHeader("Location", "/");
  server.send(303);
}

// 3. Kun painetaan "Sammuta"
void handleOff() {
  isHeating = false;
  Serial.println("Käsky vastaanotettu: LÄMMITYS POIS (Aktiivinen keskeytys)");

  // 1. Keskeytyskomento (Abort) pakottaa releet auki
  uint8_t abort_data[4] = {0x96, 0x00, 0x00, 0x00};
  for(int i = 0; i < 5; i++) {
    sendCAN(0x56E, abort_data, 4);
    delay(100);
  }

  // 2. Siirtymätila
  uint8_t transition_data[4] = {0x56, 0x08, 0x00, 0x00};
  for(int i = 0; i < 5; i++) {
    sendCAN(0x56E, transition_data, 4);
    delay(100);
  }
  
  // 3. Paluu normaaliin lepotilaan (Idle)
  uint8_t idle_data[4] = {0x46, 0x08, 0x00, 0x00};
  for(int i = 0; i < 5; i++) {
    sendCAN(0x56E, idle_data, 4);
    delay(100);
  }
  
  // 4. Lopuksi annetaan auton nukahtaa (Sleep)
  for(int i = 0; i < 10; i++) {
    sendCAN(0x56E, heat_off_data, 4); // Tämä on se {0x86, 0x00, 0x00, 0x00}
    delay(100);
  }
  
  // Ohjataan selain takaisin pääsivulle
  server.sendHeader("Location", "/");
  server.send(303);
}

  void handleChargeOn() {
  isCharging = true;
  isHeating = false; // Varmuuden vuoksi peruutetaan lämmitys
  
  sendCAN(0x68C, wakeup_data, 1);
  delay(100);
  sendCAN(0x68C, wakeup_data, 1);
  
  Serial.println("Käsky: LATAUS PÄÄLLE");
  server.sendHeader("Location", "/");
  server.send(303);
}

void handleChargeOff() {
  isCharging = false;
  Serial.println("Käsky: LATAUS POIS (Täysi alasajosekvenssi)");

  // 1. Keskeytyskomento (Abort) - Pidetään yllä 4 sekuntia!
  // Latausvirran alasajo vaatii aikaa.
  uint8_t charge_abort_data[4] = {0xA6, 0x00, 0x00, 0x00};
  for(int i = 0; i < 40; i++) { 
    sendCAN(0x56E, charge_abort_data, 4);
    delay(100);
  }
  
  // 2. Siirtymä takaisin lepotilaan (Idle)
  // Tämä kertoo VCM:lle, että keskeytys on suoritettu.
  uint8_t idle_data[4] = {0x46, 0x08, 0x00, 0x00};
  for(int i = 0; i < 10; i++) { // 1 sekunti
    sendCAN(0x56E, idle_data, 4);
    delay(100);
  }
  
  // 3. Nukahtaminen (Sleep)
  // Releiden pitäisi viimeistään tässä vaiheessa aueta.
  uint8_t sleep_data[4] = {0x86, 0x00, 0x00, 0x00};
  for(int i = 0; i < 10; i++) { // 1 sekunti
    sendCAN(0x56E, sleep_data, 4); 
    delay(100);
  }
  
  server.sendHeader("Location", "/");
  server.send(303);
}

// 3. Kun painetaan "Sammuta"
void handleOnStop() {
  isHeating = false;
  Serial.println("Käsky vastaanotettu: LÄMMITYS POIS");
  
  // Lähetetään sammutusviesti muutaman kerran heti
  for(int i = 0; i < 10; i++) {
    sendCAN(0x56E, heat_off_data, 4);
    delay(50);
  }
  
  server.sendHeader("Location", "/");
  server.send(303);
}

void setup() {
  Serial.begin(115200);
  
  // Yhdistetään Wi-Fiin
  WiFi.begin(ssid, password);
  Serial.print("Yhdistetään verkkoon ");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWi-Fi yhdistetty!");
  Serial.print("Löydät käyttöliittymän selaimella osoitteesta: http://");
  Serial.println(WiFi.localIP());

  // Alustetaan CAN
  twai_timing_config_t t_config = TWAI_TIMING_CONFIG_500KBITS();
  twai_general_config_t g_config = TWAI_GENERAL_CONFIG_DEFAULT((gpio_num_t)CAN_TX_PIN, (gpio_num_t)CAN_RX_PIN, TWAI_MODE_NORMAL);
  twai_filter_config_t f_config = TWAI_FILTER_CONFIG_ACCEPT_ALL();

  if (twai_driver_install(&g_config, &t_config, &f_config) == ESP_OK) {
    twai_start();
    Serial.println("CAN-väylä (TWAI) käynnistetty.");
  }

  // Reititetään web-palvelimen osoitteet funktioihin
  server.on("/", handleRoot);
  server.on("/heat_on", handleOn);
  server.on("/heat_off", handleOff);
  server.on("/charge_on", handleChargeOn);
  server.on("/charge_off", handleChargeOff);
  server.begin();
}

void loop() {
  // Pitää web-palvelimen pyörimässä
  server.handleClient();
  
  // Tila-kone
  if (isHeating || isCharging) {
    if (millis() - lastMsgTime >= 100) {
      if (isHeating) {
        sendCAN(0x56E, heat_on_data, 4);
      } else if (isCharging) {
        sendCAN(0x56E, charge_on_data, 4);
      }
      lastMsgTime = millis();
    }
  }
}
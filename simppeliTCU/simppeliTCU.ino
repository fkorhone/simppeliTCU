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
static uint8_t wakeup_data[1]       = {0x00};                   // Herätyspingi (0x68C)
static uint8_t wakeup_601[2]        = {0x83, 0xC0};
static uint8_t heating_init[4]      = {0x46, 0x08, 0x00, 0x00}; // Lämmitys valmistelu (0x56E)
static uint8_t idle_data[4]         = {0x86, 0x00, 0x00, 0x00}; // Nukahtaminen (0x56E) Vaatii tarkistusta!

static uint8_t heat_on_data[4]      = {0x4E, 0x08, 0x00, 0x00}; // Lämmitys PÄÄLLE (0x56E)
static uint8_t start_charge_data[4] = {0x66, 0x08, 0x00, 0x00}; // Lataus PÄÄLLE (0x56E)

static uint8_t interrupt_data[4]    = {0x96, 0x00, 0x00, 0x00}; // Toiminnon keskeytys (0x56E)
static uint8_t heat_off_data[4]     = {0x56, 0x08, 0x00, 0x00}; // Lämmitys pois (0x56E)

// Tilamuuttujat
bool isHeating = false;
unsigned long lastMsgTime = 0;

// Autosta luetut arvot
float currentSOC = -1.0;
float cabinTemp = -99.0;
float batteryVoltage = -1.0;

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

// 1. Pääsivu (HTML Käyttöliittymä)
void handleRoot() {
  String html = "<html><head><meta name='viewport' content='width=device-width, initial-scale=1'><style>";
  html += "body { font-family: Arial; text-align: center; margin-top: 20px; }";
  html += ".btn { padding: 15px 30px; font-size: 18px; margin: 10px; border-radius: 8px; text-decoration: none; color: white; display: inline-block; width: 80%; max-width: 300px;}";
  html += ".btn-refresh { background-color: #2196F3; }";
  html += ".btn-heat-on { background-color: #ff5722; }";
  html += ".btn-heat-off { background-color: #795548; }";
  html += ".btn-charge-on { background-color: #4CAF50; }";
  html += ".data-box { background-color: #f1f1f1; padding: 15px; margin: 15px auto; width: 80%; max-width: 300px; border-radius: 10px; box-shadow: 2px 2px 5px rgba(0,0,0,0.1); }";
  html += "</style></head><body>";
  
  html += "<h1>Leaf Et&auml;ohjaus</h1>";

  // Näytetään luetut auton tiedot
  html += "<div class='data-box'>";
  html += "<h3>Auton Tila</h3>";
  
  if (currentSOC >= 0) {
    html += "<p style='font-size: 20px; margin: 5px;'><b>Akku:</b> " + String(currentSOC, 1) + " %</p>";
  } else {
    html += "<p style='font-size: 20px; margin: 5px;'><b>Akku:</b> Odotetaan...</p>";
  }

  if (batteryVoltage > 0 && batteryVoltage < 32) {
    html += "<p style='font-size: 20px; margin: 5px;'><b>12V Akku:</b> " + String(batteryVoltage, 1) + " V</p>";
  } else {
    html += "<p style='font-size: 20px; margin: 5px;'><b>12V Akku:</b> Odotetaan...</p>";
  }

  if (cabinTemp > -30.0 && cabinTemp < 80.0) {
    html += "<p style='font-size: 20px; margin: 5px;'><b>Sis&auml;l&auml;mp&ouml;:</b> " + String(cabinTemp, 1) + " &deg;C</p>";
  } else {
    html += "<p style='font-size: 20px; margin: 5px;'><b>Sis&auml;l&auml;mp&ouml;:</b> Odotetaan...</p>";
  }
  html += "</div>";

  html += "<a href='/refresh' class='btn btn-refresh'>P&auml;ivit&auml; tiedot (Her&auml;t&auml;)</a><br><hr style='width: 80%; max-width: 300px;'>";

  // Tilaindikaattorit
  if (isHeating) html += "<h3 style='color: #ff5722;'>L&Auml;MMITYS P&Auml;&Auml;LL&Auml;</h3>";
  
  // Napit
  html += "<a href='/heat_on' class='btn btn-heat-on'>K&auml;ynnist&auml; L&auml;mmitys</a><br>";
  html += "<a href='/heat_off' class='btn btn-heat-off'>Sammuta L&auml;mmitys</a><br><br>";
  html += "<a href='/charge_on' class='btn btn-charge-on'>K&auml;ynnist&auml; Lataus</a><br>";
  
  html += "</body></html>";
  server.send(200, "text/html", html);
}


void readAndHandleCANMessage() {
  twai_message_t message;
  if (twai_receive(&message, 0) == ESP_OK) { // 0 = ei odota, lukee vain jos dataa on
    // SOC (Akun varaus) ID: 0x55B
    if (message.identifier == 0x55B && message.data_length_code >= 2) {
      uint16_t soc_raw = (message.data[0] << 2) | (message.data[1] >> 6);
      currentSOC = soc_raw / 10.0;
    }
    // Sisälämpötila ID: 0x54F
    else if (message.identifier == 0x54F && message.data_length_code >= 1) {
      cabinTemp = (message.data[0] / 2.0) - 40.0;
    }
    else if (message.identifier == 0x5BC) {
      batteryVoltage = message.data[4] / 12.8;
    }
  }
}

void wakeup() {
  for(int i=0; i<60; i++) { sendCAN(0x68C, wakeup_data, 1); delay(15); }
  sendCAN(0x601, wakeup_601, 2);
  delay(15);
  sendCAN(0x68C, wakeup_data, 1);
  delay(50);
}

// Päivitä tiedot (Herätys)
void handleRefresh() {

  wakeup();

  for(int i=0; i<20; i++) { sendCAN(0x56E, heating_init, 4); delay(100); }
  
  // kuunnellaan sekunti, että auto ehtii lähettää uudet arvot:
  unsigned long start_time = millis();
  while (millis() - start_time < 1000) {
    readAndHandleCANMessage(); 
    sleep(1);
  }

  // ja sitten nukkumaan:
  for(int i=0; i<20; i++) { sendCAN(0x56E, idle_data, 4); delay(100); }
  
  server.sendHeader("Location", "/");
  server.send(303);
}

void handleHeatOn() {
  isHeating = true;

  wakeup();
  for(int i=0; i<20; i++) { sendCAN(0x56E, heating_init, 4); delay(100); }
  for(int i=0; i<20; i++) { sendCAN(0x56E, heat_on_data, 4); delay(100); }
  for(int i=0; i<20; i++) { sendCAN(0x56E, idle_data, 4); delay(100); }
  
  // After this the bus goes quiet, but the heating is on. 
  server.sendHeader("Location", "/"); server.send(303);
}

void handleHeatOff() {
  isHeating = false;

  wakeup();
  for(int i=0; i<10; i++) { sendCAN(0x56E, heating_init, 4); delay(100); }
  for(int i=0; i<9; i++) { sendCAN(0x56E, interrupt_data, 4); delay(100); }
  for(int i=0; i<8; i++) { sendCAN(0x56E, heat_off_data, 4); delay(100); }
  for(int i=0; i<4; i++) { sendCAN(0x56E, heating_init, 4); delay(100); }
  for(int i=0; i<8; i++) { sendCAN(0x56E, idle_data, 4); delay(100); }
  
  // After this the bus goes quiet, and the heating is off.
  server.sendHeader("Location", "/"); server.send(303);
}

void handleChargeOn() {

  wakeup();
  for(int i=0; i<20; i++) { sendCAN(0x56E, start_charge_data, 4); delay(100); }
  for(int i=0; i<8; i++) { sendCAN(0x56E, idle_data, 4); delay(100); }
  
  server.sendHeader("Location", "/"); server.send(303);
}

void setup() {
  Serial.begin(115200);
  
  Serial.print("Yhdistetään verkkoon ");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) { delay(500); }
  Serial.println("\nWi-Fi yhdistetty!");
  Serial.print("Löydät käyttöliittymän selaimella osoitteesta: http://");
  Serial.println(WiFi.localIP());
  
  twai_timing_config_t t_config = TWAI_TIMING_CONFIG_500KBITS();
  twai_general_config_t g_config = TWAI_GENERAL_CONFIG_DEFAULT((gpio_num_t)CAN_TX_PIN, (gpio_num_t)CAN_RX_PIN, TWAI_MODE_NORMAL);
  twai_filter_config_t f_config = TWAI_FILTER_CONFIG_ACCEPT_ALL();

  twai_driver_install(&g_config, &t_config, &f_config);
  twai_start();

  server.on("/", handleRoot);
  server.on("/refresh", handleRefresh);
  server.on("/heat_on", handleHeatOn);
  server.on("/heat_off", handleHeatOff);
  server.on("/charge_on", handleChargeOn);
  server.begin();
}

void loop() {
  server.handleClient();
  readAndHandleCANMessage();
}
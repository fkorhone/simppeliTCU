#include <WiFi.h>
#include <WebServer.h>
#include "configuration.h"
#include "canLeafZE1.h"
#include "ui.h"

WebServer server(80);

// State variables
bool wifiEnabled = false;
bool isHeating = false;
bool carIsAwake = false;

// Values read from car
float currentSOC = -1.0;
float cabinTemp = -99.0;

void resetData() {
    currentSOC = -1.0;
    cabinTemp = -99.0;
}

void handleRoot() {
  sendMainPage(server, currentSOC, cabinTemp, isHeating);
}

void handleCarAwake() {
  carIsAwake = true;
}

void handleCabinTemp(float temp) {
  cabinTemp = temp;
}

void handleRawSOC(float soc) {
  currentSOC = soc;
}

void wakeup() {
  Serial.print("### Wakeup! ###");
  Serial.println();
  resetCanLogTimestamps();
  Serial.print("# type(<,>,!) time(s) identifier data...");
  Serial.println();
  carIsAwake = false;
  for(int i=0; i<130; i++) {
    wake();
    if (carIsAwake) {
      Serial.println("### Car is awake! ###");
      break;
    }
  }
  waitCAN(50);
}

// Update data (Wake up)
void handleRefresh() {
  Serial.println("### Refresh! ###");
  resetData();
  
  wakeup();
  refreshSequence();

  server.sendHeader("Location", "/");
  server.send(303);
}

void handleHeatOn() {
  Serial.println("### Heat ON! ###");
  isHeating = true;

  wakeup();
  heatOnSequence();
  
  server.sendHeader("Location", "/"); server.send(303);
}

void handleHeatOff() {
  Serial.println("### Heat OFF! ###");
  isHeating = false;

  wakeup();
  heatOffSequence();
  
  server.sendHeader("Location", "/"); server.send(303);
}

void handleChargeOn() {
  Serial.println("### Charge ON! ###");
  
  wakeup();
  chargeOnSequence();
  
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

  setupCAN();

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
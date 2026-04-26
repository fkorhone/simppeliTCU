#include <WiFi.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include <NetBIOS.h>
#include "configuration.h"
#include "canLeafZE1.h"
#include "ui.h"
#include "mqttInterface.h"

WebServer server(80);


// State variables
bool wifiEnabled = false;
bool carIsAwake = false;

// Values read from car
float currentSOC = -1.0;
float cabinTemp = -99.0;
bool isChargingNow = false;
ChargerState currentChargerState = ChargerState::IDLE;
bool isHvacOn = false;

void resetData() {
    currentSOC = -1.0;
    cabinTemp = -99.0;
    isChargingNow = false;
    currentChargerState = ChargerState::IDLE;
    isHvacOn = false;
}

void handleRoot() {
  sendMainPage(server, currentSOC, cabinTemp, isChargingNow, currentChargerState, isHvacOn);
}

void handleCarAwake() {
  carIsAwake = true;
}

void handleChargerStatus(bool isCharging, ChargerState state) {
  isChargingNow = isCharging;
  currentChargerState = state;
  mqttUpdateCharging(isCharging, state);
}

void handleHVACStatus(bool isOn) {
  isHvacOn = isOn;
  
  mqttUpdateHVAC(isOn);
}

void handleCabinTemp(float temp) {
  cabinTemp = temp;
  mqttUpdateCabinTemp(temp);
}

void handleRawSOC(float soc) {
  currentSOC = soc;
  mqttUpdateSOC(soc);
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

void handleMqttRefresh() {
  Serial.println("### Refresh! (MQTT) ###");
  resetData();
  
  wakeup();
  refreshSequence();
}

void handleHvacOn() {
  Serial.println("### HVAC ON! ###");
  isHvacOn = true;

  wakeup();
  hvacOnSequence();
  
  server.sendHeader("Location", "/"); server.send(303);
}

void handleMqttHvacOn() {
  Serial.println("### HVAC ON! (MQTT) ###");
  isHvacOn = true;

  wakeup();
  hvacOnSequence();
  mqttPublishStatus("HVAC started!");
}

void handleHvacOff() {
  Serial.println("### HVAC OFF! ###");
  isHvacOn = false;

  wakeup();
  hvacOffSequence();
  
  server.sendHeader("Location", "/"); server.send(303);
}

void handleMqttHvacOff() {
  Serial.println("### HVAC OFF! (MQTT) ###");
  isHvacOn = false;

  wakeup();
  hvacOffSequence();
  mqttPublishStatus("HVAC stopped.");
}

void handleChargeOn() {
  Serial.println("### Charge ON! ###");
  
  wakeup();
  chargeOnSequence();
  
  server.sendHeader("Location", "/"); server.send(303);
}

void handleMqttChargeOn() {
  Serial.println("### Charge ON! (MQTT) ###");
  
  wakeup();
  chargeOnSequence();
  mqttPublishStatus("Charging forced on!");
}

void manageWiFi() {
  if(!wifiEnabled) return;
  static unsigned long previousAttemptTime = 0;
  static bool wifiAvailable = false;
  const unsigned long retryInterval = 10000;
  unsigned long currentTime = millis();

  if (WiFi.status() != WL_CONNECTED)
  {
    if (wifiAvailable) {
      Serial.println("WiFi disconnected, stopping mDNS/NBNS services...");
      MDNS.end();
      NBNS.end();
      wifiAvailable = false;
    }

    if (currentTime - previousAttemptTime >= retryInterval) {
      Serial.println("WiFi connection missing, trying to connect...");
      WiFi.disconnect();
      WiFi.begin(ssid, password);
      previousAttemptTime = currentTime;
    }
  }
  else if (!wifiAvailable) {
    Serial.println("\nWi-Fi connected!");
  
    if (!MDNS.begin(hostName)) {
      Serial.println("Error starting mDNS service!");
    } else {
      Serial.println("mDNS service started.");
      MDNS.addService("http", "tcp", 80);
    }
    if (!NBNS.begin(hostName)) {
      Serial.println("Error starting NBNS service!");
    } else {
      Serial.println("NBNS service started.");
    }

    Serial.print("You can find the UI in the browser at: http://");
    Serial.print(hostName);
    Serial.print(".local or http://");
    Serial.print(hostName);
    Serial.println(" or by IP address:");
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
    Serial.println("WiFi disabled");
    wifiEnabled = false;
  }

  setupMQTT();
  setupCAN();

  if (wifiEnabled) {
      server.on("/", handleRoot);
      server.on("/refresh", handleRefresh);
      server.on("/hvac_on", handleHvacOn);
      server.on("/hvac_off", handleHvacOff);
      server.on("/charge_on", handleChargeOn);
      server.begin();
  }
}

void loop() {
  server.handleClient();
  manageMQTT();
  readAndHandleCANMessage();
  manageWiFi();
}
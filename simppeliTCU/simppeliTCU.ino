#include <WiFi.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include <NetBIOS.h>
#include "configuration.h"
#include "cliParser.h"
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
int8_t lockState = -1;
float currentSetpoint = -1.0;
float currentFanSpeed = -1.0;
bool isHeatingEnabled = false;
bool isCoolingEnabled = false;
VentilationMode currentVentilationMode = VentilationMode::UNKNOWN;

void resetData() {
    currentSOC = -1.0;
    cabinTemp = -99.0;
    isChargingNow = false;
    currentChargerState = ChargerState::IDLE;
    isHvacOn = false;
    lockState = -1;
    currentSetpoint = -1.0;
    currentFanSpeed = -1.0;
    isHeatingEnabled = false;
    isCoolingEnabled = false;
    currentVentilationMode = VentilationMode::UNKNOWN;
}

void handleRoot() {
  bool sequenceActive = (activeSequence != CanSequence::NONE);
  sendMainPage(server, currentSOC, cabinTemp, isChargingNow, currentChargerState, isHvacOn, sequenceActive, getLockingEnabled(), lockState, currentSetpoint, currentFanSpeed, isHeatingEnabled, isCoolingEnabled, currentVentilationMode);
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

void handleHVACSetpoint(float setpoint) {
  // The car reports 0 for setpoint during remote climate. Ignore it if we already have a valid setpoint.
  if ((isHvacOn || activeSequence == CanSequence::HVAC_ON) && setpoint == 0.0f && currentSetpoint > 0.0f) {
      return;
  }
  currentSetpoint = setpoint;
  mqttUpdateHVACSetpoint(setpoint);
}

void handleFanSpeed(float speed) {
  // The car reports 0 for fan speed during remote climate even though it is blowing.
  if ((isHvacOn || activeSequence == CanSequence::HVAC_ON) && speed == 0.0f && currentFanSpeed > 0.0f) {
      return;
  }
  currentFanSpeed = speed;
  mqttUpdateFanSpeed(speed);
}

void handleHeatingMode(bool heating, bool cooling) {
  isHeatingEnabled = heating;
  isCoolingEnabled = cooling;
  mqttUpdateHeatingMode(heating, cooling);
}

void handleVentilationMode(VentilationMode mode) {
  currentVentilationMode = mode;
  mqttUpdateVentilationMode(mode);
}

void handleDoorStatus(bool fl, bool fr, bool rl, bool rr, bool trunk) {
  mqttUpdateDoors(fl, fr, rl, rr, trunk);
}

void handleLockStatus(bool locked) {
  lockState = locked ? 1 : 0;
  mqttUpdateLock(locked);
}

void handleRawSOC(float soc) {
  currentSOC = soc;
  mqttUpdateSOC(soc);
}

void logSequenceStart() {
  Serial.println("### Starting Sequence ###");
  Serial.println("# type(<,>,!) time(s) identifier data...");
}


// Update data (Wake up)
void handleRefresh() {
  Serial.println("### Refresh! ###");
  logSequenceStart();
  resetData();
  startSequence(CanSequence::REFRESH, millis());
  server.sendHeader("Location", "/");
  server.send(303);
}

void handleMqttRefresh() {
  Serial.println("### Refresh! (MQTT) ###");
  logSequenceStart();
  resetData();
  startSequence(CanSequence::REFRESH, millis());
}

void handleHvacOn() {
  Serial.println("### HVAC ON! ###");
  float setpoint = 0.0f;
  if (server.hasArg("setpoint")) {
    setpoint = server.arg("setpoint").toFloat();
  }
  Serial.print("Setpoint: ");
  Serial.println(setpoint);
  logSequenceStart();
  setHVACTargetTemperature(setpoint);
  if (setpoint > 0.0f) {
      handleHVACSetpoint(setpoint); // Pre-fill the UI/MQTT
  }
  startSequence(CanSequence::HVAC_ON, millis());
  server.sendHeader("Location", "/"); 
  server.send(303);
}

void handleMqttHvacOn(float setpoint) {
  Serial.print("### HVAC ON! (MQTT) Setpoint: ");
  Serial.println(setpoint);
  logSequenceStart();
  setHVACTargetTemperature(setpoint);
  if (setpoint > 0.0f) {
      handleHVACSetpoint(setpoint); // Pre-fill the UI/MQTT with the requested setpoint
  }
  startSequence(CanSequence::HVAC_ON, millis());
}

void handleHvacOff() {
  Serial.println("### HVAC OFF! ###");
  logSequenceStart();
  startSequence(CanSequence::HVAC_OFF, millis());
  server.sendHeader("Location", "/"); 
  server.send(303);
}

void handleMqttHvacOff() {
  Serial.println("### HVAC OFF! (MQTT) ###");
  logSequenceStart();
  startSequence(CanSequence::HVAC_OFF, millis());
}

void handleMqttLock() {
  if (!getLockingEnabled()) {
    Serial.println("### Lock Doors: Feature disabled (MQTT) ###");
    mqttPublishStatus("Door locking is disabled. Enable it via CLI: set locking_enabled true");
    return;
  }
  Serial.println("### Lock Doors! (MQTT) ###");
  logSequenceStart();
  startSequence(CanSequence::LOCK_DOORS, millis());
}

void handleMqttUnlock() {
  if (!getLockingEnabled()) {
    Serial.println("### Unlock Doors: Feature disabled (MQTT) ###");
    mqttPublishStatus("Door locking is disabled. Enable it via CLI: set locking_enabled true");
    return;
  }
  Serial.println("### Unlock Doors! (MQTT) ###");
  logSequenceStart();
  startSequence(CanSequence::UNLOCK_DOORS, millis());
}

void handleChargeOn() {
  Serial.println("### Charge ON! ###");
  logSequenceStart();
  startSequence(CanSequence::CHARGE_ON, millis());
  server.sendHeader("Location", "/"); 
  server.send(303);
}

void handleUnlock() {
  if (!getLockingEnabled()) {
    Serial.println("### Unlock Doors: Feature disabled ###");
    server.send(403, "text/plain", "Door locking is disabled. Enable it via CLI: set locking_enabled true");
    return;
  }
  Serial.println("### Unlock Doors! ###");

  logSequenceStart();
  startSequence(CanSequence::UNLOCK_DOORS, millis());

  server.sendHeader("Location", "/"); server.send(303);
}

void handleLock() {
  if (!getLockingEnabled()) {
    Serial.println("### Lock Doors: Feature disabled ###");
    server.send(403, "text/plain", "Door locking is disabled. Enable it via CLI: set locking_enabled true");
    return;
  }
  Serial.println("### Lock Doors! ###");

  logSequenceStart();
  startSequence(CanSequence::LOCK_DOORS, millis());

  server.sendHeader("Location", "/"); server.send(303);
}

void handleMqttChargeOn() {
  Serial.println("### Charge ON! (MQTT) ###");
  logSequenceStart();
  startSequence(CanSequence::CHARGE_ON, millis());
}

void manageWiFi() {
  if(!wifiEnabled) return;
  bool staEnabled = strlen(getWifiSSID()) > 0;
  static bool wifiAvailable = false;

  if (!staEnabled) {
    // AP-only mode, no STA connection to manage
    if (!wifiAvailable) {
      Serial.println("\nAP-only mode active!");
      
      if (!MDNS.begin(getHostName())) {
        Serial.println("Error starting mDNS service!");
      } else {
        Serial.println("mDNS service started.");
        MDNS.addService("http", "tcp", 80);
      }
      if (!NBNS.begin(getHostName())) {
        Serial.println("Error starting NBNS service!");
      } else {
        Serial.println("NBNS service started.");
      }

      Serial.print("You can find the UI in the browser at: http://");
      Serial.print(getHostName());
      Serial.print(".local or http://");
      Serial.print(getHostName());
      Serial.println(" or by AP IP address (usually 192.168.4.1):");
      Serial.println(WiFi.softAPIP());
      wifiAvailable = true;
    }
    return;
  }

  static unsigned long previousAttemptTime = 0;
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
      Serial.println("WiFi connection missing, waiting for auto-reconnect...");
      previousAttemptTime = currentTime;
    }
  }
  else if (!wifiAvailable) {
    Serial.println("\nWi-Fi connected!");
  
    if (!MDNS.begin(getHostName())) {
      Serial.println("Error starting mDNS service!");
    } else {
      Serial.println("mDNS service started.");
      MDNS.addService("http", "tcp", 80);
    }
    if (!NBNS.begin(getHostName())) {
      Serial.println("Error starting NBNS service!");
    } else {
      Serial.println("NBNS service started.");
    }

    Serial.print("You can find the UI in the browser at: http://");
    Serial.print(getHostName());
    Serial.print(".local or http://");
    Serial.print(getHostName());
    Serial.println(" or by IP address:");
    Serial.println(WiFi.localIP());
    wifiAvailable = true;
  }
}

void setup() {
  Serial.begin(115200);
  initConfiguration();

  bool staEnabled = strlen(getWifiSSID()) > 0;
  bool apEnabled = strlen(getApSSID()) > 0;

  if (apEnabled) {
      size_t apPassLen = strlen(getApPassword());
      if (apPassLen > 0 && apPassLen < 8) {
          Serial.println("Error: AP password is too short (must be >= 8 chars). AP mode will be disabled.");
          apEnabled = false;
      }
  }
  
  if (staEnabled && apEnabled) {
      WiFi.mode(WIFI_AP_STA);
      if (strlen(getApPassword()) > 0) {
          WiFi.softAP(getApSSID(), getApPassword());
      } else {
          WiFi.softAP(getApSSID());
      }
      WiFi.setAutoReconnect(true);
      WiFi.begin(getWifiSSID(), getWifiPassword());
      WiFi.setSleep(false);
      wifiEnabled = true;
  } else if (staEnabled) {
      WiFi.mode(WIFI_STA);
      WiFi.setAutoReconnect(true);
      WiFi.begin(getWifiSSID(), getWifiPassword());
      WiFi.setSleep(false);
      wifiEnabled = true;
  } else if (apEnabled) {
      WiFi.mode(WIFI_AP);
      if (strlen(getApPassword()) > 0) {
          WiFi.softAP(getApSSID(), getApPassword());
      } else {
          WiFi.softAP(getApSSID());
      }
      WiFi.setSleep(false);
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
      server.on("/unlock", handleUnlock);
      server.on("/lock", handleLock);
      server.begin();
  }
}

void loop() {
  server.handleClient();
  manageMQTT();
  readAndHandleCANMessage();
  CanSeqResult seqRes = manageCANSequence(millis());
  if (seqRes == CanSeqResult::WAKE_SUCCESS) {
      Serial.println("### Car is awake! ###");
  } else if (seqRes == CanSeqResult::WAKE_TIMEOUT) {
      Serial.println("### Car wake timeout! ###");
  } else if (seqRes == CanSeqResult::SEQUENCE_FINISHED) {
      Serial.println("### Sequence Complete ###");
  }
  manageWiFi();
  processSerialInput();
}
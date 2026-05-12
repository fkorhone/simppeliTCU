#include "mqttInterface.h"
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include "configuration.h"
#include "stringBuffer.h"

WiFiClientSecure espClient;
PubSubClient mqttClient(espClient);
unsigned long lastMqttReconnectAttempt = 0;

struct OvmsCommands {
  const char* hvacOn = "climatecontrol on";
  const char* hvacOff = "climatecontrol off";
  const char* chargeOn = "charge start";
  const char* status = "server v3 update modified";
};
static const OvmsCommands ovmsCmds;

struct OvmsMetrics {
  const char* soc = "metric/v/b/soc";
  const char* cabinTemp = "metric/v/e/cabintemp";
  const char* chargingState = "metric/v/c/state";
  const char* chargingActive = "metric/v/c/charging";
  const char* hvacActive = "metric/v/e/hvac";
};
static const OvmsMetrics ovmsMetrics;

static float lastSOC = -1.0;
static float lastCabinTemp = -99.0;
static bool lastChargingStateSet = false;
static bool lastIsCharging = false;
static ChargerState lastChargerState = ChargerState::IDLE;
static bool lastHvacStateSet = false;
static bool lastIsHvacOn = false;
static bool mqttStatusRequested = false;
static unsigned long statusRequestTime = 0;

static StringBuffer<64> mqttPrefix;
static StringBuffer<128> currentResponseTopic;

void mqttCallback(char* topic, byte* payload, unsigned int length) {
  StringBuffer<64> msg;
  msg.copyFromData(payload, length);
  msg.trim();
  msg.toLowerCase();

  Serial.print("MQTT Command received on topic: ");
  Serial.println(topic);
  Serial.print("Payload: ");
  Serial.println(msg.c_str());

  StringBuffer<80> clientPrefix;
  clientPrefix.format("%sclient/", mqttPrefix.c_str());
  
  StringBuffer<128> topicStr;
  topicStr.copyFrom(topic);
  
  // OVMS app sends commands to client/<clientid>/command/<commandid>
  if (topicStr.startsWith(clientPrefix.c_str())) {
    const char* clientInfo = topicStr.c_str() + clientPrefix.length();
    const char* cmdMarker = strstr(clientInfo, "/command/");
    
    if (cmdMarker != NULL) {
      size_t clientIdLen = cmdMarker - clientInfo;
      StringBuffer<64> clientId;
      clientId.copyFromData((const byte*)clientInfo, clientIdLen);
      
      const char* commandId = cmdMarker + 9;
      
      // Prepare the response topic to reply to this specific command
      currentResponseTopic.format("%sclient/%s/response/%s", mqttPrefix.c_str(), clientId.c_str(), commandId);

      if (msg.equals(ovmsCmds.hvacOn)) {
        handleMqttHvacOn();
      } 
      else if (msg.equals(ovmsCmds.hvacOff)) {
        handleMqttHvacOff();
      }
      else if (msg.equals(ovmsCmds.chargeOn)) {
        handleMqttChargeOn();
      }
      else if (msg.equals(ovmsCmds.status)) {
        handleMqttRefresh();
        mqttStatusRequested = true;
        statusRequestTime = millis();
        // Respond immediately, full status sent later in manageMQTT()
        mqttPublishStatus("Reading CAN bus...");
      }
      else {
        StringBuffer<64> unknownMsg;
        unknownMsg.format("Unknown command: %s", msg.c_str());
        mqttPublishStatus(unknownMsg.c_str());
      }
    }
  }
}

void mqttPublishHassDiscovery();

boolean reconnectMQTT() {
  StringBuffer<32> clientId;
  clientId.format("simppeliTCU-%04X", (unsigned int)random(0xffff));
  
  StringBuffer<128> willTopic;
  willTopic.format("%smetric/s/v3/connected", mqttPrefix.c_str());
  
  bool connected = false;
  if (strlen(getMqttUser()) > 0) {
    connected = mqttClient.connect(clientId.c_str(), getMqttUser(), getMqttPassword(), willTopic.c_str(), 0, true, "no");
  } else {
    connected = mqttClient.connect(clientId.c_str(), "", "", willTopic.c_str(), 0, true, "no");
  }

  if (connected) {
    Serial.println("SECURE MQTT Connected!");
    
    // Mark as connected in Retained message (for OVMS app to see vehicle is online)
    mqttClient.publish(willTopic.c_str(), (const uint8_t*)"yes", 3, true);
    
    // Subscribe to OVMS commands from any client
    StringBuffer<128> cmdTopic;
    cmdTopic.format("%sclient/+/command/+", mqttPrefix.c_str());
    mqttClient.subscribe(cmdTopic.c_str());
    
    mqttPublishHassDiscovery();

    // Send latest metrics on reconnect
    if (lastSOC >= 0) mqttUpdateSOC(lastSOC);
    if (lastCabinTemp > -30) mqttUpdateCabinTemp(lastCabinTemp);
    if (lastChargingStateSet) mqttUpdateCharging(lastIsCharging, lastChargerState);
    if (lastHvacStateSet) mqttUpdateHVAC(lastIsHvacOn);
    
    return true;
  }
  return false;
}

void setupMQTT() {
  mqttPrefix.format("ovms/%s/%s/", getMqttUser(), getVehicleId());
  espClient.setInsecure(); // Skip certificate validation for simplicity
  mqttClient.setServer(getMqttServer(), getMqttPort());
  
  // OVMS messages have longer topics, so enlarging buffer is recommended
  mqttClient.setBufferSize(512);
  mqttClient.setCallback(mqttCallback);
}

void manageMQTT() {
  if (strlen(getMqttServer()) == 0) {
    return; // Don't try to connect if MQTT server is not configured
  }

  if (WiFi.status() == WL_CONNECTED) {
    if (!mqttClient.connected()) {
      if (millis() - lastMqttReconnectAttempt > 5000) {
        lastMqttReconnectAttempt = millis();
        if (reconnectMQTT()) {
          lastMqttReconnectAttempt = 0;
        }
      }
    } else {
      mqttClient.loop();
    }
  }

  // Send status with delay if requested
  if (mqttStatusRequested && (millis() - statusRequestTime > 1500)) {
    StringBuffer<16> socStr;
    if (lastSOC >= 0) {
      socStr.format("%.1f%%", lastSOC);
    } else {
      socStr.copyFrom("No data");
    }
    
    StringBuffer<16> tempStr;
    if (lastCabinTemp > -30) {
      tempStr.format("%.1fC", lastCabinTemp);
    } else {
      tempStr.copyFrom("No data");
    }
    
    StringBuffer<128> statusMsg;
    statusMsg.format("Battery: %s | Temp: %s", socStr.c_str(), tempStr.c_str());
    mqttPublishStatus(statusMsg.c_str());
    
    mqttStatusRequested = false;
  }
}

void mqttPublishMetric(const char* metric, float value, const char* format = "%.1f") {
  if (mqttClient.connected()) {
    StringBuffer<128> topic;
    topic.format("%s%s", mqttPrefix.c_str(), metric);
    StringBuffer<16> payload;
    payload.format(format, value);
    // retained = true
    mqttClient.publish(topic.c_str(), (const uint8_t*)payload.c_str(), payload.length(), true);
  }
}

void mqttPublishMetricStr(const char* metric, const char* payload) {
  if (mqttClient.connected()) {
    StringBuffer<128> topic;
    topic.format("%s%s", mqttPrefix.c_str(), metric);
    mqttClient.publish(topic.c_str(), (const uint8_t*)payload, strlen(payload), true);
  }
}

void mqttUpdateCharging(bool isCharging, ChargerState state) {
  lastChargingStateSet = true;
  lastIsCharging = isCharging;
  lastChargerState = state;
  mqttPublishMetricStr(ovmsMetrics.chargingActive, isCharging ? "yes" : "no");
  switch (state) {
    case ChargerState::CHARGING: mqttPublishMetricStr(ovmsMetrics.chargingState, "charging"); break;
    case ChargerState::FINISHED: mqttPublishMetricStr(ovmsMetrics.chargingState, "done"); break;
    case ChargerState::INTERRUPTED: mqttPublishMetricStr(ovmsMetrics.chargingState, "stopped"); break;
    case ChargerState::WAITING: mqttPublishMetricStr(ovmsMetrics.chargingState, "wait"); break;
    case ChargerState::IDLE:
    default: mqttPublishMetricStr(ovmsMetrics.chargingState, ""); break;
  }
}

void mqttUpdateHVAC(bool isOn) {
  lastHvacStateSet = true;
  lastIsHvacOn = isOn;
  mqttPublishMetricStr(ovmsMetrics.hvacActive, isOn ? "yes" : "no");
}

void mqttUpdateSOC(float soc) {
  lastSOC = soc;
  mqttPublishMetric(ovmsMetrics.soc, soc);
}

void mqttUpdateCabinTemp(float temp) {
  lastCabinTemp = temp;
  mqttPublishMetric(ovmsMetrics.cabinTemp, temp);
}

void mqttPublishStatus(const char* msg) {
  if (mqttClient.connected() && !currentResponseTopic.isEmpty()) {
    mqttClient.publish(currentResponseTopic.c_str(), msg);
    
    // Once the reading CAN bus status returns, we clear the response topic
    if (!mqttStatusRequested) {
      currentResponseTopic.clear(); 
    }
  }
}

void mqttPublishHassConfig(const char* component, const char* entityId, const char* props[][2], size_t propCount) {
  if (!mqttClient.connected()) return;
  const char* vid = getVehicleId();
  StringBuffer<128> topic;
  topic.format("homeassistant/%s/simppeliTCU_%s/%s/config", component, vid, entityId);

  StringBuffer<512> payload;
  payload.copyFrom("{");
  const char* baseTopic = mqttPrefix.c_str();

  for (size_t i = 0; i < propCount; i++) {
    const char* key = props[i][0];
    const char* val = props[i][1];

    payload.append("\"");
    payload.append(key);
    payload.append("\":\"");

    if (strcmp(key, "state_topic") == 0 || strcmp(key, "command_topic") == 0) {
      payload.append(baseTopic);
    }
    payload.append(val);
    payload.append("\",");
  }

  StringBuffer<128> device;
  device.format("\"device\":{\"identifiers\":[\"simppeliTCU_%s\"],\"name\":\"simppeliTCU %s\",\"manufacturer\":\"simppeliTCU\",\"model\":\"TCU\"}}", vid, vid);
  payload.append(device.c_str());

  mqttClient.publish(topic.c_str(), (const uint8_t*)payload.c_str(), payload.length(), true);
}

void mqttPublishHassDiscovery() {
  if (!mqttClient.connected()) return;

  const char* socProps[][2] = {
    {"name", "SOC"},
    {"device_class", "battery"},
    {"unit_of_measurement", "%"},
    {"state_topic", ovmsMetrics.soc}
  };
  mqttPublishHassConfig("sensor", "soc", socProps, 4);

  const char* tempProps[][2] = {
    {"name", "Cabin Temperature"},
    {"device_class", "temperature"},
    {"unit_of_measurement", "°C"},
    {"state_topic", ovmsMetrics.cabinTemp}
  };
  mqttPublishHassConfig("sensor", "cabintemp", tempProps, 4);

  const char* chargeProps[][2] = {
    {"name", "Charging Active"},
    {"device_class", "battery_charging"},
    {"state_topic", ovmsMetrics.chargingActive},
    {"payload_on", "yes"},
    {"payload_off", "no"}
  };
  mqttPublishHassConfig("binary_sensor", "charging", chargeProps, 5);

  const char* hvacProps[][2] = {
    {"name", "HVAC Active"},
    {"device_class", "running"},
    {"state_topic", ovmsMetrics.hvacActive},
    {"payload_on", "yes"},
    {"payload_off", "no"}
  };
  mqttPublishHassConfig("binary_sensor", "hvac", hvacProps, 5);

  const char* hvacSwProps[][2] = {
    {"name", "HVAC Control"},
    {"state_topic", ovmsMetrics.hvacActive},
    {"command_topic", "client/hass/command/0"},
    {"payload_on", "climatecontrol on"},
    {"payload_off", "climatecontrol off"},
    {"state_on", "yes"},
    {"state_off", "no"}
  };
  mqttPublishHassConfig("switch", "hvac_switch", hvacSwProps, 7);

  const char* chargeSwProps[][2] = {
    {"name", "Charge Control"},
    {"state_topic", ovmsMetrics.chargingActive},
    {"command_topic", "client/hass/command/1"},
    {"payload_on", "charge start"},
    {"state_on", "yes"},
    {"state_off", "no"}
  };
  mqttPublishHassConfig("switch", "charge_switch", chargeSwProps, 6);

  const char* refreshProps[][2] = {
    {"name", "Refresh Data"},
    {"command_topic", "client/hass/command/2"},
    {"payload_press", "server v3 update modified"}
  };
  mqttPublishHassConfig("button", "refresh", refreshProps, 3);
}

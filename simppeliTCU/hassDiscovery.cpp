#include "hassDiscovery.h"
#include "configuration.h"
#include "stringBuffer.h"
#include <PubSubClient.h>

// Define the global instance of the metric topics
const OvmsMetrics ovmsMetrics;

// Helper function to publish a single HASS config message
void publishHassConfigForMetric(PubSubClient& mqttClient, const StringBuffer<128>& mqttPrefix, const HassMqttMetric& metric) {
  const char* vid = getVehicleId();
  StringBuffer<128> topic;
  topic.format("homeassistant/%s/simppeliTCU_%s/%s/config", metric.hassComponent, vid, metric.entityId);

  StringBuffer<768> payload;
  payload.copyFrom("{");

  payload.append("\"unique_id\":\"simppeliTCU_");
  payload.append(vid);
  payload.append("_");
  payload.append(metric.entityId);
  payload.append("\",");

  for (size_t i = 0; i < metric.propertyCount; i++) {
    const auto& prop = metric.properties[i];
    payload.append("\"");
    payload.append(prop.key);
    payload.append("\":\"");

    // Prepend the base OVMS topic path for state and command topics
    if (strcmp(prop.key, "state_topic") == 0 || strcmp(prop.key, "command_topic") == 0) {
      payload.append(mqttPrefix.c_str());
    }
    payload.append(prop.value);
    payload.append("\"");
    if (i < metric.propertyCount - 1) {
      payload.append(",");
    }
  }

  // Add common device information
  StringBuffer<256> device;
  device.format(",\"device\":{\"identifiers\":[\"simppeliTCU_%s\"],\"name\":\"simppeliTCU %s\",\"manufacturer\":\"simppeliTCU\",\"model\":\"TCU\"}}", vid, vid);
  payload.append(device.c_str());

  mqttClient.publish(topic.c_str(), (const uint8_t*)payload.c_str(), payload.length(), true);
}

// --- Metric Definitions ---

// Sensor: State of Charge
static const HassMqttProperty socProps[] = {
  {"name", "SOC"},
  {"device_class", "battery"},
  {"unit_of_measurement", "%"},
  {"state_topic", ovmsMetrics.soc}
};

// Sensor: Cabin Temperature
static const HassMqttProperty tempProps[] = {
  {"name", "Cabin Temperature"},
  {"device_class", "temperature"},
  {"unit_of_measurement", "°C"},
  {"state_topic", ovmsMetrics.cabinTemp}
};

// Binary Sensor: Charging Status
static const HassMqttProperty chargeProps[] = {
  {"name", "Charging Active"},
  {"device_class", "battery_charging"},
  {"state_topic", ovmsMetrics.chargingActive},
  {"payload_on", "yes"},
  {"payload_off", "no"}
};

// Binary Sensor: HVAC Status
static const HassMqttProperty hvacProps[] = {
  {"name", "HVAC Active"},
  {"device_class", "running"},
  {"state_topic", ovmsMetrics.hvacActive},
  {"payload_on", "yes"},
  {"payload_off", "no"}
};

// Sensor: Charging State
static const HassMqttProperty chargeStateProps[] = {
  {"name", "Charging State"},
  {"state_topic", ovmsMetrics.chargingState}
};

// Sensor: HVAC Setpoint
static const HassMqttProperty hvacSetpointProps[] = {
  {"name", "HVAC Setpoint"},
  {"device_class", "temperature"},
  {"unit_of_measurement", "°C"},
  {"state_topic", ovmsMetrics.hvacSetpoint}
};

// Sensor: Fan Speed
static const HassMqttProperty fanSpeedProps[] = {
  {"name", "Fan Speed"},
  {"unit_of_measurement", "%"},
  {"state_topic", ovmsMetrics.fanSpeed}
};

// Sensor: Ventilation Mode
static const HassMqttProperty ventilationModeProps[] = {
  {"name", "Ventilation Mode"},
  {"state_topic", ovmsMetrics.ventilationMode}
};

// Binary Sensor: Lock Status
static const HassMqttProperty lockProps[] = {
  {"name", "Locked"},
  {"device_class", "lock"},
  {"state_topic", ovmsMetrics.locked},
  {"payload_on", "yes"},
  {"payload_off", "no"}
};

// Binary Sensor: Heating Status
static const HassMqttProperty heatingProps[] = {
  {"name", "Heating Active"},
  {"state_topic", ovmsMetrics.heating},
  {"payload_on", "yes"},
  {"payload_off", "no"}
};

// Binary Sensor: Cooling Status
static const HassMqttProperty coolingProps[] = {
  {"name", "Cooling Active"},
  {"state_topic", ovmsMetrics.cooling},
  {"payload_on", "yes"},
  {"payload_off", "no"}
};

// Binary Sensor: Door Status (template for all doors)
static const HassMqttProperty doorProps[] = {
  {"device_class", "door"},
  {"payload_on", "yes"},
  {"payload_off", "no"}
};

// Switch: Lock Control
static const HassMqttProperty lockSwProps[] = {
  {"name", "Door Lock"},
  {"state_topic", ovmsMetrics.locked},
  {"command_topic", "client/hass/command/lock"},
  {"payload_on", "lock"},
  {"payload_off", "unlock"},
  {"state_on", "yes"},
  {"state_off", "no"},
  {"icon", "mdi:car-door-lock"}
};

// Switch: HVAC Control
static const HassMqttProperty hvacSwProps[] = {
  {"name", "HVAC Control"},
  {"state_topic", ovmsMetrics.hvacActive},
  {"command_topic", "client/hass/command/hvac"},
  {"payload_on", "climatecontrol on"},
  {"payload_off", "climatecontrol off"},
  {"state_on", "yes"},
  {"state_off", "no"}
};

// Button: Charge Control
static const HassMqttProperty chargeBtnProps[] = {
  {"name", "Start Charging"},
  {"command_topic", "client/hass/command/charge"},
  {"payload_press", "charge start"}
};

// Button: Refresh Data
static const HassMqttProperty refreshProps[] = {
  {"name", "Refresh Data"},
  {"command_topic", "client/hass/command/refresh"},
  {"payload_press", "server v3 update modified"}
};

// The Central Metric Registry
static const HassMqttMetric hassMetrics[] = {
  // Sensors
  {"sensor", "soc", socProps, sizeof(socProps)/sizeof(HassMqttProperty)},
  {"sensor", "cabin_temp", tempProps, sizeof(tempProps)/sizeof(HassMqttProperty)},
  {"sensor", "charge_state", chargeStateProps, sizeof(chargeStateProps)/sizeof(HassMqttProperty)},
  {"sensor", "hvac_setpoint", hvacSetpointProps, sizeof(hvacSetpointProps)/sizeof(HassMqttProperty)},
  {"sensor", "fan_speed", fanSpeedProps, sizeof(fanSpeedProps)/sizeof(HassMqttProperty)},
  {"sensor", "ventilation_mode", ventilationModeProps, sizeof(ventilationModeProps)/sizeof(HassMqttProperty)},

  // Binary Sensors
  {"binary_sensor", "charging", chargeProps, sizeof(chargeProps)/sizeof(HassMqttProperty)},
  {"binary_sensor", "hvac", hvacProps, sizeof(hvacProps)/sizeof(HassMqttProperty)},
  {"binary_sensor", "locked", lockProps, sizeof(lockProps)/sizeof(HassMqttProperty)},
  {"binary_sensor", "heating", heatingProps, sizeof(heatingProps)/sizeof(HassMqttProperty)},
  {"binary_sensor", "cooling", coolingProps, sizeof(coolingProps)/sizeof(HassMqttProperty)},

  // Switches
  {"switch", "hvac_switch", hvacSwProps, sizeof(hvacSwProps)/sizeof(HassMqttProperty)},
  {"switch", "lock_switch", lockSwProps, sizeof(lockSwProps)/sizeof(HassMqttProperty)},

  // Buttons
  {"button", "charge", chargeBtnProps, sizeof(chargeBtnProps)/sizeof(HassMqttProperty)},
  {"button", "refresh", refreshProps, sizeof(refreshProps)/sizeof(HassMqttProperty)},
};

// Door definitions
struct HassDoorDefinition {
    const char* entityId;
    const char* name;
    const char* stateTopic;
};

static const HassDoorDefinition doorDefinitions[] = {
    {"door_fl", "Front Left Door", ovmsMetrics.doorFL},
    {"door_fr", "Front Right Door", ovmsMetrics.doorFR},
    {"door_rl", "Rear Left Door", ovmsMetrics.doorRL},
    {"door_rr", "Rear Right Door", ovmsMetrics.doorRR},
    {"door_trunk", "Trunk", ovmsMetrics.doorTrunk},
};

/**
 * @brief Publishes all Home Assistant discovery configuration messages.
 * 
 * Iterates through the central metric registry and sends a retained MQTT message
 * for each, allowing Home Assistant to automatically discover and configure the entities.
 * 
 * @param mqttClient The active PubSubClient instance.
 * @param mqttPrefix The base topic prefix for the device (e.g., "ovms/user/vehicleid/").
 */
void mqttPublishHassDiscovery(PubSubClient& mqttClient, const StringBuffer<128>& mqttPrefix) {
  if (!mqttClient.connected()) return;

  for (const auto& metric : hassMetrics) {
    publishHassConfigForMetric(mqttClient, mqttPrefix, metric);
  }

  // Publish door sensors separately as they share properties
  for (const auto& door : doorDefinitions) {
    const char* vid = getVehicleId();
    StringBuffer<128> topic;
    topic.format("homeassistant/binary_sensor/simppeliTCU_%s/%s/config", vid, door.entityId);

    StringBuffer<768> payload;
    payload.format("{\"unique_id\":\"simppeliTCU_%s_%s\",\"name\":\"%s\",\"state_topic\":\"%s%s\",\"device_class\":\"door\",\"payload_on\":\"yes\",\"payload_off\":\"no\"",
                   vid, door.entityId, door.name, mqttPrefix.c_str(), door.stateTopic);

    // Add common device information
    StringBuffer<256> device;
    device.format(",\"device\":{\"identifiers\":[\"simppeliTCU_%s\"],\"name\":\"simppeliTCU %s\",\"manufacturer\":\"simppeliTCU\",\"model\":\"TCU\"}}", vid, vid);
    payload.append(device.c_str());

    mqttClient.publish(topic.c_str(), (const uint8_t*)payload.c_str(), payload.length(), true);
  }
}
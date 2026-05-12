#ifndef HASS_DISCOVERY_H
#define HASS_DISCOVERY_H

#include <stddef.h>

// Forward declarations to avoid including large headers
class PubSubClient;
template <size_t N> class StringBuffer;

// Defines the topic paths for the metrics, compatible with OVMS
struct OvmsMetrics {
  const char* soc = "metric/v/b/soc";
  const char* cabinTemp = "metric/v/e/cabintemp";
  const char* chargingState = "metric/v/c/state";
  const char* chargingActive = "metric/v/c/charging";
  const char* hvacActive = "metric/v/e/hvac";
  const char* doorFL = "metric/v/d/fl";
  const char* doorFR = "metric/v/d/fr";
  const char* doorRL = "metric/v/d/rl";
  const char* doorRR = "metric/v/d/rr";
  const char* doorTrunk = "metric/v/d/trunk";
  const char* locked = "metric/v/e/locked";
  const char* hvacSetpoint = "metric/v/e/cabinsetpoint";
  const char* fanSpeed = "metric/v/e/cabinfan";
  const char* heating = "metric/v/e/heating";
  const char* cooling = "metric/v/e/cooling";
  const char* ventilationMode = "metric/v/e/cabinvent";
};
extern const OvmsMetrics ovmsMetrics;

// Represents a single key-value pair for HASS discovery JSON
struct HassMqttProperty {
  const char* key;
  const char* value;
};

// Represents a full metric to be published to HASS
struct HassMqttMetric {
  const char* hassComponent; // "sensor", "binary_sensor", "switch", etc.
  const char* entityId;
  const HassMqttProperty* properties;
  size_t propertyCount;
};

void mqttPublishHassDiscovery(PubSubClient& mqttClient, const StringBuffer<128>& mqttPrefix);

#endif // HASS_DISCOVERY_H

#include "mqttInterface.h"
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include "configuration.h"

WiFiClientSecure espClient;
PubSubClient mqttClient(espClient);
unsigned long lastMqttReconnectAttempt = 0;

static float lastSOC = -1.0;
static float lastCabinTemp = -99.0;
static bool mqttStatusRequested = false;
static unsigned long statusRequestTime = 0;

void mqttCallback(char* topic, byte* payload, unsigned int length) {
  String msg = "";
  for (unsigned int i = 0; i < length; i++) {
    msg += (char)payload[i];
  }
  
  Serial.println("MQTT Command received: " + msg);

  if (msg == "HEAT_ON") {
    handleMqttHeatOn();
  } 
  else if (msg == "HEAT_OFF") {
    handleMqttHeatOff();
  }
  else if (msg == "CHARGE_ON") {
    handleMqttChargeOn();
  }
  else if (msg == "STATUS") {
    handleMqttRefresh();
    mqttStatusRequested = true;
    statusRequestTime = millis();
    mqttPublishStatus("Reading CAN bus...");
  }
}

boolean reconnectMQTT() {
  String clientId = "simppeliTCU-" + String(random(0xffff), HEX);
  
  bool connected = false;
  if (strlen(mqtt_user) > 0) {
    connected = mqttClient.connect(clientId.c_str(), mqtt_user, mqtt_password);
  } else {
    connected = mqttClient.connect(clientId.c_str());
  }

  if (connected) {
    Serial.println("SECURE MQTT Connected!");
    mqttClient.subscribe(mqtt_topic_commands);
    mqttPublishStatus("simppeliTCU (TLS) connected to network!");
    return true;
  }
  return false;
}

void setupMQTT() {
  espClient.setInsecure(); // Skip certificate validation for simplicity
  mqttClient.setServer(mqtt_server, mqtt_port);
  mqttClient.setCallback(mqttCallback);
}

void manageMQTT() {
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
    String statusMsg = "Battery: " + (lastSOC >= 0 ? String(lastSOC, 1) + "%" : "No data") + 
                       " | Temp: " + (lastCabinTemp > -30 ? String(lastCabinTemp, 1) + "C" : "No data");
    mqttPublishStatus(statusMsg.c_str());
    mqttStatusRequested = false;
  }
}

void mqttUpdateSOC(float soc) {
  lastSOC = soc;
}

void mqttUpdateCabinTemp(float temp) {
  lastCabinTemp = temp;
}

void mqttPublishStatus(const char* msg) {
  if (mqttClient.connected()) {
    mqttClient.publish(mqtt_topic_status, msg);
  }
}

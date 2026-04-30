#ifndef CONFIGURATION_H
#define CONFIGURATION_H

#include <Arduino.h>

enum class ConfigStatus {
    OK,
    ERROR_UNKNOWN,
    ERROR_LENGTH,
    ERROR_HOSTNAME_LENGTH,
    ERROR_HOSTNAME_HYPHEN,
    ERROR_HOSTNAME_CHARS,
    ERROR_PORT_RANGE
};

void initConfiguration();

const char* getWifiSSID();
ConfigStatus setWifiSSID(const char* value);

const char* getWifiPassword();
ConfigStatus setWifiPassword(const char* value);

const char* getApSSID();
ConfigStatus setApSSID(const char* value);

const char* getApPassword();
ConfigStatus setApPassword(const char* value);

const char* getHostName();
ConfigStatus setHostName(const char* value);

const char* getMqttServer();
ConfigStatus setMqttServer(const char* value);

int getMqttPort();
ConfigStatus setMqttPort(int value);

const char* getMqttUser();
ConfigStatus setMqttUser(const char* value);

const char* getMqttPassword();
ConfigStatus setMqttPassword(const char* value);

const char* getVehicleId();
ConfigStatus setVehicleId(const char* value);

#endif
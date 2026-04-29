#include "cliParser.h"
#include <Arduino.h>
#include "configuration.h"

static char inputBuffer[128];
static int inputIndex = 0;

void executeCommand(char* cmd) {
    char* command = strtok(cmd, " ");
    if (command == NULL) return;

    if (strcmp(command, "list") == 0) {
        char* option = strtok(NULL, " ");
        bool showSecrets = (option != NULL && strcmp(option, "--secrets") == 0);

        Serial.println("Current Configuration:");
        Serial.printf("  ssid: %s\n", getWifiSSID());
        Serial.printf("  password: %s\n", showSecrets ? getWifiPassword() : "****");
        Serial.printf("  hostName: %s\n", getHostName());
        Serial.printf("  mqtt_server: %s\n", getMqttServer());
        Serial.printf("  mqtt_port: %d\n", getMqttPort());
        Serial.printf("  mqtt_user: %s\n", getMqttUser());
        Serial.printf("  mqtt_password: %s\n", showSecrets ? getMqttPassword() : "****");
        Serial.printf("  vehicle_id: %s\n", getVehicleId());
        if (!showSecrets) {
            Serial.println("  (use 'list --secrets' to display secret values)");
        }
    } 
    else if (strcmp(command, "get") == 0) {
        char* key = strtok(NULL, " ");
        if (key != NULL) {
            if (strcmp(key, "ssid") == 0) Serial.println(getWifiSSID());
            else if (strcmp(key, "password") == 0) Serial.println("****");
            else if (strcmp(key, "hostName") == 0) Serial.println(getHostName());
            else if (strcmp(key, "mqtt_server") == 0) Serial.println(getMqttServer());
            else if (strcmp(key, "mqtt_port") == 0) Serial.println(getMqttPort());
            else if (strcmp(key, "mqtt_user") == 0) Serial.println(getMqttUser());
            else if (strcmp(key, "mqtt_password") == 0) Serial.println("****");
            else if (strcmp(key, "vehicle_id") == 0) Serial.println(getVehicleId());
            else Serial.println("Unknown parameter.");
        } else {
            Serial.println("Usage: get <key>");
        }
    } 
    else if (strcmp(command, "set") == 0) {
        char* key = strtok(NULL, " ");
        if (key != NULL) {
            char* val = strtok(NULL, ""); // Get rest of the string
            if (val == NULL) val = (char*)""; // Allow empty values

            ConfigStatus status = ConfigStatus::ERROR_UNKNOWN;

            if (strcmp(key, "ssid") == 0) status = setWifiSSID(val);
            else if (strcmp(key, "password") == 0) status = setWifiPassword(val);
            else if (strcmp(key, "hostName") == 0) status = setHostName(val);
            else if (strcmp(key, "mqtt_server") == 0) status = setMqttServer(val);
            else if (strcmp(key, "mqtt_port") == 0) {
                int port = atoi(val);
                if (port > 0 && port <= 65535) {
                    status = setMqttPort(port);
                } else {
                    status = ConfigStatus::ERROR_PORT_RANGE;
                }
            }
            else if (strcmp(key, "mqtt_user") == 0) status = setMqttUser(val);
            else if (strcmp(key, "mqtt_password") == 0) status = setMqttPassword(val);
            else if (strcmp(key, "vehicle_id") == 0) status = setVehicleId(val);
            else {
                Serial.println("Unknown parameter.");
                return;
            }
            
            if (status == ConfigStatus::OK) {
                Serial.println("Value updated.");
            } else if (status == ConfigStatus::ERROR_LENGTH) {
                Serial.println("Error: Value exceeds maximum allowed length.");
            } else if (status == ConfigStatus::ERROR_HOSTNAME_LENGTH) {
                Serial.println("Error: Hostname must be 1-15 characters long.");
            } else if (status == ConfigStatus::ERROR_HOSTNAME_HYPHEN) {
                Serial.println("Error: Hostname cannot start or end with a hyphen.");
            } else if (status == ConfigStatus::ERROR_HOSTNAME_CHARS) {
                Serial.println("Error: Hostname can only contain letters, numbers, and hyphens.");
            } else if (status == ConfigStatus::ERROR_PORT_RANGE) {
                Serial.println("Error: Invalid port number. Must be 1-65535. Value not updated.");
            } else {
                Serial.println("Error: Failed to update value.");
            }
        } else {
            Serial.println("Usage: set <key> <value>");
        }
    } 
    else if (strcmp(command, "reboot") == 0) {
        Serial.println("Rebooting...");
        delay(100);
        ESP.restart();
    } 
    else {
        Serial.println("Unknown command.");
    }
}

void processSerialInput() {
    while (Serial.available() > 0) {
        char c = Serial.read();
        
        if (c == '\n' || c == '\r') {
            if (inputIndex > 0) {
                inputBuffer[inputIndex] = '\0';
                executeCommand(inputBuffer);
                inputIndex = 0;
            }
        } 
        else if (inputIndex < (sizeof(inputBuffer) - 1)) {
            inputBuffer[inputIndex++] = c;
        }
    }
}
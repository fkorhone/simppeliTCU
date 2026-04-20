#ifndef CONFIGURATION_H
#define CONFIGURATION_H

// --- WI-FI SETTINGS ---
static const char* ssid = "";
static const char* password = "";

// --- MDNS / NETBIOS SETTINGS ---

// hostName must be valid for both mDNS and NetBIOS:
// - 1..15 characters (NetBIOS compatibility)
// - letters, digits, and '-' only
// - must not start or end with '-'
static const char* hostName = "leaf";

// --- MQTT SETTINGS (Secure TLS connection) ---
static const char* mqtt_server = "";
static const int mqtt_port = 8883; // 8883 is the port for secure traffic
static const char* mqtt_user = "";
static const char* mqtt_password = "";
static const char* vehicle_id = "";

#endif
#ifndef CONFIGURATION_H
#define CONFIGURATION_H

// --- WI-FI SETTINGS ---
const char* ssid = "";
const char* password = "";

// --- MDNS / NETBIOS SETTINGS ---

// hostName must be valid for both mDNS and NetBIOS:
// - 1..15 characters (NetBIOS compatibility)
// - letters, digits, and '-' only
// - must not start or end with '-'
const char* hostName = "leaf";

#endif
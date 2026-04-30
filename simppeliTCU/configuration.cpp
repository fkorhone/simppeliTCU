#include "configuration.h"
#include <Preferences.h>
#include "stringBuffer.h"

static Preferences preferences;
static bool nvsReady = false;

template <size_t N>
class ConfigString {
private:
    StringBuffer<N> value;
    const char* key;
    const char* default_val;
public:
    ConfigString(const char* k, const char* def) : key(k), default_val(def) {}
    
    void load() {
        if (nvsReady && preferences.isKey(key)) {
            preferences.getString(key, value.data(), value.capacity());
        } else {
            value.copyFrom(default_val);
            if (nvsReady) preferences.putString(key, value.c_str());
        }
    }
    
    ConfigStatus set(const char* val) {
        if (strlen(val) >= value.capacity()) {
            return ConfigStatus::ERROR_LENGTH;
        }
        value.copyFrom(val);
        if (nvsReady) preferences.putString(key, value.c_str());
        return ConfigStatus::OK;
    }
    
    const char* get() const { 
        return value.c_str(); 
    }
};

class ConfigInt {
private:
    int value;
    const char* key;
    int default_val;
public:
    ConfigInt(const char* k, int def) : key(k), default_val(def) {}
    
    void load() {
        if (nvsReady && preferences.isKey(key)) {
            value = preferences.getInt(key, default_val);
        } else {
            value = default_val;
            if (nvsReady) preferences.putInt(key, value);
        }
    }
    
    ConfigStatus set(int val) {
        value = val;
        if (nvsReady) preferences.putInt(key, value);
        return ConfigStatus::OK;
    }
    
    int get() const { 
        return value; 
    }
};

// Define config objects
static ConfigString<64> conf_ssid("ssid", "");
static ConfigString<64> conf_password("password", "");
static ConfigString<64> conf_ap_ssid("ap_ssid", "");
static ConfigString<64> conf_ap_password("ap_password", "");
static ConfigString<64> conf_hostName("hostName", "leaf");
static ConfigString<64> conf_mqtt_server("mqtt_server", "");
static ConfigInt        conf_mqtt_port("mqtt_port", 8883);
static ConfigString<64> conf_mqtt_user("mqtt_user", "");
static ConfigString<64> conf_mqtt_password("mqtt_password", "");
static ConfigString<64> conf_vehicle_id("vehicle_id", "");

void initConfiguration() {
    nvsReady = preferences.begin("config", false);
    if (!nvsReady) {
        Serial.println("Error: Failed to initialize NVS (Preferences). Falling back to RAM defaults.");
    }

    conf_ssid.load();
    conf_password.load();
    conf_ap_ssid.load();
    conf_ap_password.load();
    conf_hostName.load();
    conf_mqtt_server.load();
    conf_mqtt_port.load();
    conf_mqtt_user.load();
    conf_mqtt_password.load();
    conf_vehicle_id.load();
}

const char* getWifiSSID() { return conf_ssid.get(); }
ConfigStatus setWifiSSID(const char* value) { return conf_ssid.set(value); }

const char* getWifiPassword() { return conf_password.get(); }
ConfigStatus setWifiPassword(const char* value) { return conf_password.set(value); }

const char* getApSSID() { return conf_ap_ssid.get(); }
ConfigStatus setApSSID(const char* value) { return conf_ap_ssid.set(value); }

const char* getApPassword() { return conf_ap_password.get(); }
ConfigStatus setApPassword(const char* value) { return conf_ap_password.set(value); }

const char* getHostName() { return conf_hostName.get(); }
ConfigStatus setHostName(const char* value) {
    size_t len = strlen(value);
    if (len < 1 || len > 15) {
        return ConfigStatus::ERROR_HOSTNAME_LENGTH;
    }
    if (value[0] == '-' || value[len - 1] == '-') {
        return ConfigStatus::ERROR_HOSTNAME_HYPHEN;
    }
    for (size_t i = 0; i < len; i++) {
        if (!isalnum(value[i]) && value[i] != '-') {
            return ConfigStatus::ERROR_HOSTNAME_CHARS;
        }
    }
    return conf_hostName.set(value);
}

const char* getMqttServer() { return conf_mqtt_server.get(); }
ConfigStatus setMqttServer(const char* value) { return conf_mqtt_server.set(value); }

int getMqttPort() { return conf_mqtt_port.get(); }
ConfigStatus setMqttPort(int value) { return conf_mqtt_port.set(value); }

const char* getMqttUser() { return conf_mqtt_user.get(); }
ConfigStatus setMqttUser(const char* value) { return conf_mqtt_user.set(value); }

const char* getMqttPassword() { return conf_mqtt_password.get(); }
ConfigStatus setMqttPassword(const char* value) { return conf_mqtt_password.set(value); }

const char* getVehicleId() { return conf_vehicle_id.get(); }
ConfigStatus setVehicleId(const char* value) { return conf_vehicle_id.set(value); }
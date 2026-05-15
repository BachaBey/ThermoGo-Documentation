#include "Storage.h"
#include "secrets.h"
#include <Preferences.h>
#include <WiFi.h>

static Preferences prefs;

void Storage::init() {
    prefs.begin("thermogo", false);

    // Generate device ID from MAC address on first boot
    if (!prefs.isKey("device_id")) {
        String mac = WiFi.macAddress();
        mac.replace(":", "");
        mac.toUpperCase();
        prefs.putString("device_id", mac);
    }

    // Seed default networks if not already stored
    int defaultCount = sizeof(DEFAULT_NETWORKS) / sizeof(DEFAULT_NETWORKS[0]);
    for (int d = 0; d < defaultCount; d++) {
        if (strlen(DEFAULT_NETWORKS[d].ssid) == 0) continue;

        // Check if this SSID is already saved
        int count = prefs.getInt("net_count", 0);
        bool found = false;
        for (int i = 0; i < count; i++) {
            char key[12], stored[33];
            snprintf(key, sizeof(key), "ssid_%d", i);
            prefs.getString(key, stored, sizeof(stored));
            if (strcmp(stored, DEFAULT_NETWORKS[d].ssid) == 0) { found = true; break; }
        }

        if (!found && count < MAX_NETWORKS) {
            saveNetwork(count, DEFAULT_NETWORKS[d].ssid, DEFAULT_NETWORKS[d].password, "connected");
        }
    }
}

String Storage::getDeviceId() {
    return prefs.getString("device_id", "");
}

int Storage::getNetworkCount() {
    return prefs.getInt("net_count", 0);
}

bool Storage::getNetwork(int index, NetworkEntry& entry) {
    if (index < 0 || index >= MAX_NETWORKS) return false;

    char key[12];
    snprintf(key, sizeof(key), "ssid_%d", index);
    if (!prefs.isKey(key)) return false;

    prefs.getString(key, entry.ssid, sizeof(entry.ssid));
    snprintf(key, sizeof(key), "pass_%d", index);
    prefs.getString(key, entry.password, sizeof(entry.password));
    snprintf(key, sizeof(key), "state_%d", index);
    prefs.getString(key, entry.state, sizeof(entry.state));

    return true;
}

void Storage::saveNetwork(int index, const char* ssid, const char* password, const char* state) {
    if (index < 0 || index >= MAX_NETWORKS) return;

    char key[12];
    snprintf(key, sizeof(key), "ssid_%d", index);
    prefs.putString(key, ssid);
    snprintf(key, sizeof(key), "pass_%d", index);
    prefs.putString(key, password);
    snprintf(key, sizeof(key), "state_%d", index);
    prefs.putString(key, state);

    int count = prefs.getInt("net_count", 0);
    if (index >= count) {
        prefs.putInt("net_count", index + 1);
    }
}

void Storage::setNetworkState(int index, const char* state) {
    if (index < 0 || index >= MAX_NETWORKS) return;
    char key[12];
    snprintf(key, sizeof(key), "state_%d", index);
    prefs.putString(key, state);
}

void Storage::clearNetworks() {
    char key[12];
    for (int i = 0; i < MAX_NETWORKS; i++) {
        snprintf(key, sizeof(key), "ssid_%d", i);  prefs.remove(key);
        snprintf(key, sizeof(key), "pass_%d", i);  prefs.remove(key);
        snprintf(key, sizeof(key), "state_%d", i); prefs.remove(key);
    }
    prefs.putInt("net_count", 0);
    prefs.putInt("last_working", -1);
}

int Storage::getLastWorkingIndex() {
    return prefs.getInt("last_working", -1);
}

void Storage::setLastWorkingIndex(int index) {
    prefs.putInt("last_working", index);
}

bool Storage::deleteNetwork(const char* ssid) {
    int count = prefs.getInt("net_count", 0);
    int found = -1;

    // Find the index of the network to delete
    for (int i = 0; i < count; i++) {
        char key[12];
        char stored[33];
        snprintf(key, sizeof(key), "ssid_%d", i);
        prefs.getString(key, stored, sizeof(stored));
        if (strcmp(stored, ssid) == 0) { found = i; break; }
    }
    if (found < 0) return false;

    // Shift remaining networks down to fill the gap
    for (int i = found; i < count - 1; i++) {
        char keyFrom[12], keyTo[12];
        char buf[65];

        snprintf(keyFrom, sizeof(keyFrom), "ssid_%d",  i + 1);
        snprintf(keyTo,   sizeof(keyTo),   "ssid_%d",  i);
        prefs.getString(keyFrom, buf, sizeof(buf)); prefs.putString(keyTo, buf);

        snprintf(keyFrom, sizeof(keyFrom), "pass_%d",  i + 1);
        snprintf(keyTo,   sizeof(keyTo),   "pass_%d",  i);
        prefs.getString(keyFrom, buf, sizeof(buf)); prefs.putString(keyTo, buf);

        snprintf(keyFrom, sizeof(keyFrom), "state_%d", i + 1);
        snprintf(keyTo,   sizeof(keyTo),   "state_%d", i);
        prefs.getString(keyFrom, buf, sizeof(buf)); prefs.putString(keyTo, buf);
    }

    // Remove the last slot
    char key[12];
    snprintf(key, sizeof(key), "ssid_%d",  count - 1); prefs.remove(key);
    snprintf(key, sizeof(key), "pass_%d",  count - 1); prefs.remove(key);
    snprintf(key, sizeof(key), "state_%d", count - 1); prefs.remove(key);
    prefs.putInt("net_count", count - 1);

    // Reset last working index if it pointed to the deleted or shifted network
    int last = prefs.getInt("last_working", -1);
    if (last >= count - 1) prefs.putInt("last_working", -1);

    return true;
}

bool Storage::isConfigFlagSet() {
    return prefs.getBool("cfg_flag", false);
}

void Storage::setConfigFlag() {
    prefs.putBool("cfg_flag", true);
}

void Storage::clearConfigFlag() {
    prefs.putBool("cfg_flag", false);
}

#pragma once
#include <Arduino.h>

#define MAX_NETWORKS 5

struct NetworkEntry {
    char ssid[33];
    char password[65];
    char state[20];  // pending, connected, wrong_password, not_available, error
};

namespace Storage {
    void init();

    // Device ID
    String getDeviceId();

    // Networks
    int    getNetworkCount();
    bool   getNetwork(int index, NetworkEntry& entry);
    void   saveNetwork(int index, const char* ssid, const char* password, const char* state);
    void   setNetworkState(int index, const char* state);
    void   clearNetworks();

    // Last working network
    int    getLastWorkingIndex();
    void   setLastWorkingIndex(int index);

    // Delete a network by SSID
    bool   deleteNetwork(const char* ssid);

    // Config flag (set remotely via /start_config to trigger hotspot on next boot)
    bool   isConfigFlagSet();
    void   setConfigFlag();
    void   clearConfigFlag();
}

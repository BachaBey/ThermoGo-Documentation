#include "WiFiManager.h"
#include "Storage.h"
#include <WiFi.h>

#define CONNECT_TIMEOUT_MS 10000

void WiFiManager::init() {
    WiFi.mode(WIFI_STA);
    WiFi.disconnect(true);
    delay(100);
}

static bool ssidInScan(const char* ssid, int scanCount) {
    for (int i = 0; i < scanCount; i++) {
        if (WiFi.SSID(i).equals(ssid)) return true;
    }
    return false;
}

static bool tryConnect(const char* ssid, const char* password, int index) {
    WiFi.begin(ssid, password);

    unsigned long start = millis();
    while (WiFi.status() != WL_CONNECTED) {
        if (millis() - start > CONNECT_TIMEOUT_MS) break;
        delay(100);
    }

    if (WiFi.status() == WL_CONNECTED) {
        Storage::setNetworkState(index, "connected");
        Storage::setLastWorkingIndex(index);
        Serial.printf("[WiFi] Connected to: %s\n", ssid);
        return true;
    }

    if (WiFi.status() == WL_CONNECT_FAILED) {
        Storage::setNetworkState(index, "wrong_password");
        Serial.printf("[WiFi] Wrong password for: %s\n", ssid);
    } else {
        Storage::setNetworkState(index, "error");
        Serial.printf("[WiFi] Error connecting to: %s\n", ssid);
    }

    WiFi.disconnect(true);
    return false;
}

bool WiFiManager::connect() {
    int count = Storage::getNetworkCount();
    if (count == 0) return false;

    // Scan available networks
    Serial.println("[WiFi] Scanning...");
    int scanCount = WiFi.scanNetworks();
    Serial.printf("[WiFi] Found %d networks\n", scanCount);

    // Build connection order: last working first, then the rest
    int order[MAX_NETWORKS];
    int orderLen = 0;
    int lastWorking = Storage::getLastWorkingIndex();

    if (lastWorking >= 0 && lastWorking < count) {
        order[orderLen++] = lastWorking;
    }
    for (int i = 0; i < count; i++) {
        if (i != lastWorking) order[orderLen++] = i;
    }

    // Try each network
    for (int i = 0; i < orderLen; i++) {
        NetworkEntry entry;
        if (!Storage::getNetwork(order[i], entry)) continue;

        if (!ssidInScan(entry.ssid, scanCount)) {
            Storage::setNetworkState(order[i], "not_available");
            Serial.printf("[WiFi] Not available: %s\n", entry.ssid);
            continue;
        }

        if (tryConnect(entry.ssid, entry.password, order[i])) {
            return true;
        }
    }

    return false;
}

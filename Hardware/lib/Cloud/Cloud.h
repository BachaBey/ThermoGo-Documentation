#pragma once
#include <Arduino.h>
#include "Sensor.h"

#define CLOUD_BUFFER_SIZE 10

namespace Cloud {
    bool deviceExists(const char* deviceId);
    bool fetchDeviceConfig(const char* deviceId);
    bool pushNotification(const char* deviceId, const char* type, const char* message);
    bool pushReadings(const char* deviceId, SensorReading* readings, int count);
    bool updateDeviceStatus(const char* deviceId);
    void fetchAndSavePendingNetworks(const char* deviceId);

    // Calls Edge Function → Claude API, writes a short insight into outBuf (max bufLen chars)
    bool fetchAiInsight(const char* deviceId, float temp, float hum, int battery, char* outBuf, size_t bufLen);
}
